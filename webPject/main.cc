#include"HttpServer.hpp"
#include"Log.hpp"

int main(int argc, char *argv[])
{
  if(argc > 2) {
    std::cout << "./httpserver port" << std::endl;
    return 0;
  }
  int port = 8080;
  if(argc == 2) {
    port = atoi(argv[1]);
  }
  std::cout << "port:" << port << std::endl;

  HttpServer *hs = new HttpServer(port);
  hs->InitServer();
  hs->Start();
}
