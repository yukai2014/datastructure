/*
 * ThreadPoolTest.cpp
 *
 *  Created on: 2014-8-17
 *      Author: imdb
 */
#include "ThreadPool.h"

ThreadPool::ThreadPool(){

}

ThreadPool::~ThreadPool(){

}

bool ThreadPool::Thread_Pool_init(int thread_count_in_pool_){
	bool success = true;
	thread_count = thread_count_in_pool_;
	free_thread_count = thread_count;
	undo_task_count = 0;

	pthread_mutex_init(&free_thread_count_lock, NULL);
	pthread_mutex_init(&undo_task_count_lock, NULL);
	pthread_mutex_init(&task_queue_lock, NULL);

	sem_init(&undo_task_sem, 0, 0);	// init semaphore

	thread_list_ = (pthread_t*)malloc(thread_count_in_pool_ * sizeof(pthread_t));
	while (!task_queue_.empty()){
		task_queue_.pop();
	}

	for (int i = 0;  i < thread_count; ++ i) {
		if (pthread_create(&thread_list_[i], NULL, thread_exec, this) != 0){	//if any failed, return false
			cout<<"ERROR: create pthread failed!"<<endl;
			success = false;
			break;
		}
		++free_thread_count;
	}
	return false;
}

void ThreadPool::add_task(void (*f)(void *), void *a, bool e){
	Task *t = new Task(f, a, e);
	pthread_mutex_lock(&task_queue_lock);
	task_queue_.push(t);
	pthread_mutex_unlock(&task_queue_lock);

	sem_post(&undo_task_sem);
}

//todo: if task is more than thread, create some thread that execute function once

void *ThreadPool::thread_exec(void *arg){
	ThreadPool *thread_pool = (ThreadPool*)arg;
	Task *task = NULL;

	// every thread execute a endless loop, waiting for task, and exit when receive a task with end member of 'true'
	while (1){
		sem_wait(&(thread_pool->undo_task_sem));

		pthread_mutex_lock(&(thread_pool->task_queue_lock));
		if (!thread_pool->task_queue_.empty()){
			task = thread_pool->task_queue_.front();
			thread_pool->task_queue_.pop();
		}
		pthread_mutex_unlock(&(thread_pool->task_queue_lock));

		if (task != NULL){
			if (task->end)	//it means destory this thread
				break;

			(*(task->func))(task->arg);

			Task::destroy_task(task);
			task = NULL;
		}

//		sem_post(&task_sem);
	}
	pthread_exit(NULL);
	return NULL;
}
/*
void *ThreadPool::thread_exec_with_cond(void *arg){
	ThreadPool *thread_pool = (ThreadPool*)arg;
	Task *task = new Task();

	while (1){
		pthread_mutex_lock(&cond_lock);
		while (free_thread_count == 0){
			pthread_cond_wait(&free_thread_cond, &cond_lock);
		}

		pthread_mutex_lock(&free_thread_count_lock);
		--free_thread_count;
		pthread_mutex_unlock(&free_thread_count_lock);

		pthread_mutex_unlock(&cond_lock);

		pthread_mutex_lock(&task_queue_lock);
		if (!thread_pool->task_queue_.empty()){
			task = thread_pool->task_queue_.pop();
		}
		pthread_mutex_unlock(&task_queue_lock);

		(*(task->func))(task->args);

		pthread_mutex_lock(&free_thread_count_lock);
		++free_thread_count;
		pthread_mutex_unlock(&free_thread_count_lock);
	}

}
*/

void ThreadPool::destroy_pool(ThreadPool *tp){
	//destory every thread
	for (int i = 0; i < tp->thread_count; ++i){	// send destory task to every thread
		tp->add_task(NULL, NULL, true);
	}
	for (int i = 0; i < tp->thread_count; ++i){
		pthread_join(tp->thread_list_[i], NULL);
	}
	while (!tp->task_queue_.empty()){
		Task *temp = tp->task_queue_.front();
		tp->task_queue_.pop();
		Task::destroy_task(temp);
	}

	sem_destroy(&tp->undo_task_sem);
	pthread_mutex_destroy(&tp->free_thread_count_lock);
	pthread_mutex_destroy(&tp->undo_task_count_lock);
	pthread_mutex_destroy(&tp->task_queue_lock);

}

