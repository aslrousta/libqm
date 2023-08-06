#include <stddef.h>
#include <stdint.h>

#if !defined(LIBQM_H__INCLUDED)
#define LIBQM_H__INCLUDED

#if defined(__cplusplus)
extern "C" {
#endif

typedef struct qnum {
  uint32_t data[4];
} qnum_t;

int qm_sign(const qnum_t *a);
int qm_cmp(const qnum_t *a, const qnum_t *b);
qnum_t qm_abs(const qnum_t *a);
qnum_t qm_neg(const qnum_t *a);

qnum_t qm_from_int(int v);
qnum_t qm_from_double(double v);

void qm_to_str(const qnum_t *a, char *buf, int len);

qnum_t qm_add(const qnum_t *a, const qnum_t *b);
qnum_t qm_sub(const qnum_t *a, const qnum_t *b);

#if defined(__cplusplus)
}
#endif

#endif /* LIBQM_H__INCLUDED */
