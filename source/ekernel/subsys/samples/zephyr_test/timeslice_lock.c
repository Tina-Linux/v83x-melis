/*
 * =====================================================================================
 *
 *       Filename:  timeslice_lock.c
 *
 *    Description:
 *
 *        Version:  2.0
 *         Create:  2017-12-01 17:23:04
 *       Revision:  none
 *       Compiler:  gcc version 6.3.0 (crosstool-NG crosstool-ng-1.23.0)
 *
 *         Author:  caozilong@allwinnertech.com
 *   Organization:  BU1-PSW
 *  Last Modified:  2017-12-02 11:39:02
 *
 * =====================================================================================
 */

#include "timeslice_lock.h"
#include "test.h"

#define THREADS_NUM     3
static K_THREAD_STACK_ARRAY_DEFINE(tstack, THREADS_NUM, STACK_SIZE);

static struct thread_data tdata[THREADS_NUM];
static struct k_thread tthread[THREADS_NUM];
static int old_prio, init_prio;

static void thread_entry(void *p1, void *p2, void *p3)
{
    int sleep_ms = (int)p2;

    if (sleep_ms > 0)
    {
        k_sleep(sleep_ms);
    }

    int tnum = (int)p1;

    tdata[tnum].executed = 1;
}

static void setup_threads(void)
{
    int i;
    old_prio = k_thread_priority_get(k_current_get());
    for (i = 0; i < THREADS_NUM; i++)
    {
        if (i == 0)
        {
            /* spawn thread with higher priority */
            tdata[i].priority = init_prio - 1;
        }
        else if (i == 1)
        {
            /* spawn thread with same priority */
            tdata[i].priority = init_prio;
        }
        else
        {
            /* spawn thread with lower priority */
            tdata[i].priority = init_prio + 1;
        }
        tdata[i].executed = 0;
    }
    k_thread_priority_set(k_current_get(), init_prio);
}

static void spawn_threads(int sleep_sec)
{
    int i;
    for (i = 0; i < THREADS_NUM; i++)
    {
        tdata[i].tid = k_thread_create(&tthread[i], tstack[i],
                                       STACK_SIZE, thread_entry,
                                       (void *)i, (void *)sleep_sec,
                                       NULL, tdata[i].priority, 0, 0);
    }
}

static void teardown_threads(void)
{
    int i;
    for (i = 0; i < THREADS_NUM; i++)
    {
        k_thread_abort(tdata[i].tid);
    }
    k_thread_priority_set(k_current_get(), old_prio);
}

/*test cases*/
void test_yield_cooperative(void)
{
    int i;
    /* set current thread to a cooperative priority */
    init_prio = -1;
    setup_threads();

    spawn_threads(0);
    /* checkpoint: only higher priority thread get executed when yield */
    k_yield();
    zassert_true(tdata[0].executed == 1, NULL);
    zassert_true(tdata[1].executed == 1, NULL);
    for (i = 2; i < THREADS_NUM; i++)
    {
        zassert_true(tdata[i].executed == 0, NULL);
    }
    /* restore environment */
    teardown_threads();
}

void test_sleep_cooperative(void)
{
    int i;
    /* set current thread to a cooperative priority */
    init_prio = -1;
    setup_threads();

    spawn_threads(0);
    /* checkpoint: all ready threads get executed when k_sleep */
    k_sleep(100);
    for (i = 0; i < THREADS_NUM; i++)
    {
        zassert_true(tdata[i].executed == 1, NULL);
    }

    /* restore environment */
    teardown_threads();
}

void test_busy_wait_cooperative(void)
{
    int i;
    /* set current thread to a preemptible priority */
    init_prio = -1;
    setup_threads();

    spawn_threads(0);
    k_busy_wait(100000); /* 100 ms */
    /* checkpoint: No other threads get executed */
    for (i = 0; i < THREADS_NUM; i++)
    {
        zassert_true(tdata[i].executed == 0, NULL);
    }
    /* restore environment */
    teardown_threads();
}

void test_sleep_wakeup_preemptible(void)
{
    int i;
    /* set current thread to a preemptible priority */
    init_prio = 0;
    setup_threads();

    spawn_threads(10 * 1000); /* 10 second */
    /* checkpoint: lower threads not executed, high threads are in sleep */
    for (i = 0; i < THREADS_NUM; i++)
    {
        zassert_true(tdata[i].executed == 0, NULL);
    }
    k_wakeup(tdata[0].tid);
    zassert_true(tdata[0].executed == 1, NULL);
    /* restore environment */
    teardown_threads();
}

void test_time_slicing_preemptible(void)
{
    int i;
    /* set current thread to a preemptible priority */
    init_prio = 0;
    setup_threads();

    k_sched_time_slice_set(200, 0); /* 200 ms */
    spawn_threads(0);
    /* checkpoint: higher priority threads get executed immediately */
    zassert_true(tdata[0].executed == 1, NULL);
    k_busy_wait(500000); /* 500 ms */
    /* checkpoint: equal priority threads get executed every time slice */
    zassert_true(tdata[1].executed == 1, NULL);
    for (i = 2; i < THREADS_NUM; i++)
    {
        zassert_true(tdata[i].executed == 0, NULL);
    }

    /* restore environment */
    k_sched_time_slice_set(0, 0); /* disable time slice */
    teardown_threads();
}

void test_time_slicing_disable_preemptible(void)
{
    int i;
    /* set current thread to a preemptible priority */
    init_prio = 0;
    setup_threads();

    spawn_threads(0);
    /* checkpoint: higher priority threads get executed immediately */
    zassert_true(tdata[0].executed == 1, NULL);
    k_busy_wait(500000); /* 500 ms */
    /* checkpoint: equal priority threads get executed every time slice */
    zassert_true(tdata[1].executed == 0, NULL);
    for (i = 2; i < THREADS_NUM; i++)
    {
        zassert_true(tdata[i].executed == 0, NULL);
    }
    /* restore environment */
    teardown_threads();
}

void test_lock_preemptible(void)
{
    int i;
    /* set current thread to a preemptible priority */
    init_prio = 0;
    setup_threads();

    k_sched_lock();
    spawn_threads(0);
    /* do critical thing */
    k_busy_wait(100000);
    /* checkpoint: all other threads not been executed */
    for (i = 0; i < THREADS_NUM; i++)
    {
        zassert_true(tdata[i].executed == 0, NULL);
    }
    /* make current thread unready */
    k_sleep(100);
    /* checkpoint: all other threads get executed */
    for (i = 0; i < THREADS_NUM; i++)
    {
        zassert_true(tdata[i].executed == 1, NULL);
    }
    /* restore environment */
    k_sched_unlock();
    teardown_threads();
}

void test_unlock_preemptible(void)
{
    int i;
    /* set current thread to a preemptible priority */
    init_prio = 0;
    setup_threads();

    k_sched_lock();
    spawn_threads(0);
    /* do critical thing */
    k_busy_wait(100000);
    /*software_break();*/
    k_sched_unlock();
    /* checkpoint: higher threads get executed */
    zassert_true(tdata[0].executed == 1, NULL);
    for (i = 1; i < THREADS_NUM; i++)
    {
        zassert_true(tdata[i].executed == 0, NULL);
    }
    /* restore environment */
    teardown_threads();
}
