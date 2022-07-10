#pragma once

#include<pthread.h>
#include"Sock.hpp"
#include"Log.hpp"
#include"Protocol.hpp"
#include"ThreadPool.hpp"

#define PORT 8080

class HttpServer{
  private:
    int fd;
    int port;
    ThreadPool* tp;
  public:
    HttpServer(int _port = PORT):port(_port), fd(-1), tp(nullptr)
    {}
    void InitServer()
    {
      signal(SIGPIPE, SIG_IGN);//当客户端请求资源时，服务器在通过写给客户端，客户端强行关闭时会关闭服务器相应的文件，会导致写入数据失败进而会收到信号导致进程崩溃
      fd = Sock::Socket();
      Sock::SetSockOpt(fd);//端口复用
    
      Sock::Bind(fd, port);
      Sock::Listen(fd);
      tp = new ThreadPool();
    }
    void Start()
    {
      while(true){
        int ffd = Sock::Accept(fd);
        if(ffd < 0){
          continue;
        }
        std::cout << ffd << std::endl;
        Task* t = new Task(ffd);
        tp->ThreadPoolAddTask(t);
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
      close(fd);
    }
};
