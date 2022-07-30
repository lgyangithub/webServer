#pragma once

#include<pthread.h>
#include"Sock.hpp"
#include"Log.hpp"
#include"Protocol.hpp"
#include"ThreadPool.hpp"
#include"Epoll.hpp"

#define PORT 8080

class HttpServer{
  private:
    int fd;
    int port;
    ThreadPool* tp;
    Epoll* ep;
  public:
    HttpServer(int _port = PORT):port(_port), fd(-1), tp(nullptr), ep(nullptr)
    {}
    void InitServer()
    {
      signal(SIGPIPE, SIG_IGN);//当客户端请求资源时，服务器在通过写给客户端，客户端强行关闭时会关闭服务器相应的文件，会导致写入数据失败进而会收到信号导致进程崩溃
      fd = Sock::Socket();
      Sock::SetSockOpt(fd);//端口复用
    
      Sock::Bind(fd, port);
      Sock::Listen(fd);
      tp = new ThreadPool();
      ep = new Epoll(fd);
    }
    void Start()
    {
      ep->StartEpoll();
      while(true){
        int state = ep->EpollWait();
        if(state == - 1) {
          exit(-1);
        }

        struct epoll_event* events= ep->GetEvents();
        for(int i = 0; i < state; i++) {
          if(events[i].data.fd == fd) {
            int client_fd = Sock::Accept(fd);
            ep->EpollAdd(client_fd);
          }
          else {
            char* c;
            int client_fd = events[i].data.fd; 
            int ret = recv(client_fd, c, 1, MSG_PEEK);
            if(ret <= 0) {
              std::cout << "close:" << client_fd << std::endl;
              ep->EpollDel(client_fd);
              close(client_fd);
            }
            else {
              Task* t = new Task(client_fd);
              tp->ThreadPoolAddTask(t);
            }
          }
        }
/*
        int ffd = Sock::Accept(fd);
        if(ffd < 0){
          continue;
        }
        //std::cout << ffd << std::endl;
        Task* t = new Task(ffd);
        tp->ThreadPoolAddTask(t);
 */      
        //tp->PushTask(t);
    /*
        pthread_t tid;
        pthread_create(&tid, nullptr, Entry::HandlerHttp, &ffd);
        pthread_detach(tid);
        */
      }
    }
    ~HttpServer()
    {
      tp->~ThreadPool();
      ep->~Epoll();
      close(fd);
    }
};
