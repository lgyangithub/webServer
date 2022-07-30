#pragma once

#include"Protocol.hpp"
#include<pthread.h>
#include<queue>

#define THREAD_MAX 500
#define QUE_MAX 10

typedef void* (*handler_t)(void*);

class Task {//任务
public:
  Task(int _fd)
  :fd(_fd)
  ,handler(Entry::HandlerHttp)
  {}
  ~Task(){}
  
  void run(){
    handler((void*)&fd);
  }
  
private:
  int fd;
  handler_t handler;
};

class ThreadPool {
public:
  ThreadPool(int _que_max = QUE_MAX, int _thread_max = THREAD_MAX)
  : que_max(_que_max)
  , thread_max(_thread_max)
  , thread_cur(_thread_max)
  , keep_running(1)
  {//初始化线程池,分离线程
    pthread_t* tid = new pthread_t[thread_max];

    pthread_mutex_init(&mutex, nullptr);
    pthread_cond_init(&empty, nullptr);
    pthread_cond_init(&full, nullptr);

    for(int i =  0; i < thread_max; i++) {
      if(pthread_create(&tid[i], nullptr, ThreadRunning, (void*)this) != 0) {
        std::cout << "pthread create error!" << std::endl;
        exit(0);

        pthread_detach(tid[i]);
      }
    }
  }

  ~ThreadPool() {
    pthread_mutex_destroy(&mutex);
    pthread_cond_destroy(&empty);
    pthread_cond_destroy(&full);
  }

  bool ThreadPoolAddTask(Task* task) {//生产者模型

    if(keep_running == 0) return false;

    ThreadLock();

    while(TaskQueueIsFull()) {
      ProdectorWait();
    }

    PushTask(task);
    //std::cout << "new task add to threadpool" << std::endl;
    ConsumerSignal();
    ThreadUnlock();
    return true;
  }

  void StopThreadPool() {//释放线程池资源
    if(keep_running == 0) return;
    ThreadLock();

    keep_running = 0;
    while(thread_cur > 0) {
      ConsumerSignal();//唤醒线程执行完任务，没有任务就时直接释放，有任务执行完再释放
      ProdectorWait();//阻塞生产者
    }
    
    //线程释放完，释放调用线程池线程
    ThreadExit();

    ThreadUnlock();
  }
private:

  pthread_t* tid;
  std::queue<Task*> task_que;//任务队列
  int thread_max;            //线程池线程总数
  int thread_cur;            //当前线程数
  int que_max;               //任务队列最大数量
  int keep_running;          //线程池运行或关闭状态

  pthread_mutex_t mutex;
  pthread_cond_t empty;
  pthread_cond_t full;
  
  void ThreadLock() {
    pthread_mutex_lock(&mutex);
  }

  void ThreadUnlock() {
    pthread_mutex_unlock(&mutex);
  }

  void ProdectorWait() {
    pthread_cond_wait(&full, &mutex);
  }

  void ProdectorSignal() {
    pthread_cond_signal(&full);
  }

  void ConsumerWait() {
    pthread_cond_wait(&empty, &mutex);
  }

  void ConsumerSignal() {
    pthread_cond_signal(&empty);
  }

  bool ThreadIsRunning() {
    return keep_running == 0 ? false : true;
  }

  void ThreadExit() {
    thread_cur--;
    //释放线程
    std::cout << pthread_self() << " thread exit!" << std::endl;
    ProdectorSignal();//唤醒StopThreadPool继续释放
    pthread_exit(nullptr);
  }

  bool TaskQueueIsEmpty() {
    return task_que.empty();
  }

  bool TaskQueueIsFull() {
    return task_que.size() == que_max ?  true :  false;
  }

  void PopTask(Task** task) {
    *task = task_que.front();
    task_que.pop();
  }

  void PushTask(Task* task){
    task_que.push(task);
  }

  static void* ThreadRunning(void* argc) {
    ThreadPool* tp = (ThreadPool*)argc;

    while(true) {
      tp->ThreadLock();
      
      while(tp->ThreadIsRunning() && tp->TaskQueueIsEmpty()) {
        tp->ConsumerWait();
      }

      if(!tp->ThreadIsRunning() && tp->TaskQueueIsEmpty()) {
        tp->ThreadUnlock();
        tp->ThreadExit();
      }

      Task* task;
      tp->PopTask(&task);
      tp->ProdectorSignal();
      tp->ThreadUnlock();

      task->run();

    }
  }

};
