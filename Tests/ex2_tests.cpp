//
// Created by barcavia123 on 02/04/2022.
//
#include <iostream>
#include "uthreads.h"

void thread1(void) {
    std::cout << "starting thread 1" << std::endl;
    int tid = uthread_get_tid();
    int time = uthread_get_quantums(tid);
    for (int i = 0; i < 1000000; ++i) {
        //        std::cout << "thread1 << std::endl;
        int t = uthread_get_quantums(tid);
        if (time != t) {
            std::cout << "thread " << tid << " just started again for the " << t << " time" << std::endl;
            time = t;
        }
    }
    std::cout << "terminating thread 1" << std::endl;
    uthread_terminate(tid);
}

void thread2(void) {
    std::cout << "starting thread 2" << std::endl;
    int tid = uthread_get_tid();
    int time = uthread_get_quantums(tid);
    for (int i = 0; i < 2000000; ++i) {
        int t = uthread_get_quantums(tid);
        if (time != t) {
            std::cout << "thread " << tid << " just started again for the " << t << " time" << std::endl;
            time = t;
        }
    }
    std::cout << "terminating thread 2" << std::endl;
    uthread_terminate(tid);
}

void thread3(void) {
    std::cout << "starting thread 3" << std::endl;
    for (int i = 0; i < 500000; ++i) {
        //        std::cout << "thread3" << std::endl;
    }
    int cur_time = uthread_get_total_quantums();
    std::cout << "thread 3 going to sleep on time " << cur_time << std::endl;
    uthread_sleep(1);
    cur_time = uthread_get_total_quantums();
    std::cout << "thread 3 waking up on time " << cur_time << std::endl;
    for (int i = 0; i < 5000; ++i) {
        //        std::cout << "thread3" << std::endl;
        for (int j = 0; j < 1000; ++j) {

        }
    }
    std::cout << "terminating thread 3" << std::endl;
    int tid = uthread_get_tid();
    uthread_terminate(tid);
}

void thread4(void) {
    int thid = 1;
    std::cout << "starting thread " << thid << std::endl;
    std::cout << "thread " << thid << " blocking thread 2" << std::endl;
    int res = uthread_block(2);
    for (int i = 0; i < 5000; ++i) {
        //        std::cout << "thread2" << std::endl;
        for (int j = 0; j < 1000; ++j) {

        }
    }
    std::cout << "terminating thread " << thid << std::endl;
    int tid = uthread_get_tid();
    uthread_terminate(tid);
}

void thread5(void) {
    int thid = 2;
    std::cout << "starting thread " << thid << std::endl;
    for (int i = 0; i < 5000; ++i) {
        //        std::cout << "thread2" << std::endl;
        for (int j = 0; j < 100000; ++j) {

        }
    }

    std::cout << "terminating thread " << thid << std::endl;
    int tid = uthread_get_tid();
    uthread_terminate(tid);
}

void thread6(void) {
    int thid = 3;
    std::cout << "starting thread " << thid << std::endl;
    for (int i = 0; i < 5000; ++i) {
        //        std::cout << "thread2" << std::endl;
        for (int j = 0; j < 1000; ++j) {

        }
    }
    std::cout << "thread " << thid << " resuming thread 2" << std::endl;
    int res = uthread_resume(2);
    for (int i = 0; i < 5000; ++i) {
        //        std::cout << "thread2" << std::endl;
        for (int j = 0; j < 100000; ++j) {

        }
    }
    std::cout << "terminating thread " << thid << std::endl;
    int tid = uthread_get_tid();
    uthread_terminate(tid);
}


void thread7(void) {
    int tid = uthread_get_tid();
    std::cout << "starting thread " << tid << std::endl;
    int time = uthread_get_quantums(tid);
    std::cout << "thread " << tid << " blocked itself" << std::endl;
    int res = uthread_block(tid);
    for (int i = 0; i < 4000000; ++i) {
        int t = uthread_get_quantums(tid);
        if (time != t) {
            std::cout << "thread " << tid << " just started again for the " << t << " time" << std::endl;
            time = t;
        }
    }
    std::cout << "terminating thread " << tid << std::endl;
    uthread_terminate(tid);
}

void thread8(void) {
    int tid = uthread_get_tid();
    std::cout << "starting thread " << tid << std::endl;
    int time = uthread_get_quantums(tid);
    for (int i = 0; i < 1000000; ++i) {
        int t = uthread_get_quantums(tid);
        if (time != t) {
            std::cout << "thread " << tid << " just started again for the " << t << " time" << std::endl;
            time = t;
        }
    }
    std::cout << "thread " << tid << " resumed thread 1" << std::endl;
    int res = uthread_resume(1);
    std::cout << "terminating thread " << tid << std::endl;
    uthread_terminate(tid);
}

void thread9(void) {
    int tid = uthread_get_tid();
    std::cout << "starting thread " << tid << std::endl;
    int time = uthread_get_quantums(tid);
    std::cout << "thread " << tid << " going to sleep" << std::endl;
    int res = uthread_sleep(1);
    for (int i = 0; i < 1000000; ++i) {
        int t = uthread_get_quantums(tid);
        if (time != t) {
            std::cout << "thread " << tid << " just started again for the " << t << " time" << std::endl;
            time = t;
        }
    }
    std::cout << "terminating thread " << tid << std::endl;
    uthread_terminate(tid);
}

void thread10(void) {
    int tid = uthread_get_tid();
    std::cout << "starting thread " << tid << std::endl;
    int time = uthread_get_quantums(tid);
    //std::cout << "thread "<<tid <<" blocking thread " <<1<<std::endl;
    //int res = uthread_block(1);
    for (int i = 0; i < 1000000; ++i) {
        int t = uthread_get_quantums(tid);
        if (time != t) {
            std::cout << "thread " << tid << " just started again for the " << t << " time" << std::endl;
            time = t;
        }
    }
    //std::cout << "thread "<<tid <<" resuming thread " <<1<<std::endl;
    //int res1 = uthread_resume(1);
    std::cout << "terminating thread " << tid << std::endl;
    uthread_terminate(tid);
}

void thread11(void) {
    int tid = uthread_get_tid();
    std::cout << "starting thread " << tid << std::endl;
    int time = uthread_get_quantums(tid);
    for (int i = 0; i < 1000000; ++i) {
        int t = uthread_get_quantums(tid);
        if (time != t) {
            std::cout << "thread " << tid << " just started again for the " << t << " time" << std::endl;
            time = t;
        }
    }
    std::cout << "terminating thread " << tid << std::endl;
    uthread_terminate(tid);
}

void thread12(void) {
    int tid = uthread_get_tid();
    std::cout << "starting thread " << tid << std::endl;
    int time = uthread_get_quantums(tid);
    std::cout << "thread " << tid << " terminates thread "<<1<<std::endl;
    int res = uthread_terminate(1);
    std::cout << "thread " << tid << " tryes to get quanums of "<<1<<std::endl;
    int res2 = uthread_get_quantums(1);

    for (int i = 0; i < 1000000; ++i) {
        int t = uthread_get_quantums(tid);
        if (time != t) {
            std::cout << "thread " << tid << " just started again for the " << t << " time" << std::endl;
            time = t;
        }
    }
    std::cout << "terminating thread " << tid << std::endl;
    uthread_terminate(tid);
}

void test_noraml_run() {
    std::cout << "uthread init main thread" << std::endl;
    uthread_init(100000);
    std::cout << "spawn thread 1 main thread" << std::endl;
    int t1 = uthread_spawn(thread1);
    std::cout << "spawn thread 2 main thread" << std::endl;
    int t2 = uthread_spawn(thread2);
    std::cout << "main thread waiting" << std::endl;
    for (int i = 0; i < 500000; ++i) {
        //std::cout << "main thread" << std::endl;
        for (int j = 0; j < 1000; ++j) {

        }
    }
    std::cout << "terminating main thread" << std::endl;
    uthread_terminate(0);
}

void test_sleep_run() {
    std::cout << "uthread init main thread" << std::endl;
    uthread_init(150000);
    std::cout << "spawn thread 3 main thread" << std::endl;
    int t3 = uthread_spawn(thread3);
    std::cout << "spawn thread 1 main thread" << std::endl;
    int t1 = uthread_spawn(thread1);
    std::cout << "spawn thread 2 main thread" << std::endl;
    int t2 = uthread_spawn(thread2);
    std::cout << "main thread waiting" << std::endl;
    for (int i = 0; i < 500000; ++i) {
        //std::cout << "main thread" << std::endl;
        for (int j = 0; j < 10000; ++j) {

        }
    }
    std::cout << "terminating main thread" << std::endl;
    uthread_terminate(0);
}

void test_block_run() {
    std::cout << "uthread init main thread- block run" << std::endl;
    uthread_init(100000);
    std::cout << "spawn thread 1 main thread" << std::endl;
    int t1 = uthread_spawn(thread4); // thread 1
    std::cout << "spawn thread 2 main thread" << std::endl;
    int t2 = uthread_spawn(thread5); // thread 2
    std::cout << "spawn thread 3 main thread" << std::endl;
    int t3 = uthread_spawn(thread6); // thread 3
    std::cout << "main thread waiting" << std::endl;
    for (int i = 0; i < 500000; ++i) {
        //std::cout << "main thread" << std::endl;
        for (int j = 0; j < 10000; ++j) {

        }
    }
    std::cout << "terminating main thread" << std::endl;
    uthread_terminate(0);
}

void test_block_itself() {
    std::cout << "uthread init main thread- block itself" << std::endl;
    uthread_init(150000);
    int t1 = uthread_spawn(thread7); // thread 1
    std::cout << "spawn thread " << t1 << " main thread" << std::endl;
    int t2 = uthread_spawn(thread8); // thread 2
    std::cout << "spawn thread " << t2 << " main thread" << std::endl;
    std::cout << "main thread waiting" << std::endl;
    for (int i = 0; i < 1000000000; ++i) {
        //std::cout << "main thread" << std::endl;
    }
    std::cout << "terminating main thread" << std::endl;
    uthread_terminate(0);
}

void test_block_sleep() {
    std::cout << "uthread init main thread- block sleep" << std::endl;
    uthread_init(100000);
    int t1 = uthread_spawn(thread9); // thread 1
    std::cout << "spawn thread " << t1 << " main thread" << std::endl;
    int t2 = uthread_spawn(thread10); // thread 2
    std::cout << "spawn thread " << t2 << " main thread" << std::endl;
    std::cout << "main thread waiting" << std::endl;
    for (int i = 0; i < 1000000000; ++i) {
        //std::cout << "main thread" << std::endl;
    }
    std::cout << "terminating main thread" << std::endl;
    uthread_terminate(0);
}

void test_termination_of_other_thread() {
    std::cout << "uthread init main thread- block sleep" << std::endl;
    uthread_init(100000);
    int t1 = uthread_spawn(thread11); // thread 1
    std::cout << "spawn thread " << t1 << " main thread" << std::endl;
    int t2 = uthread_spawn(thread12); // thread 2
    std::cout << "spawn thread " << t2 << " main thread" << std::endl;
    std::cout << "main thread waiting" << std::endl;
    for (int i = 0; i < 1000000000; ++i) {
        //std::cout << "main thread" << std::endl;
    }
    std::cout << "terminating main thread" << std::endl;
    uthread_terminate(0);
}

int main(void) {
    //test_noraml_run();
    //test_sleep_run();
    //test_block_run();
    test_block_itself();
    //test_block_sleep();
    //test_termination_of_other_thread();
    return 0;
}
