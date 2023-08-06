#include <check.h>
#include <libqm.h>
#include <math.h>

static qnum_t zero;
static qnum_t one;
static qnum_t minus_one;
static qnum_t pi;

static void simple_setup(void) {
  zero = qm_from_int(0);
  one = qm_from_int(1);
  minus_one = qm_from_int(-1);
  pi = qm_from_double(M_PI);
}

START_TEST(sign_returns_correct_number_sign) {
  ck_assert_int_eq(qm_sign(&zero), 0);
  ck_assert_int_eq(qm_sign(&one), 1);
  ck_assert_int_eq(qm_sign(&minus_one), -1);
}
END_TEST

START_TEST(cmp_returns_correct_order) {
  ck_assert_int_eq(qm_cmp(&zero, &zero), 0);
  ck_assert_int_eq(qm_cmp(&one, &one), 0);
  ck_assert_int_eq(qm_cmp(&one, &one), 0);
  ck_assert_int_eq(qm_cmp(&one, &zero), 1);
  ck_assert_int_eq(qm_cmp(&zero, &one), -1);
  ck_assert_int_eq(qm_cmp(&one, &minus_one), 1);
  ck_assert_int_eq(qm_cmp(&minus_one, &one), -1);
  ck_assert_int_eq(qm_cmp(&zero, &minus_one), 1);
  ck_assert_int_eq(qm_cmp(&minus_one, &zero), -1);
}
END_TEST

START_TEST(abs_returns_correct_absolute_number) {
  qnum_t zero_ = qm_abs(&zero);
  qnum_t one_ = qm_abs(&one);
  qnum_t minus_one_ = qm_abs(&minus_one);

  ck_assert_mem_eq(&zero, &zero_, sizeof(qnum_t));
  ck_assert_mem_eq(&one, &one_, sizeof(qnum_t));
  ck_assert_mem_eq(&one, &minus_one_, sizeof(qnum_t));
}
END_TEST

START_TEST(to_str_formats_number_correctly) {
  char buf[100];

  qm_to_str(&zero, buf, 100);
  ck_assert_str_eq("0", buf);
  qm_to_str(&one, buf, 100);
  ck_assert_str_eq("1", buf);
  qm_to_str(&minus_one, buf, 100);
  ck_assert_str_eq("-1", buf);
  qm_to_str(&pi, buf, 100);
  ck_assert_str_eq("3.14159265358979328", buf);
}
END_TEST

int main(void) {
  SRunner *sr;
  Suite *s;
  TCase *tc_simple;
  int failed;

  tc_simple = tcase_create("Simple Operations");
  tcase_add_unchecked_fixture(tc_simple, simple_setup, NULL);
  tcase_add_test(tc_simple, sign_returns_correct_number_sign);
  tcase_add_test(tc_simple, cmp_returns_correct_order);
  tcase_add_test(tc_simple, abs_returns_correct_absolute_number);
  tcase_add_test(tc_simple, to_str_formats_number_correctly);

  s = suite_create("Basics");
  suite_add_tcase(s, tc_simple);

  sr = srunner_create(s);
  srunner_run_all(sr, CK_NORMAL);

  failed = srunner_ntests_failed(sr);
  srunner_free(sr);
  return failed > 0;
}