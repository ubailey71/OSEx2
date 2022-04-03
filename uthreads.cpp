//
// Created by Uri Bailey on 02-Apr-22.
//

#include "uthreads.h"
#include "thread.cpp"
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




#define FAIL -1
#define SUCCESS 0
#define SCHEDULE 2

class ThreadManager {
public:
    std::vector<Thread *> threads;
    std::map<int, std::vector<int> *> wake_up_calls;
    std::deque<int> ready;
    std::set<int> vacantIndices;
    int current_active_thread;


    explicit ThreadManager(int quantum_length) {
        this->threads = std::vector<Thread *>();
        this->ready = std::deque<int>();
        this->vacantIndices = std::set<int>();
        this->wake_up_calls = std::map<int, std::vector<int> *>();
        this->current_active_thread = -1;
    }

    void time_handler(int sig, int cur_quantum) {
        wake_up_all(cur_quantum);
        if (ready.empty()) return;
        else {
            this->rr_cycle();
        }
    }

    int add(thread_entry_point entry_point,bool is_main = false) {
        int index = get_next_index();
        Thread *new_thread = new Thread(entry_point,READY, is_main);
        if (index == MAX_THREAD_NUM) return FAIL;
        if (index == threads.size()) threads.push_back(new_thread); else threads[index] = new_thread;
        add_to_ready(index);
        return SUCCESS;
    }

    int rr_cycle() {
        Thread *curr_thread = this->threads[this->current_active_thread];
        if (curr_thread->curr_state()==RUNNING)
        {
            curr_thread->preempt();
            this->add_to_ready(this->current_active_thread);
        }
        int res = curr_thread->save_thread();
        // If thread just saved, load next thread;
        if(res==SUCCESS)
        {
            this->current_active_thread = this->ready.front();
            this->ready.pop_front();
            curr_thread = this->threads[this->current_active_thread];
            curr_thread->schedule();
            curr_thread->activate_thread();
        }
        return SUCCESS;
    }

    int block(int tid) {
        Thread *to_block = threads[tid];
        if (to_block->state != BLOCKED) erase_from_ready(tid);
        int res = to_block->block();
        if (res== FAIL)
        {
            // todo - print error
        }
        if (res == SCHEDULE)
        {
            return this->rr_cycle();
        }
        return SUCCESS;
    }

    int resume(int tid) {
        Thread *to_resume = threads[tid];
        if (to_resume->state == BLOCKED) {
            to_resume->resume();
            if (!to_resume->is_asleep) add_to_ready(tid);
        }
        return SUCCESS;
    }

    int terminate(int tid) {
        Thread *to_terminate = threads[tid];
        int is_over = to_terminate->terminate();
        if (is_over==1)
        {
            return this->end_program();
        }

        if (to_terminate->state== RUNNING) this->rr_cycle();
        if (to_terminate->state == READY) erase_from_ready(tid);
        if (to_terminate->is_asleep) erase_from_wake_up_calls(to_terminate->wake_up_time, tid);
        threads[tid] = nullptr;
        vacantIndices.insert(tid);
        delete to_terminate;
        return SUCCESS;
    }

    int put_to_sleep(int wake_up_time) {
        Thread *to_sleep = threads[this->current_active_thread];
        int res = to_sleep->sleep(wake_up_time);
        if (res == FAIL)
        {
            // todo fail
        }
        erase_from_ready(current_active_thread);
        add_to_wake_up_calls(wake_up_time, current_active_thread);
        if(res==2)
        {
            return this->rr_cycle();
        }
        return SUCCESS;
    }

        int end_program()
        {
            for (auto ptr: this->threads)
            {
                delete ptr;
            }
            return 2;
        }

    bool is_index_vacant(int index) {
        return (index > 0 && index < MAX_THREAD_NUM && threads[index] == nullptr);
    }

private:
    void wake_up_all(int cur_quantum) {
        if (wake_up_calls[cur_quantum] == nullptr) return;
        for (int tid : *wake_up_calls[cur_quantum]) {
            wake_up_thread(tid);
        }
        wake_up_calls.erase(cur_quantum);
    }

    void wake_up_thread(int tid) {
        Thread *to_wake = threads[tid];
        to_wake->wake_up();
        if (to_wake->state != BLOCKED) add_to_ready(tid);
    }

    void erase_from_wake_up_calls(int wake_up_time, int tid) {
        std::vector<int> *calls = wake_up_calls[wake_up_time];
        calls->erase(std::remove(calls->begin(), calls->end(), tid), calls->end());
    }

    void add_to_wake_up_calls(int wake_up_time, int tid) {
        if (wake_up_calls[wake_up_time] == nullptr) wake_up_calls[wake_up_time] = new std::vector<int>{tid};
        else wake_up_calls[wake_up_time]->push_back(tid);
    }

    void erase_from_ready(int tid) {
        ready.erase(std::remove(ready.begin(), ready.end(), tid), ready.end());
    }

    void add_to_ready(int tid) {
        ready.push_back(tid);
    }

    int get_next_index(){
        int index = threads.size();
        if (!vacantIndices.empty()) {
            index = *vacantIndices.begin();
            vacantIndices.erase(vacantIndices.begin());
        }
        return index;
    }

};


ThreadManager *manager;
struct itimerval timer;
struct sigaction sa = {0};
int usecs;
int quantums_passed;

void timer_handler(int sig) {
    quantums_passed++;
    manager->time_handler(sig, quantums_passed);
}

int throwError(int errorId);

// todo: Handle errors, mask signals every function;


void initTimer(int usecs) {
    timer.it_interval.tv_sec = 0;    // following time intervals, seconds part
    timer.it_interval.tv_usec = usecs;    // following time intervals, microseconds part
    setitimer(ITIMER_VIRTUAL, &timer, nullptr);
    sa.sa_handler = &timer_handler;
    if (sigaction(SIGVTALRM, &sa, NULL) < 0) {
        printf("sigaction error.");
    }
}

void resetTimer() {
    // Configure the timer to expire after 1 sec... */
    timer.it_value.tv_sec = 0;        // first time interval, seconds part
    timer.it_value.tv_usec = 0;        // first time interval, microseconds part

    // configure the timer to expire every 3 sec after that.
    timer.it_interval.tv_sec = 0;    // following time intervals, seconds part
    timer.it_interval.tv_usec = usecs;    // following time intervals, microseconds part
}

int uthread_init(int quantum_usecs) {
    if (quantum_usecs < 1) return FAIL;
    initTimer(quantum_usecs);
    usecs = quantum_usecs;
    quantums_passed = 1;
    manager = new ThreadManager(quantum_usecs);
    manager->add(nullptr, true);
    return SUCCESS;
}

bool is_valid_tid(int tid, bool include_zero) {
    bool is_big_enough = include_zero ? tid >= 0 : tid > 0;
    bool is_small_enough = tid > MAX_THREAD_NUM;
    bool is_non_vacant = !(manager->is_index_vacant(tid));
    return is_big_enough && is_small_enough && is_non_vacant;
}

int uthread_spawn(thread_entry_point entry_point) {
    return manager->add(entry_point);
}

int uthread_terminate(int tid) {
    if (!is_valid_tid(tid, false)) return throwError(1);
    if (manager->current_active_thread == tid) resetTimer();
    int res = manager->terminate(tid);
    if (res==2)
    {
        delete manager;
        exit(0);
    }
    return res;
}

int uthread_block(int tid) {
    if (!is_valid_tid(tid, false)) return throwError(2);
    if (manager->current_active_thread == tid) resetTimer();
    return manager->block(tid);
}

int uthread_resume(int tid) {
    if (!is_valid_tid(tid, true)) return throwError(3);
    return manager->resume(tid);
}

int uthread_sleep(int num_quantums) {
    if (manager->current_active_thread == 0) return throwError(4);
    return manager->put_to_sleep(num_quantums + quantums_passed);
}

int uthread_get_tid() {
    return manager->current_active_thread;
}

int uthread_get_total_quantums() {
    return quantums_passed;
}

int uthread_get_quantums(int tid) {
    if (!is_valid_tid(tid, true)) return throwError(5);
    return manager->threads[tid]->quantums;
}
