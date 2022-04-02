//
// Created by Uri Bailey on 02-Apr-22.
//
#include "uthreads.h"
#define NEVER -1
enum State {
    RUNNING, BLOCKED, READY
};

class Thread {
public:
    thread_entry_point entry_point;
    int quantums;
    State state;
    bool is_asleep;
    int wake_up_time;

    explicit Thread(thread_entry_point entry_point, State state = READY) {
        this->entry_point = entry_point;
        this->state = state;
        this->quantums = 0;
        this->is_asleep = false;
        this->wake_up_time = NEVER;
    }

    void block() { this->state = BLOCKED; }

    void schedule() { this->state = RUNNING; }

    void preempt() { this->state = READY; }

    void resume() { this->state = READY; }

    void sleep(int wake_time) {
        this->is_asleep = true;
        this->wake_up_time = wake_time;
    }

    void wake_up() {
        this->is_asleep = false;
        this->wake_up_time = NEVER;
    }

};