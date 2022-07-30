#include"Sock.hpp"
#include"Log.hpp"

class Util{
  public:
    static void GetLine(int fd, std::string &line)
    {
      //换行有\r \n \r\n三种情况
      char c = '\n';
      do{
        ssize_t s = recv(fd, &c, 1, 0);
        if(s > 0){
          if(c == '\r'){
            ssize_t ss = recv(fd, &c, 1, MSG_PEEK);
            if(ss > 0){
              if(c == '\n'){
                recv(fd, &c, 1, 0);
              }
            }
            c = '\n';
          }
          line += c;
        }
        std::cout<< c;
      }while(c != '\n');
    }
    static void StringParse(const std::string& request_line, std::string& method, std::string& uri, std::string& version)
    {
      std::stringstream ss(request_line);
      ss >> method >> uri >> version;
    }
    static void MakeStringToKV(std::string line, std::string& k, std::string& v)
    {
      //std::stringstream s(line);
      //s >> k >> v;
      std::size_t pos = line.find(": ");
      if(std::string::npos != pos){
        k = line.substr(0, pos);
        v = line.substr(pos+2);
      }
    }
};
