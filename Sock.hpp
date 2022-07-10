#pragma once

#include<iostream>
#include<sys/socket.h>
#include<sys/types.h>
#include<sys/stat.h>
#include<sys/sendfile.h>
#include<sys/wait.h>
#include<netinet/in.h>
#include<arpa/inet.h>
#include<fcntl.h>
#include<signal.h>
#include<unistd.h>
#include<stdlib.h>
#include<string.h>
#include<string>
#include<sstream>
#include<vector>
#include<unordered_map>
#include"Log.hpp"

#define BACKLOG 5

class Sock{
  public:
    static int Socket()
    {
      int fd = socket(AF_INET, SOCK_STREAM, 0);
      if(fd < 0){
        LOG(Fatal, "socket error!");
        exit(SocketErr);
      }
      return fd;
    }
    static void Bind(int fd, int port)
    {
      struct sockaddr_in local;
      local.sin_addr.s_addr = htonl(INADDR_ANY);
      local.sin_family = AF_INET;
      local.sin_port = htons(port);
      
      if(bind(fd, (struct sockaddr*)&local, sizeof(local)) < 0){
      LOG(Fatal, "bind error!");
        exit(BindErr);
      }
    }
    static void Listen(int fd)
    {
      if(listen(fd, BACKLOG) < 0){
        LOG(Fatal, "listen error!");
        exit(ListenErr);
      }
    }
    static void SetSockOpt(int fd)
    {
      int opt = 1;
      setsockopt(fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));
    }
    static int Accept(int fd)
    {
      struct sockaddr_in peer;
      socklen_t len = sizeof(peer);
      int s = accept(fd, (struct sockaddr*)&peer, &len);
      if (s < 0){
        LOG(Warning, "accept error!");
      }
      return s;
    }
};
