#include <stdint.h>

#if !defined(LIBQM_H__INCLUDED)
#define LIBQM_H__INCLUDED

#if defined(__cplusplus)
extern "C" {
#endif

typedef struct qnum {
  uint32_t data[4];
} qnum_t;

qnum_t qm_add(qnum_t a, qnum_t b);
qnum_t qm_sub(qnum_t a, qnum_t b);
qnum_t qm_mul(qnum_t a, qnum_t b);
qnum_t qm_div(qnum_t a, qnum_t b);

#if defined(__cplusplus)
}
#endif

#endif /* LIBQM_H__INCLUDED */
