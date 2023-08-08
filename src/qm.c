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

static const qnum_t ZERO = {0, 0, 0, 0};
static const qnum_t INF = {INF_BIT, 0, 0, 0};

int qm_sign(const qnum_t *a) {
  if (qm_is_zero(a)) return 0;
  return a->data[0] & SIGN_BIT ? -1 : 1;
}

int qm_cmp(const qnum_t *a, const qnum_t *b) {
  if (!memcmp(a, b, sizeof(qnum_t))) return 0;
  if (qm_sign(a) > qm_sign(b)) return 1;
  if (qm_sign(a) < qm_sign(b)) return -1;
  if (qm_sign(a) < 0) return qm_cmp(b, a);
  for (int i = 3; i >= 0; i--) {
    if (a->data[i] > b->data[i]) return 1;
    if (a->data[i] < b->data[i]) return -1;
  }
  return 0;
}

int qm_equal(const qnum_t *a, const qnum_t *b) {
  return !memcmp(a, b, sizeof(qnum_t));
}

int qm_is_zero(const qnum_t *n) { return qm_equal(n, &ZERO); }

int qm_is_inf(const qnum_t *n) { return n->data[0] & INF_BIT ? 1 : 0; }

qnum_t qm_abs(qnum_t a) {
  a.data[0] &= ~SIGN_BIT;
  return a;
}

qnum_t qm_neg(qnum_t a) {
  if (!qm_is_zero(&a)) a.data[0] ^= SIGN_BIT;
  return a;
}

qnum_t qm_from_int(int v) {
  qnum_t r;
  int is_neg = v < 0;

  if (is_neg) v = -v;
  r.data[0] = v / 1000000000;
  r.data[1] = v % 1000000000;
  r.data[2] = 0;
  r.data[3] = 0;
  return is_neg ? qm_neg(r) : r;
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
  return is_neg ? qm_neg(r) : r;
}

void qm_to_str(const qnum_t *a, char *buf, int len) {
  char s[38];
  char *begin = s;
  char *end = s + sizeof(s) - 1;

  if (qm_is_zero(a))
    strncpy(buf, "0", len);
  else if (qm_is_inf(a))
    strncpy(buf, qm_sign(a) < 0 ? "-inf" : "inf", len);
  else {
    sprintf(s, "%09d%09d.%09d%09d", (a->data[0] & ~SIGN_BIT), a->data[1],
            a->data[2], a->data[3]);
    while (*begin == '0' && *(begin + 1) != '.') begin++;
    while (*(end - 1) == '0' || *(end - 1) == '.') end--;
    *end = '\0';
    if (qm_sign(a) < 0) strncpy(buf++, "-", len--);
    strncpy(buf, begin, len);
  }
  buf[len - 1] = '\0';
}

qnum_t qm_add(qnum_t a, qnum_t b) {
  qnum_t r;
  uint32_t c = 0;

  if (qm_is_zero(&b) || qm_is_inf(&a)) return a;
  if (qm_is_zero(&a) || qm_is_inf(&b)) return b;
  if (qm_sign(&a) > 0 && qm_sign(&b) < 0) return qm_sub(a, qm_abs(b));
  if (qm_sign(&a) < 0 && qm_sign(&b) > 0) return qm_sub(b, qm_abs(a));
  if (qm_sign(&a) < 0 && qm_sign(&b) < 0)
    return qm_neg(qm_add(qm_abs(a), qm_abs(b)));
  for (int i = 3; i >= 0; i--) {
    r.data[i] = a.data[i] + b.data[i] + c;
    c = r.data[i] / 1000000000;
    r.data[i] = r.data[i] % 1000000000;
  }
  if (c > 0) return INF; /* overflow! */
  return r;
}

qnum_t qm_sub(qnum_t a, qnum_t b) {
  qnum_t r;
  uint32_t c = 0;

  if (qm_is_zero(&b) || qm_is_inf(&a)) return a;
  if (qm_is_zero(&a) || qm_is_inf(&b)) return qm_neg(b);
  if (qm_sign(&a) > 0 && qm_sign(&b) < 0) return qm_add(a, qm_abs(b));
  if (qm_sign(&a) < 0 && qm_sign(&b) > 0) return qm_neg(qm_add(qm_abs(a), b));
  if (qm_sign(&a) < 0 && qm_sign(&b) < 0) return qm_sub(qm_abs(b), qm_abs(a));
  if (qm_cmp(&a, &b) < 0) return qm_neg(qm_sub(b, a));
  for (int i = 3; i >= 0; i--) {
    if (a.data[i] < b.data[i] + c) {
      r.data[i] = 1000000000 + a.data[i] - b.data[i] - c;
      c = 1;
    } else {
      r.data[i] = a.data[i] - b.data[i] - c;
      c = 0;
    }
  }
  return r;
}
