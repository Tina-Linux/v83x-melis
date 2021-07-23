/*
 *   Copyright (c) 2014 - 2019 Oleh Kulykov <info@resident.name>
 *
 *   Permission is hereby granted, free of charge, to any person obtaining a copy
 *   of this software and associated documentation files (the "Software"), to deal
 *   in the Software without restriction, including without limitation the rights
 *   to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 *   copies of the Software, and to permit persons to whom the Software is
 *   furnished to do so, subject to the following conditions:
 *
 *   The above copyright notice and this permission notice shall be included in
 *   all copies or substantial portions of the Software.
 *
 *   THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 *   IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 *   FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 *   AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 *   LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 *   OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 *   THE SOFTWARE.
 */


#include "librws.h"
#include "rws_thread.h"
#include "rws_memory.h"
#include "rws_common.h"

#include <assert.h>

#include <pthread.h>
#include <unistd.h>

struct rws_thread_struct {
	rws_thread_funct thread_function;
	void * user_object;
	pthread_t thread;
};

typedef struct _rws_threads_joiner_struct {
	rws_thread thread;
	rws_mutex mutex;
} _rws_threads_joiner;

static _rws_threads_joiner * _threads_joiner = NULL;
static void rws_threads_joiner_clean(void) { // private
	rws_thread t = _threads_joiner->thread;
	void * r = NULL;

	if (!t) {
		return;
	}
	_threads_joiner->thread = NULL;

	pthread_join(t->thread, &r);
    assert(r == NULL);

	rws_free(t);
}

static void rws_threads_joiner_add(rws_thread thread) { // public
	rws_mutex_lock(_threads_joiner->mutex);
	rws_threads_joiner_clean();
	_threads_joiner->thread = thread;
	rws_mutex_unlock(_threads_joiner->mutex);
}

static void rws_threads_joiner_create_ifneed(void) {
	if (_threads_joiner) {
		return;
	}
	_threads_joiner = (_rws_threads_joiner *)rws_malloc_zero(sizeof(_rws_threads_joiner));
	_threads_joiner->mutex = rws_mutex_create_recursive();
}

static void * rws_thread_func_priv(void * some_pointer) {
	rws_thread t = (rws_thread)some_pointer;
	t->thread_function(t->user_object);
	rws_threads_joiner_add(t);

	return NULL;
}

rws_thread rws_thread_create(rws_thread_funct thread_function, void * user_object) {
	rws_thread t = NULL;
	int res = -1;
	pthread_attr_t attr;

	if (!thread_function) {
		return NULL;
	}
	rws_threads_joiner_create_ifneed();
	t = (rws_thread)rws_malloc_zero(sizeof(struct rws_thread_struct));
	t->user_object = user_object;
	t->thread_function = thread_function;
	if (pthread_attr_init(&attr) == 0) {
        if (pthread_attr_setstacksize(&attr, 12 * 1024) == 0) {
            //if (pthread_attr_setscope(&attr, PTHREAD_SCOPE_SYSTEM) == 0) {
                struct sched_param sched;
                sched.sched_priority = 30;
                pthread_attr_setschedparam(&attr,&sched);
                if (pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_JOINABLE) == 0) {
                    res = pthread_create(&t->thread, &attr, &rws_thread_func_priv, (void *)t);
                    pthread_setname_np(t->thread, "GnWebsoc");
                }
            //}
        }

		pthread_attr_destroy(&attr);
	}
    assert(res == 0);
    //CHECK_RET_WITH_RET(res == 0, NULL);
	return t;
}

void rws_thread_sleep(const unsigned int millisec) {
	usleep(millisec * 1000); // 1s = 1'000'000 microsec.
}

rws_mutex rws_mutex_create_recursive(void) {
	pthread_mutex_t * mutex = (pthread_mutex_t *)rws_malloc_zero(sizeof(pthread_mutex_t));
	int res = -1;
	pthread_mutexattr_t attr;
	if (pthread_mutexattr_init(&attr) == 0) {
		if (pthread_mutexattr_settype(&attr, PTHREAD_MUTEX_RECURSIVE) == 0) {
			res = pthread_mutex_init(mutex, &attr);
		}
		pthread_mutexattr_destroy(&attr);
	}
        assert(res == 0);
        //aos_check(res == 0, EINVAL);
	return mutex;
}

void rws_mutex_lock(rws_mutex mutex) {
	if (mutex) {
		pthread_mutex_lock((pthread_mutex_t *)mutex);
	}
}

void rws_mutex_unlock(rws_mutex mutex) {
	if (mutex) {
		pthread_mutex_unlock((pthread_mutex_t *)mutex);
	}
}

void rws_mutex_delete(rws_mutex mutex) {
	if (mutex) {
		pthread_mutex_destroy((pthread_mutex_t *)mutex);
		rws_free(mutex);
	}
}

