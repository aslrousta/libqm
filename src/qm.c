#include <libqm.h>
#include <math.h>
#include <stdio.h>
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
static const uint32_t INF_BIT = 1 << 30;

static const qnum_t INF = {INF_BIT, 0, 0, 0};

static inline int equal(const qnum_t *a, const qnum_t *b) {
  return !memcmp(a, b, sizeof(qnum_t));
}

static inline int is_zero(const qnum_t *n) {
  static const qnum_t ZERO = {0, 0, 0, 0};
  return equal(n, &ZERO);
}

static inline int is_inf(const qnum_t *n) {
  return n->data[0] & INF_BIT ? 1 : 0;
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
  if (!memcmp(a, b, sizeof(qnum_t))) return 0;
  if (sign(a) > sign(b)) return 1;
  if (sign(a) < sign(b)) return -1;
  if (sign(a) < 0) return cmp(b, a);
  for (int i = 3; i >= 0; i--) {
    if (a->data[i] > b->data[i]) return 1;
    if (a->data[i] < b->data[i]) return -1;
  }
  return 0;
}

int qm_sign(const qnum_t *a) { return sign(a); }

int qm_cmp(const qnum_t *a, const qnum_t *b) { return cmp(a, b); }

qnum_t qm_abs(const qnum_t *a) { return abs(a); }

qnum_t qm_neg(const qnum_t *a) { return neg(a); }

qnum_t qm_from_int(int v) {
  qnum_t r;
  int is_neg = v < 0;

  if (is_neg) v = -v;
  r.data[0] = v / 1000000000;
  r.data[1] = v % 1000000000;
  r.data[2] = 0;
  r.data[3] = 0;
  return is_neg ? neg(&r) : r;
}

qnum_t qm_from_double(double v) {
  qnum_t r;
  int is_neg = v < 0;

  if (is_neg) v = -v;
  if (v >= 1e18)
    r = INF;
  else {
    r.data[0] = (uint32_t)floor(v * 1e-9);
    r.data[1] = (uint32_t)floor(fmod(v, 1e9));
    r.data[2] = (uint32_t)floor(fmod(v * 1e9, 1e9));
    r.data[3] = (uint32_t)floor(fmod(v * 1e18, 1e9));
  }
  return is_neg ? neg(&r) : r;
}

void qm_to_str(const qnum_t *a, char *buf, int len) {
  char s[38];
  char *begin = s;
  char *end = s + sizeof(s) - 1;

  if (is_zero(a))
    strncpy(buf, "0", len);
  else if (is_inf(a))
    strncpy(buf, sign(a) < 0 ? "-inf" : "inf", len);
  else {
    sprintf(s, "%09d%09d.%09d%09d", (a->data[0] & ~SIGN_BIT), a->data[1],
            a->data[2], a->data[3]);
    while (*begin == '0' && *(begin + 1) != '.') begin++;
    while (*(end - 1) == '0' || *(end - 1) == '.') end--;
    *end = '\0';
    if (sign(a) < 0) strncpy(buf++, "-", len--);
    strncpy(buf, begin, len);
  }
  buf[len - 1] = '\0';
}

qnum_t qm_add(const qnum_t *a, const qnum_t *b) {
  qnum_t r;
  uint32_t c = 0;

  if (is_zero(b) || is_inf(a)) return *a;
  if (is_zero(a) || is_inf(b)) return *b;
  if (sign(a) > 0 && sign(b) < 0) {
    qnum_t b_ = abs(b);
    return qm_sub(a, &b_);
  }
  if (sign(a) < 0 && sign(b) > 0) {
    qnum_t a_ = abs(a);
    return qm_sub(b, &a_);
  }
  if (sign(a) < 0 && sign(b) < 0) {
    qnum_t a_ = abs(a);
    qnum_t b_ = abs(b);
    qnum_t t_ = qm_add(&a_, &b_);
    return neg(&t_);
  }
  for (int i = 3; i >= 0; i--) {
    r.data[i] = a->data[i] + b->data[i] + c;
    c = r.data[i] / 1000000000;
    r.data[i] = r.data[i] % 1000000000;
  }
  if (c > 0) return INF; /* overflow! */
  return r;
}

qnum_t qm_sub(const qnum_t *a, const qnum_t *b) {
  qnum_t r;
  uint32_t c = 0;

  if (is_zero(b) || is_inf(a)) return *a;
  if (is_zero(a) || is_inf(b)) return neg(b);
  if (sign(a) > 0 && sign(b) < 0) {
    qnum_t t_ = abs(b);
    return qm_add(a, &t_);
  }
  if (sign(a) < 0 && sign(b) > 0) {
    qnum_t a_ = abs(a);
    qnum_t t_ = qm_add(&a_, b);
    return neg(&t_);
  }
  if (sign(a) < 0 && sign(b) < 0) {
    qnum_t a_ = abs(a);
    qnum_t b_ = abs(b);
    return qm_sub(&b_, &a_);
  }
  if (cmp(a, b) < 0) {
    qnum_t t_ = qm_sub(b, a);
    return neg(&t_);
  }
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
