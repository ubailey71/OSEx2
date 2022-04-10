//
// Created by Uri Bailey on 02-Apr-22.
//
#include "uthreads.h"
#include <stdio.h>
#include <setjmp.h>
#include <signal.h>
#include <unistd.h>
#include <sys/time.h>
#include <stdbool.h>

#define FAIL -1
#define SUCCESS 0


#ifdef __x86_64__
/* code for 64 bit Intel arch */

typedef unsigned long address_t;
#define JB_SP 6
#define JB_PC 7

/* A translation is required when using an address of a variable.
   Use this as a black box in your code. */
address_t translate_address(address_t addr)
{
    address_t ret;
    asm volatile("xor    %%fs:0x30,%0\n"
                 "rol    $0x11,%0\n"
                 : "=g" (ret)
                 : "0" (addr));
    return ret;
}

#else
/* code for 32 bit Intel arch */

typedef unsigned int address_t;
#define JB_SP 4
#define JB_PC 5


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


#endif

#define SECOND 1000000
#define STACK_SIZE 4096


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
    bool is_main;
    int wake_up_time;
    char *stack;
    int my_id;
    sigjmp_buf env;

    explicit Thread(thread_entry_point entry_point, State state = READY,
                    bool is_main = false) {
        this->entry_point = entry_point;
        this->state = state;
        this->quantums = 0;
        this->is_asleep = false;
        this->wake_up_time = NEVER;
        this->stack = nullptr;
        this->is_main = is_main;
        sigsetjmp(this->env, 1);
        if(this->is_main){
            return;
        }
        else{
            this->stack = new char[STACK_SIZE];
            address_t sp = (address_t) stack +STACK_SIZE -sizeof (address_t);
            address_t pc = (address_t) entry_point;
            (this->env->__jmpbuf)[JB_SP] = translate_address(sp);
            (this->env->__jmpbuf)[JB_PC] = translate_address(pc);
            sigemptyset(&(this->env)->__saved_mask);
        }
        }

        ~ Thread(){
        delete this->stack;
    }

    int save_thread(){
        return sigsetjmp(this->env,1);
    }

    int terminate(){
        if (this->is_main)
        {
            return 1
        }
        return SUCCESS;
    }

    int curr_state(){
        return this->state;
    }

    int activate_thread(){
        siglongjmp(this->env, 1);
    }

    int block() {
        int bookmark;
        if(this->is_main)
        {
            return FAIL;
        }
        if(this->state==RUNNING)
        {
            this->state = BLOCKED;
            return 2;
        }
        this->state = BLOCKED;
        return SUCCESS;
    }

    void schedule() { this->state = RUNNING; }

    void preempt() { this->state = READY;
    }

    void resume() { this->state = READY; }

    int sleep(int wake_time) {
        if (this->is_main)
        {
            return FAIL;
        }
        this->is_asleep = true;
        this->wake_up_time = wake_time;
        int bookmark;
        if(this->state==RUNNING)
        {
            return 2;
        }
        return SUCCESS;
    }

    void wake_up() {
        this->is_asleep = false;
        this->wake_up_time = NEVER;
        if(this->state!=BLOCKED)
        {
            this->state = READY;
        }
    }

};