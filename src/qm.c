#include <libqm.h>
#include <math.h>
#include <stdio.h>
#include <string.h>

/*
 * The numbers are stored in a 128-bit data structure composed of four 32-bit
 * unsigned integers, each holding a number between 0 and 999'999'999.
 *
 *      +-------------------------------------------------------+
 *      | most significant                    least significant |
 *      +-------------+-------------+-------------+-------------+
 *      |   data[0]   |   data[1]   |   data[2]   |   data[3]   |
 *      +-------------+-------------+-------------+-------------+
 *      | 999'999'999 | 999'999'999 | 999'999'999 | 999'999'999 |
 *      +-------------+-------------+-------------+-------------+
 */

const int QM_FLAG_NEG = 1;
const int QM_FLAG_INF = 2;
const int QM_FLAG_NAN = 4;

static const qnum_t ZERO = {.flag = 0, .data = {0, 0, 0, 0}};
static const qnum_t INF = {.flag = QM_FLAG_INF, .data = {0, 0, 0, 0}};
static const qnum_t NaN = {.flag = QM_FLAG_NAN, .data = {0, 0, 0, 0}};

int qm_sign(const qnum_t *a) {
  if (qm_is_zero(a)) return 0;
  return a->flag & QM_FLAG_NEG ? -1 : 1;
}

int qm_cmp(const qnum_t *a, const qnum_t *b) {
  if (!memcmp(a, b, sizeof(qnum_t))) return 0;
  if (qm_sign(a) > qm_sign(b)) return 1;
  if (qm_sign(a) < qm_sign(b)) return -1;

  int r = qm_sign(a) < 0 ? -1 : 1;
  for (int i = 0; i < 4; i++) {
    if (a->data[i] > b->data[i]) return r;
    if (a->data[i] < b->data[i]) return -r;
  }
  return 0;
}

int qm_equal(const qnum_t *a, const qnum_t *b) {
  return !memcmp(a, b, sizeof(qnum_t));
}

int qm_is_zero(const qnum_t *n) { return qm_equal(n, &ZERO); }

int qm_is_inf(const qnum_t *n) { return n->flag & QM_FLAG_INF ? 1 : 0; }

int qm_is_nan(const qnum_t *n) { return n->flag & QM_FLAG_NAN ? 1 : 0; }

qnum_t qm_abs(qnum_t a) {
  a.flag &= ~QM_FLAG_NEG;
  return a;
}

qnum_t qm_neg(qnum_t a) {
  if (!qm_is_zero(&a)) a.flag ^= QM_FLAG_NEG;
  return a;
}

qnum_t qm_from_int(int v) {
  qnum_t r = ZERO;
  int is_neg = v < 0;

  if (is_neg) v = -v;
  r.data[0] = v / 1000000000;
  r.data[1] = v % 1000000000;
  return is_neg ? qm_neg(r) : r;
}

qnum_t qm_from_str(const char *s) {
  qnum_t r = ZERO;
  const char *d = strchr(s, '.');
  int is_neg = *s == '-';

  if (!d) d = s + strlen(s);
  if (is_neg) s++;

  while (s < d - 9) r.data[0] = r.data[0] * 10 + (*s++ - '0');
  while (s < d) r.data[1] = r.data[1] * 10 + (*s++ - '0');

  s++; /* skip dot */
  while (*s && s <= d + 9) r.data[2] = r.data[2] * 10 + (*s++ - '0');
  while (*s && s < d + 18) r.data[3] = r.data[3] * 10 + (*s++ - '0');
  while (s <= d + 9) r.data[2] *= 10, s++;
  while (s <= d + 18) r.data[3] *= 10, s++;
  return is_neg ? qm_neg(r) : r;
}

void qm_to_str(const qnum_t *a, char *buf, int len) {
  if (qm_is_zero(a))
    strncpy(buf, "0", len);
  else if (qm_is_inf(a))
    strncpy(buf, qm_sign(a) < 0 ? "-inf" : "inf", len);
  else if (qm_is_nan(a))
    strncpy(buf, "NaN", len);
  else {
    char s[38];
    char *begin = s;
    char *end = s + sizeof(s) - 1;

    if (qm_sign(a) < 0) strncpy(buf++, "-", len--);
    sprintf(s, "%09d%09d.%09d%09d", a->data[0], a->data[1], a->data[2],
            a->data[3]);
    while (*begin == '0' && *(begin + 1) != '.') begin++;
    while (*(end - 1) == '0' || *(end - 1) == '.') *(--end) = '\0';
    strncpy(buf, begin, len);
    buf[len - 1] = '\0';
  }
}

qnum_t qm_add(qnum_t a, qnum_t b) {
  if (qm_is_nan(&a) || qm_is_nan(&b)) return NaN;
  if (qm_is_zero(&b) || qm_is_inf(&a)) return a;
  if (qm_is_zero(&a) || qm_is_inf(&b)) return b;
  if (qm_sign(&a) > 0 && qm_sign(&b) < 0) return qm_sub(a, qm_abs(b));
  if (qm_sign(&a) < 0 && qm_sign(&b) > 0) return qm_sub(b, qm_abs(a));
  if (qm_sign(&a) < 0 && qm_sign(&b) < 0)
    return qm_neg(qm_add(qm_abs(a), qm_abs(b)));

  qnum_t r = ZERO;
  uint32_t t, c = 0;
  for (int i = 3; i >= 0; i--) {
    t = a.data[i] + b.data[i] + c;
    c = t / 1000000000;
    r.data[i] = t % 1000000000;
  }
  if (c > 0) return INF; /* overflow! */
  return r;
}

qnum_t qm_sub(qnum_t a, qnum_t b) {
  if (qm_is_nan(&a) || qm_is_nan(&b)) return NaN;
  if (qm_is_zero(&b) || qm_is_inf(&a)) return a;
  if (qm_is_zero(&a) || qm_is_inf(&b)) return qm_neg(b);
  if (qm_sign(&a) > 0 && qm_sign(&b) < 0) return qm_add(a, qm_abs(b));
  if (qm_sign(&a) < 0 && qm_sign(&b) > 0) return qm_neg(qm_add(qm_abs(a), b));
  if (qm_sign(&a) < 0 && qm_sign(&b) < 0) return qm_sub(qm_abs(b), qm_abs(a));
  if (qm_cmp(&a, &b) < 0) return qm_neg(qm_sub(b, a));

  qnum_t r = ZERO;
  uint32_t c = 0;
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
