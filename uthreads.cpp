//
// Created by Uri Bailey on 02-Apr-22.
//
#include "uthreads.h"
//#include "thread.cpp"

#include <vector>
#include <deque>
#include <map>
#include <set>
#include <algorithm>
#include <sys/time.h>
#include <iostream>
#include <stdio.h>
#include <setjmp.h>
#include <signal.h>
#include <unistd.h>
#include <sys/time.h>
#include <stdbool.h>
#include <signal.h>

#define FAIL -1
#define SUCCESS 0
#define SCHEDULE 2

#define INVALID_TID_MESSAGE "received invalid thread index"
#define MAIN_THREAD_SLEEP_MESSAGE "cannot put main thread (tid = 0) to sleep"
#define MEMORY_ALLOCATION_FAILURE_MESSAGE "could not allocate memory for thread"

#ifdef __x86_64__
/* code for 64 bit Intel arch */

typedef unsigned long address_t;
#define JB_SP 6
#define JB_PC 7

/* A translation is required when using an address of a variable.
   Use this as a black box in your code. */
#ifndef COMMANDS_H_translate
#define COMMANDS_H_translate

#endif /* COMMANDS_H_ */
#else
/* code for 32 bit Intel arch */

typedef unsigned int address_t;
#define JB_SP 4
#define JB_PC 5

#ifndef COMMANDS_H_2
#define COMMANDS_H_2
/* A translation is required when using an address of a variable.
   Use this as a black box in your code. */
address_t translate_address(address_t addr)
{
    address_t ret;
    asm volatile("xor    %%gs:0x18,%0\n"
                 "rol    $0x9,%0\n"
                 : "=g" (ret)
                 : "0" (addr));
    return ret;
}
#endif /* COMMANDS_H_2 */

#endif

#define SECOND 1000000
//#define STACK_SIZE 4096
#define STACK_SIZE 1280000 /* stack size per thread (in bytes) */

#define NEVER -1
enum State {
  RUNNING, BLOCKED, READY
};

enum Error_Type {
    SYSTEM_ERROR, LIBRARY_ERROR
};

static int throwError(Error_Type type, const std::string &message) {
    std::string prefix;
    switch (type) {
        case SYSTEM_ERROR:
            prefix = "system error: ";
            break;
        case LIBRARY_ERROR:
            prefix = "thread library error: ";
            break;
    }
    std::cerr << prefix << message << std::endl;
    return -1;
}

class Thread {
 public:
  thread_entry_point entry_point;
  int quantums;
  State state;
  bool is_asleep;
  bool is_main;
  int wake_up_time;
  char *stack;
  int my_id;
  bool is_blocked_because_sleep;
  sigjmp_buf env;

  /**
* @brief initializes a Thread instance.
* @return self
*/
  explicit Thread (thread_entry_point entry_point, State state = READY,
                   bool is_main = false, thread_entry_point costume_entry = nullptr)
  {
    this->entry_point = entry_point;
    this->state = state;
    this->quantums = 0;
    this->is_asleep = false;
    this->wake_up_time = NEVER;
    this->stack = nullptr;
    this->is_main = is_main;
    sigsetjmp(this->env, 1);
    if (this->is_main)
      {
        this->state = RUNNING;
        this->quantums++;
        return;
      }
    else
      {       try{
                this->stack = new char[STACK_SIZE];
            }catch (...) {
                throwError(SYSTEM_ERROR, MEMORY_ALLOCATION_FAILURE_MESSAGE);
                return;
            }
        
        address_t sp = (address_t) stack + STACK_SIZE - sizeof (address_t);
        address_t pc = (address_t) costume_entry;
        (this->env->__jmpbuf)[JB_SP] = translate_address (sp);
        (this->env->__jmpbuf)[JB_PC] = translate_address (pc);
        sigemptyset (&(this->env)->__saved_mask);
      }
  }

  ~ Thread ()
  {
    delete this->stack;
  }

  /**
* @brief Save the current thread's context
* @return When saved 0, when returns after saved 1
*/
  int save_thread ()
  {
    return sigsetjmp(this->env, 1);
  }

  int terminate ()
  {
    if (this->is_main)
      {
        return 1;
      }
    return SUCCESS;
  }

  int curr_state ()
  {
    return this->state;
  }

  /**
* @brief Return to current thread's context.
* @return On success, return 0. On failure, return -1.
*/
  int activate_thread ()
  {
    siglongjmp(this->env, 1);
  }
  /**
 * @brief Tell thread to put itself in block, if running
 * make a scheduling decision.
 * @return On success, return 0. On failure, return -1.
 */
  int block ()
  {
    if (this->is_main)
      {
        return FAIL;
      }

    this->is_blocked_because_sleep = false;

    if (this->state == RUNNING)
      {
        this->state = BLOCKED;
        return 2;
      }
    this->state = BLOCKED;
    return SUCCESS;
  }


  /**
 * @brief Tell thread to activate itself.
 */
  void schedule ()
  {
    this->state = RUNNING;
    this->quantums++;
  }

  /**
 * @brief Tell thread to pause itself.
 */
  void preempt ()
  {
    this->state = READY;
  }

  /**
 * @brief Tell thread to unblock itself.
 */
  void resume ()
  { this->state = READY;
    this->is_blocked_because_sleep = true;
    }

/**
 * @brief Tell thread to put itself in sleep, if running
 * make a scheduling decision.
 * @return On success, return 0. On failure, return -1.
 */
  int sleep (int wake_time)
  {
    if (this->is_main)
      {
        return FAIL;
      }
    if ((!this->state == BLOCKED))
      {
        this->is_blocked_because_sleep = true;
      }
    this->is_asleep = true;
    this->wake_up_time = wake_time;
    if (this->state == RUNNING)
      {
        this->state = BLOCKED;
        return 2;
      }
    this->state = BLOCKED;
    return SUCCESS;
  }

  /**
 * @brief Tell thread to wake itself, if not
 * blocked become ready.
 */
  void wake_up ()
  {
    this->is_asleep = false;
    this->wake_up_time = NEVER;
    if (this->is_blocked_because_sleep)
      {
        this->state = READY;
      }
  }
};

class ThreadManager {

 public:
  std::vector<Thread *> threads;
  std::map<int, std::vector<int> *> wake_up_calls;
  std::deque<int> ready;
  std::set<int> vacantIndices;
  int current_active_thread;
  int garbage_thread_id;
  int quntums_passed;


  /**
   * ThreadManager constructor
   * @param quantum_length
  **/
   explicit ThreadManager (int quantum_length)
  {
    this->threads = std::vector<Thread *> ();
    this->ready = std::deque<int> ();
    this->vacantIndices = std::set<int> ();
    this->wake_up_calls = std::map<int, std::vector<int> *> ();
    this->current_active_thread = 0;
    this->garbage_thread_id = -1;
    this->quntums_passed = 1;
  }


  /**
 * @brief Delete previous thread if necessary.
 * @return On success, return 0. On failure, return -1.
 */
  int garbage_thread_handle ()
  {
    int tid = this->garbage_thread_id;
    Thread *to_terminate = threads[tid];
    if (to_terminate->state == READY) erase_from_ready (tid);
    if (to_terminate->is_asleep) erase_from_wake_up_calls (to_terminate->wake_up_time, tid);
    this->threads[tid] = nullptr;
    std::cout << "Inserting vacant indice: " << tid << std::endl;
    vacantIndices.insert (tid);
    delete to_terminate;
    this->garbage_thread_id = -1;
    return SUCCESS;
  }

 private:
  void wake_up_all ()
  {
    int cur_quantum = this->quntums_passed;
    if (wake_up_calls[cur_quantum] == nullptr) return;
    for (int tid: *wake_up_calls[cur_quantum])
      {
        wake_up_thread (tid);
      }
    wake_up_calls.erase (cur_quantum);
  }

  void erase_from_ready (int tid)
  {
    ready.erase (std::remove (ready.begin (), ready.end (), tid), ready.end ());
  }

  void add_to_ready (int tid)
  {
    ready.push_back (tid);
  }

  int get_next_index ()
  {
    int index = threads.size ();
    if (!vacantIndices.empty ())
      {
        index = *vacantIndices.begin ();
        vacantIndices.erase (vacantIndices.begin ());
      }
    std::cout << "get_next idx: " << index << std::endl;
    return index;
  }

  /**
   * Finish program if thread 0 is terminated, free all memory.
   * @return
  **/
  int end_program ()
  {
    for (auto &ptr: threads)
      {
        if (!(ptr == nullptr))
          {
            if (ptr->is_main)
              {
                std::cout << "Deleting main thread and finishing" << std::endl;
              }
            delete ptr;
          }
      }
    return 2;
  }

 public:

  void time_handler (int sig)
  {
    // Passing quantum inside rr cycle
    this->rr_cycle ();
  }

  int add (thread_entry_point entry_point, thread_entry_point custume_entry, bool is_main = false)
  {
    int index = get_next_index ();
    std::cout << "IDX of add: " << index << std::endl;
    Thread *new_thread = new Thread (entry_point, READY, is_main, custume_entry);
    if (index == MAX_THREAD_NUM) return FAIL;
    if (index == threads.size ())
      threads.push_back (new_thread);
    else threads[index] = new_thread;
    if (!is_main)
      {
        add_to_ready (index);
      }
    return index;
  }

/**
 * @brief Make a scheduling decision, terminate running thread if necessary.
 * @return On success, return 0. On failure, return -1.
 */
  int rr_cycle (bool dont_activate = false)
  {
    std::cout << "RR CYCLE, current active thread: "
              << this->current_active_thread << std::endl;
    this->quntums_passed++;
    this->wake_up_all ();
    Thread *curr_thread = this->threads[this->current_active_thread];

    // If timer called, add current running thread to ready.
    if (curr_thread->curr_state () == RUNNING)
      {
        curr_thread->preempt ();
        this->add_to_ready (this->current_active_thread);
      }
    int res = curr_thread->save_thread ();
    // If thread just saved, load next thread;
    if (res == SUCCESS)
      {
        this->current_active_thread = this->ready.front ();
        std::cout << "load_next_thread " << this->current_active_thread
                  << std::endl;
        this->ready.pop_front ();
        curr_thread = this->threads[this->current_active_thread];
        curr_thread->schedule ();
        std::cout << "Loading thread: " << this->current_active_thread
                  << std::endl;
        curr_thread->activate_thread ();
      }
    std::cout << "Hello, I'm back! It's thread: "
              << this->current_active_thread << std::endl;

    // If previous thread needs termination, finish deleting it.
    if (this->garbage_thread_id != -1)
      {
        std::cout << "New thread calling garbage collector" << std::endl;
        this->garbage_thread_handle ();
      }
    return SUCCESS;
  }

  int block (int tid)
  {
    Thread *to_block = threads[tid];
    if (to_block->state != BLOCKED) erase_from_ready (tid);
    std::cout << "Thread manager telling thread " << tid << " to block itself"
              << std::endl;
    int res = to_block->block ();
    if (res == FAIL)
      {
        // todo - print error
      }
    if (res == SCHEDULE)
      {
        return this->rr_cycle ();
      }
    return SUCCESS;
  }

  int resume (int tid)
  {
    Thread *to_resume = threads[tid];
    if (to_resume->state == BLOCKED)
      {
        to_resume->resume ();
        if (!to_resume->is_asleep) add_to_ready (tid);
      }
    return SUCCESS;
  }

  int terminate (int tid)
  {
    Thread *to_terminate = threads[tid];
    int is_over = to_terminate->terminate ();
    if (is_over == 1)
      {
        return this->end_program ();
      }
    std::cout << "Delete thread " << tid << std::endl;
    this->garbage_thread_id = tid;
    if (to_terminate->state == RUNNING)
      {
        std::cout << "Thread " << tid << " deleting itself" << std::endl;
        this->rr_cycle ();
      }
    return this->garbage_thread_handle ();
  }

  int put_to_sleep (int wake_up_time)
  {
    std::cout << "Thread " << this->current_active_thread
              << " goes to sleep until " <<
              wake_up_time << ", current quantum: " << this->quntums_passed
              << std::endl;
    Thread *to_sleep = threads[this->current_active_thread];
    int res = to_sleep->sleep (wake_up_time);
    if (res == FAIL)
      {
        // todo fail
      }
    erase_from_ready (current_active_thread);
    add_to_wake_up_calls (wake_up_time, current_active_thread);
    if (res == 2)
      {
        return this->rr_cycle ();
      }
    return SUCCESS;
  }

  bool is_index_vacant (int index)
  {
    return (index > 0 && index < MAX_THREAD_NUM && threads[index] == nullptr);
  }

 private:
  void wake_up_thread (int tid)
  {
    Thread *to_wake = threads[tid];
    to_wake->wake_up ();
    if (to_wake->state != BLOCKED) add_to_ready (tid);
  }

  void erase_from_wake_up_calls (int wake_up_time, int tid)
  {
    std::vector<int> *calls = wake_up_calls[wake_up_time];
    calls->erase (std::remove (calls->begin (), calls->end (), tid), calls->end ());
  }

  void add_to_wake_up_calls (int wake_up_time, int tid)
  {
    if (wake_up_calls[wake_up_time] == nullptr)
      wake_up_calls[wake_up_time] = new std::vector<int>{tid};
    else wake_up_calls[wake_up_time]->push_back (tid);
  }

};

ThreadManager *manager;
struct itimerval timer;
struct sigaction sa = {0};
int usecs;
sigset_t all_mask;
sigset_t current_mask;

void timer_handler (int sig)
{
  printf ("Time's up!\n");
  manager->time_handler (sig);
}

// todo: Handle errors, mask signals every function;


void initTimer (int usecs)
{

  sa.sa_handler = &timer_handler;
  if (sigaction (SIGUSR1, &sa, NULL) < 0)
    {
      printf ("sigaction error.");
    }

  timer.it_value.tv_sec = 0;
  timer.it_value.tv_usec = usecs;
  timer.it_interval.tv_sec = 0;
  timer.it_interval.tv_usec = usecs;
  if (setitimer (ITIMER_VIRTUAL, &timer, nullptr))
    {
      printf ("setitimer error.");
    }
  std::cout << "\nTimer is ready" << std::endl;
}

void resetTimer ()
{
  timer.it_interval.tv_sec = 0;    // following time intervals, seconds part
  timer.it_interval.tv_usec = usecs;    // following time intervals, microseconds part
}

int uthread_init (int quantum_usecs)
{
  sigprocmask (0, nullptr, &current_mask);
  sigfillset (&all_mask);
  sigprocmask (SIG_BLOCK, &all_mask, &current_mask);
  if (quantum_usecs < 1) return FAIL;
  initTimer (quantum_usecs);
  usecs = quantum_usecs;
  manager = new ThreadManager (quantum_usecs);
  manager->add (nullptr, nullptr, true);
  sigprocmask (SIG_SETMASK, &current_mask, nullptr);
  return SUCCESS;
}

bool is_valid_tid (int tid, bool include_zero)
{
  bool is_big_enough = include_zero ? tid >= 0 : tid > 0;
  bool is_small_enough = tid < MAX_THREAD_NUM;
  bool is_non_vacant = !(manager->is_index_vacant (tid));
  return is_big_enough && is_small_enough && is_non_vacant;
}


/**
 * @brief DDecoration used to remember to delete previous thread if necessary
 */
void decorate_entry_point ()
{
  if (manager->garbage_thread_id != -1)
    {
      std::cout << "New thread calling garbage collector" << std::endl;
      manager->garbage_thread_handle ();
    }
  manager->threads[manager->current_active_thread]->entry_point ();
}

int uthread_spawn (thread_entry_point entry_point)
{
  sigprocmask (0, nullptr, &current_mask);
  sigfillset (&all_mask);
  sigprocmask (SIG_BLOCK, &all_mask, &current_mask);
  int res = manager->add (entry_point, &decorate_entry_point);
  sigprocmask (SIG_SETMASK, &current_mask, nullptr);
  return res;
}

int uthread_terminate (int tid)
{
  sigprocmask (0, nullptr, &current_mask);
  sigfillset (&all_mask);
  sigprocmask (SIG_BLOCK, &all_mask, &current_mask);
  if (!is_valid_tid (tid, true)) return throwError (1);
  if (manager->current_active_thread == tid) resetTimer ();
  int res = manager->terminate (tid);
  if (res == 2)
    {
      delete manager;
      sigprocmask (SIG_SETMASK, &current_mask, nullptr);
      std::cout << "Terminated prgoram" << std::endl;
      exit (0);
    }
  sigprocmask (SIG_SETMASK, &current_mask, nullptr);
  return res;
}

int uthread_block (int tid)
{
  sigprocmask (0, nullptr, &current_mask);
  sigfillset (&all_mask);
  sigprocmask (SIG_BLOCK, &all_mask, &current_mask);
  if (!is_valid_tid (tid, false)) return throwError (2);
  if (manager->current_active_thread == tid) resetTimer ();
  std::cout << "Blocking thread " << tid << ", called thread manager"
            << std::endl;
  int res = manager->block (tid);
  sigprocmask (SIG_SETMASK, &current_mask, nullptr);
  return res;
}

int uthread_resume (int tid)
{
  sigprocmask (0, nullptr, &current_mask);
  sigfillset (&all_mask);
  sigprocmask (SIG_BLOCK, &all_mask, &current_mask);
  if (!is_valid_tid (tid, true)) return throwError (3);
  int res = manager->resume (tid);
  sigprocmask (SIG_SETMASK, &current_mask, nullptr);
  return res;
}

int uthread_sleep (int num_quantums)
{
  sigprocmask (0, nullptr, &current_mask);
  sigfillset (&all_mask);
  sigprocmask (SIG_BLOCK, &all_mask, &current_mask);
  if (manager->current_active_thread == 0) return throwError (4);
  int res = manager->put_to_sleep (1 + num_quantums + manager->quntums_passed);
  sigprocmask (SIG_SETMASK, &current_mask, nullptr);
  return res;
}

int uthread_get_tid ()
{
  sigprocmask (0, nullptr, &current_mask);
  sigfillset (&all_mask);
  sigprocmask (SIG_BLOCK, &all_mask, &current_mask);
  int res = manager->current_active_thread;
  sigprocmask (SIG_SETMASK, &current_mask, nullptr);
  return res;
}

int uthread_get_total_quantums ()
{
  sigprocmask (0, nullptr, &current_mask);
  sigfillset (&all_mask);
  sigprocmask (SIG_BLOCK, &all_mask, &current_mask);
  int res = manager->quntums_passed;
  sigprocmask (SIG_SETMASK, &current_mask, nullptr);
  return res;
}

int uthread_get_quantums (int tid)
{
  sigprocmask (0, nullptr, &current_mask);
  sigfillset (&all_mask);
  sigprocmask (SIG_BLOCK, &all_mask, &current_mask);
  if (!is_valid_tid (tid, true)) return throwError (5);
  int res = manager->threads[tid]->quantums;
  sigprocmask (SIG_SETMASK, &current_mask, nullptr);
  return res;
}
