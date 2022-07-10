#pragma once 

#include<iostream>
#include<string>
#include<sys/time.h>

#define Notice  1
#define Warning 2
#define Error   3
#define Fatal   4

enum ERR{
  SocketErr=1,
  BindErr,
  ListenErr,
  AcceptErr
};

#define LOG(level, message) Log(#level, message, __FILE__, __LINE__)

void Log(std::string level, std::string message, std::string filename, size_t line)
{
  struct timeval t;
  gettimeofday(&t, nullptr);
  std::cout << "[" << level << "][" << message << "][" << filename << "][" << line << "]" << std::endl;
}
