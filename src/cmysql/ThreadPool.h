/*
 * ThreadPoolTest.h
 *
 *  Created on: 2014-8-17
 *      Author: imdb
 */

#ifndef THREADPOOLTEST_H_
#define THREADPOOLTEST_H_

#include <iostream>
#include <pthread.h>
#include <queue>
#include <semaphore.h>
#include <stdlib.h>
//#include "c_mysql_server.h"

using namespace std;

struct Args;

class ThreadPool
{
private:
	struct Task{
		Task();
		Task(void (*f)(void *), void *a, bool e):func(f),arg(a),end(e){}

		void (*func)(void *arg);	// pointer to function
		void *arg;						// the parameter of function
		bool end;						// whether exit thread
		static void destroy_task(Task *task){
			// todo:whether delete
//			delete (Args*)task->arg;

			delete task;
		};
	};
public:
	ThreadPool();
	virtual ~ThreadPool();

	bool Thread_Pool_init(int thread_count_in_pool_);
	static void *thread_exec(void *arg);
	void *thread_exec_with_cond(void *arg);

	//arg can't be a class, because delete void * won't execute destructor function
	void add_task(void (*)(void *), void *arg, bool e = false);
	static void destroy_pool(ThreadPool * tp);	// destroy pool after every thread exit normally
	//todo: add a function that destroys pool immediately

private:
//	pthread_cond_t free_thread_cond;
//	pthread_mutex_t cond_lock;

	sem_t undo_task_sem;	// undo task count

	pthread_t *thread_list_;

	queue<Task*> task_queue_;
	pthread_mutex_t task_queue_lock;

	int thread_count;
	int free_thread_count;
	pthread_mutex_t free_thread_count_lock;

	int task_count;
	int undo_task_count;
	pthread_mutex_t undo_task_count_lock;

};

#endif /* THREADPOOLTEST_H_ */
