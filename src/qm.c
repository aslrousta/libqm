#include <libqm.h>
#include <string.h>

/*
 * The numbers are stored in a 128-bit data structure composed of four 32-bit
 * unsigned integers, each holding a number between 0 and 999'999'999. The MSB
 * of the first integer is also used as a sign bit.
 *
 *      +-------------------------------------------------------+
 *      | most significant                    least significant |
 *      +-------------+-------------+-------------+-------------+
 *      |   data[0]   |   data[1]   |   data[2]   |   data[3]   |
 *      +-------------+-------------+-------------+-------------+
 *      | 999'999'999 | 999'999'999 | 999'999'999 | 999'999'999 |
 *      +-------------+-------------+-------------+-------------+
 */

static const uint32_t SIGN_BIT = 1 << 31;

static inline int equal(const qnum_t *a, const qnum_t *b) {
  return !memcmp(a, b, sizeof(qnum_t));
}

static inline int is_zero(const qnum_t *n) {
  static const qnum_t ZERO = {0, 0, 0, 0};
  return equal(n, &ZERO);
}

static inline int sign(const qnum_t *n) {
  if (is_zero(n)) return 0;
  return n->data[0] & SIGN_BIT ? -1 : 1;
}

static inline qnum_t abs(const qnum_t *n) {
  qnum_t r = *n;
  r.data[0] &= ~SIGN_BIT;
  return r;
}

static inline qnum_t neg(const qnum_t *n) {
  qnum_t r = *n;
  if (!is_zero(n)) r.data[0] ^= SIGN_BIT;
  return r;
}

static inline int cmp(const qnum_t *a, const qnum_t *b) {
  if (sign(a) > sign(b)) {
    return 1;
  } else if (sign(a) < sign(b)) {
    return -1;
  } else if (!memcmp(a, b, sizeof(qnum_t))) {
    return 0;
  } else if (sign(a) < 0) {
    return cmp(b, a);
  } else {
    for (int i = 3; i >= 0; i--) {
      if (a->data[i] > b->data[i]) {
        return 1;
      } else if (a->data[i] < b->data[i]) {
        return -1;
      }
    }
    return 0;
  }
}

int qm_sign(const qnum_t *a) { return sign(a); }

int qm_cmp(const qnum_t *a, const qnum_t *b) { return cmp(a, b); }

qnum_t qm_abs(const qnum_t *a) { return abs(a); }

qnum_t qm_neg(const qnum_t *a) { return neg(a); }

qnum_t qm_add(const qnum_t *a, const qnum_t *b) {
  if (is_zero(b)) {
    return *a;
  } else if (is_zero(a)) {
    return *b;
  } else if (sign(a) > 0 && sign(b) < 0) {
    qnum_t b_ = abs(b);
    return qm_sub(a, &b_);
  } else if (sign(a) < 0 && sign(b) > 0) {
    qnum_t a_ = abs(a);
    return qm_sub(b, &a_);
  } else if (sign(a) < 0 && sign(b) < 0) {
    qnum_t a_ = abs(a);
    qnum_t b_ = abs(b);
    qnum_t t_ = qm_add(&a_, &b_);
    return neg(&t_);
  } else {
    qnum_t r;
    uint32_t c = 0;
    for (int i = 3; i >= 0; i--) {
      r.data[i] = a->data[i] + b->data[i] + c;
      c = r.data[i] / 1000000000;
      r.data[i] = r.data[i] % 1000000000;
    }
    /* TODO: if c > 0, it's an overflow */
    return r;
  }
}

qnum_t qm_sub(const qnum_t *a, const qnum_t *b) {
  if (is_zero(b)) {
    return *a;
  } else if (is_zero(a)) {
    return neg(b);
  } else if (sign(a) > 0 && sign(b) < 0) {
    qnum_t t_ = abs(b);
    return qm_add(a, &t_);
  } else if (sign(a) < 0 && sign(b) > 0) {
    qnum_t a_ = abs(a);
    qnum_t t_ = qm_add(&a_, b);
    return neg(&t_);
  } else if (sign(a) < 0 && sign(b) < 0) {
    qnum_t a_ = abs(a);
    qnum_t b_ = abs(b);
    return qm_sub(&b_, &a_);
  } else if (cmp(a, b) < 0) {
    qnum_t t_ = qm_sub(b, a);
    return neg(&t_);
  } else {
    qnum_t r;
    uint32_t c = 0;
    for (int i = 3; i >= 0; i--) {
      if (a->data[i] < b->data[i] + c) {
        r.data[i] = 1000000000 + a->data[i] - b->data[i] - c;
        c = 1;
      } else {
        r.data[i] = a->data[i] - b->data[i] - c;
        c = 0;
      }
    }
    return r;
  }
}
