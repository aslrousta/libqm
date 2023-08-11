#include <check.h>
#include <libqm.h>
#include <math.h>

static qnum_t zero;
static qnum_t one;
static qnum_t two;
static qnum_t minus_one;
static qnum_t minus_two;
static qnum_t pi;
static qnum_t two_pi;

static void simple_setup(void) {
  zero = qm_from_int(0);
  one = qm_from_int(1);
  two = qm_from_int(2);
  minus_one = qm_from_int(-1);
  minus_two = qm_from_int(-2);
  pi = qm_from_str("3.14159265358979323");
  two_pi = qm_from_str("6.28318530717958646");
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
  ck_assert_int_eq(qm_cmp(&minus_one, &minus_one), 0);

  ck_assert_int_eq(qm_cmp(&two, &one), 1);
  ck_assert_int_eq(qm_cmp(&one, &zero), 1);
  ck_assert_int_eq(qm_cmp(&one, &minus_one), 1);
  ck_assert_int_eq(qm_cmp(&zero, &minus_one), 1);
  ck_assert_int_eq(qm_cmp(&minus_one, &minus_two), 1);

  ck_assert_int_eq(qm_cmp(&one, &two), -1);
  ck_assert_int_eq(qm_cmp(&zero, &one), -1);
  ck_assert_int_eq(qm_cmp(&minus_one, &one), -1);
  ck_assert_int_eq(qm_cmp(&minus_one, &zero), -1);
  ck_assert_int_eq(qm_cmp(&minus_two, &minus_one), -1);
}
END_TEST

START_TEST(abs_returns_correct_absolute_number) {
  qnum_t r;

  r = qm_abs(zero);
  ck_assert_mem_eq(&r, &zero, sizeof(qnum_t));

  r = qm_abs(one);
  ck_assert_mem_eq(&r, &one, sizeof(qnum_t));

  r = qm_abs(minus_one);
  ck_assert_mem_eq(&r, &one, sizeof(qnum_t));
}
END_TEST

START_TEST(to_str_formats_number_correctly) {
  char buf[100];

  qm_to_str(&zero, buf, 100);
  ck_assert_str_eq(buf, "0");

  qm_to_str(&one, buf, 100);
  ck_assert_str_eq(buf, "1");

  qm_to_str(&minus_one, buf, 100);
  ck_assert_str_eq(buf, "-1");

  qm_to_str(&pi, buf, 100);
  ck_assert_str_eq(buf, "3.14159265358979323");
}
END_TEST

START_TEST(add_returns_correct_result) {
  qnum_t r;

  r = qm_add(zero, one);
  ck_assert_mem_eq(&r, &one, sizeof(qnum_t));

  r = qm_add(one, one);
  ck_assert_mem_eq(&r, &two, sizeof(qnum_t));

  r = qm_add(one, minus_one);
  ck_assert_mem_eq(&r, &zero, sizeof(qnum_t));

  r = qm_add(minus_one, minus_one);
  ck_assert_mem_eq(&r, &minus_two, sizeof(qnum_t));

  r = qm_add(minus_two, one);
  ck_assert_mem_eq(&r, &minus_one, sizeof(qnum_t));

  r = qm_add(pi, pi);
  ck_assert_mem_eq(&r, &two_pi, sizeof(qnum_t));
}
END_TEST

START_TEST(sub_returns_correct_result) {
  qnum_t r;

  r = qm_sub(one, zero);
  ck_assert_mem_eq(&r, &one, sizeof(qnum_t));

  r = qm_sub(one, one);
  ck_assert_mem_eq(&r, &zero, sizeof(qnum_t));

  r = qm_sub(one, minus_one);
  ck_assert_mem_eq(&r, &two, sizeof(qnum_t));

  r = qm_sub(minus_one, minus_one);
  ck_assert_mem_eq(&r, &zero, sizeof(qnum_t));

  r = qm_sub(minus_two, minus_one);
  ck_assert_mem_eq(&r, &minus_one, sizeof(qnum_t));

  r = qm_sub(two_pi, pi);
  ck_assert_mem_eq(&r, &pi, sizeof(qnum_t));
}
END_TEST

int main(void) {
  SRunner *sr;
  Suite *s;
  TCase *tc_simple, *tc_addsub;
  int failed;

  tc_simple = tcase_create("Simple Operations");
  tcase_add_unchecked_fixture(tc_simple, simple_setup, NULL);
  tcase_add_test(tc_simple, sign_returns_correct_number_sign);
  tcase_add_test(tc_simple, cmp_returns_correct_order);
  tcase_add_test(tc_simple, abs_returns_correct_absolute_number);
  tcase_add_test(tc_simple, to_str_formats_number_correctly);

  tc_addsub = tcase_create("Addition & Subtraction");
  tcase_add_unchecked_fixture(tc_addsub, simple_setup, NULL);
  tcase_add_test(tc_addsub, add_returns_correct_result);
  tcase_add_test(tc_addsub, sub_returns_correct_result);

  s = suite_create("Basics");
  suite_add_tcase(s, tc_simple);
  suite_add_tcase(s, tc_addsub);

  sr = srunner_create(s);
  srunner_run_all(sr, CK_NORMAL);

  failed = srunner_ntests_failed(sr);
  srunner_free(sr);
  return failed > 0;
}
