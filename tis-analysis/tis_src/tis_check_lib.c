/**************************************************************************/
/*                                                                        */
/*  This file is part of deliverable T3.3 of project INGOPCS              */
/*                                                                        */
/*    Copyright (C) 2017 TrustInSoft                                      */
/*                                                                        */
/*  All rights reserved.                                                  */
/*                                                                        */
/**************************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <check.h>

struct SRunner { const char *name; } tis_srunner;
SRunner *CK_EXPORT srunner_create(Suite * s) { return &tis_srunner; }
void srunner_set_fork_status (SRunner * sr, enum fork_status fstat) {}
void CK_EXPORT srunner_run_all(SRunner * sr, enum print_output print_mode) {}
int srunner_ntests_failed(SRunner * sr) { return 0; }
void srunner_free(SRunner * sr) { }

struct Suite { const char *name; } tis_suite;
Suite * suite_create(const char *name) { return &tis_suite; }
void srunner_add_suite(SRunner * sr, Suite * s) { }
void suite_add_tcase (Suite * s, TCase * tc) { }

void _mark_point(const char *file, int line) {
  printf ("%s:%d: _mark_point\n", file, line);
}
void tcase_fn_start(const char *fname, const char *file, int line) { }

void _ck_assert_failed(const char *file, int line, const char *expr, ...) {
  printf ("warning: %s:%d: %s\n", file, line, expr);
  abort ();
}

static int cpt_tcase = 0;
struct TCase {
    const char *name;
    SFun setup;
    SFun teardown;
};
TCase tcase_array[100];

//@ requires new_tc: tcase_array[cpt_tcase].name == \null;
TCase * tcase_create (const char *name) {
  TCase * tc = tcase_array + cpt_tcase++;
  tc->name = name;
  tc->setup = NULL;
  tc->teardown = NULL;
  return tc;
}

//@ requires known_tc: tc->name != \null;
void tcase_add_unchecked_fixture (TCase * tc, SFun setup, SFun teardown) {
  tc->setup = setup;
  tc->teardown = teardown;
}

//@ requires known_tc: tc->name != \null;
void tcase_add_checked_fixture (TCase * tc, SFun setup, SFun teardown) {
  tc->setup = setup;
  tc->teardown = teardown;
}

void _tcase_add_test(TCase * tc, const TTest * ttest,
                     int _signal, int allowed_exit_value,
                     int start, int end) {
  if (tc->setup) tc->setup ();
  ttest->fn (0);
  if (tc->teardown) tc->teardown ();
}

//==============================================================================
