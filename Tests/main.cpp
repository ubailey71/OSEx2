/*
 * sigsetjmp/siglongjmp demo program.
 * Hebrew University OS course.
 * Author: OS, os@cs.huji.ac.il
 */

#include "uthreads.h"
#include <iostream>
#include <csignal>
#include <setjmp.h>
#include <cassert>
#include <vector>

void trigger() {
  std::raise(SIGUSR1);
}



static int test_val = 0;
static int nap_time = 4;

void func1() {
  test_val = 1;
  trigger();
  test_val = 2;
}

void spawn_should_run_and_terminate() {
  test_val = 0;
  uthread_spawn(&func1);
  trigger();
  assert((test_val == 1) && "Should have been waken");
  trigger();
  assert((test_val == 2) && "Should have been waken");
  assert((uthread_get_quantums(1) == -1) && "Should have been waken");
}

void func2() {
  test_val = 1;
  trigger();
  uthread_sleep(nap_time);
  test_val = 2;
}

void sleep_should_sleep_for_specified_quantums() {
  test_val = 0;
  nap_time = 12;
  uthread_spawn(&func2);
  trigger();
  assert((test_val == 1) && "Should have been waken");
  for (int i = 0; i < nap_time; ++i)
    {
      trigger();
      assert((test_val == 1) && "Should have been waken");
    }
  trigger();
  assert((test_val == 2) && "Should have been waken");
  assert((uthread_get_quantums(1) == -1) && "Should have been waken");
}


static std::vector<int> thread_order;

void func3() {
  thread_order.push_back (uthread_get_tid());
  trigger();
  thread_order.push_back (uthread_get_tid());
  trigger();
}

void spawn_should_allocate_next_available_tid() {
  thread_order = { };
  std::vector<int> expected_thread_order = { 1, 2, 3, 4};
  uthread_spawn(&func3);
  uthread_spawn(&func3);
  uthread_spawn(&func3);
  uthread_spawn(&func3);
  trigger();

  assert((expected_thread_order == thread_order) && "Should have been waken");
  thread_order.clear();

  uthread_terminate(3);
  uthread_spawn(&func3);
  trigger();

  expected_thread_order = { 1, 2, 4, 3};
  assert((expected_thread_order == thread_order) && "Should have been waken");
  trigger();
  trigger();
  trigger();
  trigger();
  trigger();
}


void func4_1() {
  test_val = 1;
  trigger();
  test_val = 2;
}

void func4_2() {
  uthread_block (1);
  trigger();
  while (test_val == 1) {
    trigger();
  }
}

void block_should_block_thread_until_resume() {
  test_val = 0;
  uthread_spawn(&func4_1);
  uthread_spawn(&func4_2);
  trigger();
  assert((test_val == 1) && "Should have been waken");
  trigger();
  trigger();
  trigger();
  trigger();
  assert((test_val == 1) && "Should have been waken");
  uthread_resume (1);
  trigger();
  assert((test_val == 2) && "Should have been waken");
}


void func5() {
  test_val = 1;
  uthread_sleep (nap_time);
  test_val = 2;
}

void sleep_non_should_help() {
  test_val = 0;
  nap_time = 5;
  uthread_spawn(&func5);
  trigger();
  assert((test_val == 1) && "Should have been waken");
  uthread_resume (1);
  assert((test_val == 1) && "Should have been waken");
  trigger();
  assert((test_val == 1) && "Should have been waken");
  uthread_resume (1);
  assert((test_val == 1) && "Should have been waken");
  trigger();
  assert((test_val == 1) && "Should have been waken");
  uthread_resume (1);
  assert((test_val == 1) && "Should have been waken");
  trigger();
  assert((test_val == 1) && "Should have been waken");
  trigger();
  trigger();
  trigger();
  assert((test_val == 2) && "Should have been waken");
}

void func6() {
  test_val = 1;
  uthread_sleep (nap_time);
  test_val = 2;
}

void sleep_with_block_should_not_be_ready_immediately() {
  test_val = 0;
  nap_time = 5;
  uthread_spawn(&func6);
  trigger();
  assert((test_val == 1) && "Should have been waken");
  uthread_block (1);
  uthread_resume(1);

  uthread_block (1);
  for (int i = 0; i < nap_time + 10; ++i)
    {
      trigger();
    }
  assert((test_val == 1) && "Should have been waken");
  uthread_resume (1);
  trigger();
  assert((test_val == 2) && "Should have been waken");
}

void func7() {
  test_val = 1;
  uthread_sleep (nap_time);
  test_val = 2;
}

void sleep_with_block_then_resume_should_be_ready_immediately() {
  test_val = 0;
  nap_time = 5;
  uthread_spawn(&func7);
  trigger();
  assert((test_val == 1) && "Should have been waken");
  uthread_block (1);
  uthread_resume(1);
  uthread_block (1);
  uthread_resume(1);
  for (int i = 0; i < nap_time + 10; ++i)
    {
      trigger();
    }
  assert((test_val == 2) && "Should have been waken");
}

int main()
{
  uthread_init(1000000);
  spawn_should_run_and_terminate();
  sleep_should_sleep_for_specified_quantums();
  spawn_should_allocate_next_available_tid();
  block_should_block_thread_until_resume();
  sleep_non_should_help();
  sleep_with_block_should_not_be_ready_immediately();
  sleep_with_block_then_resume_should_be_ready_immediately();
  return 0;
}


