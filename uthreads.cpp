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

#define FAIL -1
#define SUCCESS 0

class ThreadManager {
public:
    std::vector<Thread *> threads;
    std::map<int, std::vector<int>*> wake_up_calls;
    std::deque<int> ready;
    std::set<int> vacantIndices;
    int current_active_thread;


    explicit ThreadManager(int quantum_length) {
        this->threads = std::vector<Thread *>();
        this->ready = std::deque<int>();
        this->vacantIndices = std::set<int>();
        this->wake_up_calls = std::map<int,  std::vector<int>*>();
        this-> current_active_thread = -1;
    }

    void time_handler(int sig){
        // todo - fine's part goes here - see the use of current_active_thread
        printf("got sigvtalrm");
        if(ready.empty()) return;
        else{}
    }

    int add(thread_entry_point entry_point) {
        Thread *new_thread = new Thread(entry_point);
        int index = threads.size();

        if (!vacantIndices.empty()) {
            index = *vacantIndices.begin();
            vacantIndices.erase(vacantIndices.begin());
        }

        if (index == MAX_THREAD_NUM) return FAIL;

        if(index == threads.size()) threads.push_back(new_thread); else threads[index] = new_thread;

        ready.push_back(index);
        return SUCCESS;
    }

    int terminate(int tid) {
        if(tid > threads.size()) return FAIL;
        Thread *to_terminate = threads[tid];
        if (to_terminate == nullptr ) return FAIL;
        if (to_terminate->state == READY) ready.erase(std::remove(ready.begin(), ready.end(), tid), ready.end());
        if(to_terminate->is_asleep) {
            std::vector<int>* calls = wake_up_calls[to_terminate->wake_up_time];
            calls->erase(std::remove(calls->begin(), calls->end(), tid), calls->end());
        }
        threads[tid] = nullptr;
        delete(to_terminate);
        return SUCCESS;
    }
};


ThreadManager *manager;
struct itimerval timer;
struct sigaction sa = {0};

void timer_handler(int sig){
    manager-> time_handler(sig);
}

void initTimer(int usecs){
    timer.it_interval.tv_sec = 0;    // following time intervals, seconds part
    timer.it_interval.tv_usec = usecs;    // following time intervals, microseconds part
    setitimer(ITIMER_VIRTUAL, &timer, nullptr);
    sa.sa_handler = &timer_handler;
    if (sigaction(SIGVTALRM, &sa, NULL) < 0)
    {
        printf("sigaction error.");
    }
}

int uthread_init(int quantum_usecs) {
    if (quantum_usecs < 1) return FAIL;
    initTimer(quantum_usecs);
    manager = new ThreadManager(quantum_usecs);
    return SUCCESS;
}

int uthread_spawn(thread_entry_point entry_point) {
    return manager->add(entry_point);
}

int uthread_terminate(int tid) {
    return manager->terminate(tid);
}

int uthread_block(int tid){

}