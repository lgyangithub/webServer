#include<iostream>
#include<string>
#include<stdlib.h>
#include<strings.h>
#include<unistd.h>

void CalDate(const std::string& s)
{
  std::string s1, s2;
  int x = 0, y = 0;
  size_t pos = s.find("&");
  if(pos != std::string::npos){
    s1 = s.substr(0, pos);
    s2 = s.substr(pos + 1);
  }
  pos = s1.find("=");
  if(pos != std::string::npos){
    x = atoi(s1.substr(pos +1).c_str());
  }
  pos = s2.find("=");
  if(pos != std::string::npos){
    y = atoi(s2.substr(pos + 1).c_str());
  }

  std::cout << "<html>" << std::endl;
  std::cout << "<h1>" << x << "+" << y << "=" << x + y << "</h1>" << std::endl;
  std::cout << "<h1>" << x << "-" << y << "=" << x - y << "</h1>" << std::endl;
  std::cout << "<h1>" << x << "*" << y << "=" << x * y << "</h1>" << std::endl;
  if(y != 0){
    std::cout << "<h1>" << x << "/" << y << "=" << x / y << "</h1>" << std::endl;
  }
  else{
    std::cout<< "<h1>" << "dividend is 0!" << "</h1>" << std::endl;
  }
  std::cout << "</html>" <<std::endl;
}

int main()
{
  //std::cout << "hello!\n";
  std::string method;
  std::string query_string;
  if(getenv("METHOD")){
    method = getenv("METHOD");
  }
  else{
    return 1;
  }

  if(strcasecmp(method.c_str(), "GET") == 0){
    query_string = getenv("QUERY_STRING");
  }
  else if(strcasecmp(method.c_str(), "POST") == 0){
    //cgi 0 读取
    //cgi 1 写入
    int content_length = atoi(getenv("CONTENT-LENGTH"));
    char c = 'C';
    while(content_length--){
      read(0, &c, 1);
      query_string.push_back(c);
    }
  }
  CalDate(query_string);
  //std::cout << "test_cgi#\nmethod:" << method << std::endl << "quert_string:" << query_string << std::endl;
  return 0;
}
