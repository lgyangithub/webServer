#include<sys/epoll.h>
#include<fcntl.h>
#include<unistd.h>
#include "Log.hpp"

#define EPOLL_SIZE 100

class Epoll {
public:
  Epoll(int fd) 
  : listen_fd(fd)
  , events(new struct epoll_event[EPOLL_SIZE])
  {}

  ~Epoll() {
    delete events;
  }
  
  int StartEpoll() {
    //std::cout << "start epoll" << std::endl;
    epoll_fd = epoll_create(EPOLL_SIZE);
    
    struct epoll_event event;
    event.data.fd = listen_fd;
    event.events = EPOLLIN;//事件触发模式
    return epoll_ctl(epoll_fd, EPOLL_CTL_ADD, listen_fd, &event);
  }
  
  int EpollAdd(int fd) {
    struct epoll_event event;
    event.data.fd = fd;
    event.events = EPOLLIN | EPOLLET;
    //std::cout << "Epoll add\n";
    SetNonBlockingMode(fd);//水平触发模式
    return epoll_ctl(epoll_fd, EPOLL_CTL_ADD, fd, &event);
  }

  int EpollDel(int fd) {
    return epoll_ctl(epoll_fd, EPOLL_CTL_DEL, fd, nullptr);
  }

  int EpollWait() {
    int state =  epoll_wait(epoll_fd, events, EPOLL_SIZE, -1);
    if(state == -1) {
      LOG(Fatal, "epoll wait error");
      exit( -1 );
    }
    return state;
  }

  struct epoll_event* GetEvents() {
    return events;
  }

  void SetNonBlockingMode(int fd) {
    int flag = fcntl(fd, F_GETFL, 0);
    fcntl(fd, F_SETFL, flag | O_NONBLOCK);
  }
private:
  struct epoll_event* events;
  struct epoll_event event;
  int listen_fd;
  int epoll_fd;
};
