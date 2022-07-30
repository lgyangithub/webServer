#pragma once

#include"Sock.hpp"
#include"Log.hpp"
#include"Util.hpp"

#define WEBROOT "wwwroot"
#define HOMEPAGE "index.html"
#define VERSION "HTTP/1.0"

static std::string CodeToDesc(int code)
{
  std::string desc;
  switch(code){
    case 200:
      desc = "OK";
      break;
    case 404:
      desc = "Not Found";
    default:
      break;
  }
  return desc;
}
static std::string SuffixToDesc(const std::string& suffix)
{
  if(suffix == ".html" || suffix == ".htm"){ 
    return "text/html";
  }
  else if(suffix == ".js"){ 
    return "application/x-javascript";
  }
  else if(suffix == ".css"){ 
    return "text/css";
  }
  else if(suffix == ".jpg"){ 
    return "image/jpeg";
  }
  else if(suffix == ".txt"){
    return "txet/plain";
  }
  else if(suffix == ".mp4"){
    return "video/mepeg4";
  }
  else{ 
    return "text/html";
  }
}
class HttpRequest{
  private:
    std::string request_line;
    std::vector<std::string> request_header;
    std::string blank;
    std::string request_body;
  private:
    std::string method;
    std::string uri;
    std::string version;

    std::unordered_map<std::string, std::string> header_key;

    std::string path;
    std::string query_string;

    int content_length;
    
    ssize_t file_size;

    bool cgi;
  public:
    HttpRequest()
    {
      blank = '\n';
      content_length = -1;
      path = WEBROOT;
      cgi = false;
    }
    void SetRequestLine(const std::string &line)
    {
      request_line = line;
    }
    void SetRequestHeader(const std::string& line)
    {
      request_header.push_back(line);
    }
    void SetRequestBody(const std::string& body)
    {
      request_body = body;
    }
    void SetUri()
    {
      path = uri;
    }
    void SetPath(const std::string& _path)
    {
      path = _path;
    }
    void SetCgi(bool _cgi)
    {
      cgi = _cgi;
    }
    void SetFileSize(ssize_t size)
    {
      file_size = size;
    }
    std::string GetRequestLine()
    {
      return request_line;
    }
    std::string GetMethod()
    {
      return method;
    }
    std::string GetUri()
    {
      return uri;
    }
    std::string GetVersion()
    {
      return version;
    }
    std::string GetPath()
    {
      return path;
    }
    std::string GetQueryString()
    {
      return query_string;
    }
    std::string GetBody()
    {
      return request_body;
    }
    int GetContentLength()
    {
      return content_length;
    }
    ssize_t GetFileSize()
    {
      return file_size;
    }
    std::string MakeSuffix()
    {
      std::string suffix;
      size_t pos = path.find(".");
      if(pos != std::string::npos){
        suffix = path.substr(pos);
      }
      return suffix;
    }
    void RequestParse()
    {
      Util::StringParse(request_line, method, uri, version);
    }
    void RequestHeaderParse()
    {
      for(int i = 0; i < request_header.size(); i++){
        std::string k, v;
        Util::MakeStringToKV(request_header[i], k, v);
        header_key.insert({k, v});
        if(k == "Content-Length"){
          content_length = atoi(v.c_str());
        }
        //std::cout << "k:" << k << std::endl << "v:" << v;
      }
    }
    void UriParse()
    {
      std::size_t pos = uri.find("?");
      if(pos != std::string::npos){
        path += uri.substr(0, pos);
        query_string += uri.substr(pos + 1);
      }
      else{
        path += uri;
      }
    }
    bool IsNeedRecvBody()
    {
      if(strcasecmp(method.c_str(), "POST") == 0 && content_length > 0){
        return true;
      }
      return false;
    }
    bool IsGet()
    {
      if(strcasecmp(method.c_str(), "GET") == 0){
        return true;
      }
      return false;
    }
    bool IsPost()
    {
      if(strcasecmp(method.c_str(), "POST"))return true;
      return false;
    }
    bool IsMethodLegal()
    {
      //post get
      if(strcasecmp(method.c_str(), "POST") == 0 || strcasecmp(method.c_str(), "GET") == 0){
        return true;
      }
      return false;
    }
    bool IsAddHomePage()
    {
      //访问的资源确定资源还是一个目录
      if(path[path.size() - 1] == '/'){
        path += HOMEPAGE;
      }
    }
    bool IsCgi()
    {
      return cgi;
    }
    ~HttpRequest()
    {}
};

class HttpResponse{
  private:
    std::string status_line;
    std::vector<std::string> response_header;
    std::string blank;
    std::string response_body;
  public:
    HttpResponse()
    {
      blank = "\r\n";
    }
    void SetStatusLine(std::string line)
    {
      status_line = line;
    }
    void AddHeader(const std::string& s)
    {
      response_header.push_back(s);
    }
    std::string GetStatusLine()
    {
      return status_line;
    }
    std::vector<std::string> GetResponseHander()
    {
      return response_header;
    }
    std::string GetBlank()
    {
      return blank;
    }
    ~HttpResponse()
    {}
};

class EndPoint{
  private:
    int fd;
    HttpRequest req;
    HttpResponse rep;
  private:
    void GetRequestLine()//获取请求第一行
    {
      std::string line;
      Util::GetLine(fd, line);
      //
      req.SetRequestLine(line);
      
      req.RequestParse();
    }
    void GetRequestHeader()//请求报头
    {
      std::string line;
      do{
        line = "";
        Util::GetLine(fd, line);
        if(line != "\n")req.SetRequestHeader(line);
        //std::cout << line;
      }while(line != "\n" &&  line != "");
      req.RequestHeaderParse();
    }
    void GetRequestBody()//body
    {
      int len = req.GetContentLength();
      char c;
      std::string body;
      while(len--){
        int s = recv(fd, &c, 1, 0);
        body.push_back(c);
      }
      req.SetRequestBody(body);
    }
    void SetResponseStatusLine(int code)//响应第一行
    {
			std::string status_line;
      status_line += VERSION;
      status_line += " ";
      status_line += std::to_string(code);
      status_line += " ";                                                                            
      status_line += CodeToDesc(code);                              
      status_line += "\r\n";
      //blank
      //status_line += rep.GetBlank();
			rep.SetStatusLine(status_line);                                     
    }
    void SetResponseHeaderLine()//响应报头
    {
      std::string suffix = req.MakeSuffix();
      std::string content_type = "Content-Type: ";
      content_type += SuffixToDesc(suffix);
      content_type += "\r\n";
      rep.AddHeader(content_type);

      //空行
      rep.AddHeader("\r\n");
    }
  public:
    EndPoint(int _fd):fd(_fd)
    {}
    void RecvRequest()
    {
      //第一行
      GetRequestLine();
      //报头信息
      GetRequestHeader();
      //是否需要读取正文
      if(req.IsNeedRecvBody()){
        //正文
        GetRequestBody();
      }
      //以上已经读完请求
      
    }
    void MakeRequest(int fd)
    {
      //分析请求
      int code = 200;
      std::string path;
      struct stat st;
      ssize_t size = 0;
      if(!req.IsMethodLegal()){
        std::cout << req.GetPath() << std::endl;
        if(req.GetPath() == WEBROOT)close(fd);
        LOG(Warning, "method is not legal!");
        code = 404;
        goto end;
      }
       if(req.IsGet()){
         req.UriParse();
       }
       else{
         req.SetUri();
       }
       req.IsAddHomePage();
       //三种情况
       //get 无参 :path 
       //get 有参 :path query_string 
       //post :path body
       
       path = req.GetPath();
       //std::cout << path << std::endl;
       if(stat(path.c_str(), &st) < 0){
         //std::cout<<path<<std::endl;
         LOG(Warning, "html is not exist! 404");
         code = 404;
         goto end;
       }
       else{
         if(S_ISDIR(st.st_mode)){
           path += "/";
           req.SetPath(path);
           req.IsAddHomePage();
         }
         else{
           if(st.st_mode & S_IXUSR || st.st_mode & S_IXGRP || st.st_mode & S_IXOTH){
             //CGI
             req.SetCgi(true);
           }
           else {
             //正常网页请求
           }
         }
         if(!req.IsCgi()){
           req.SetFileSize(st.st_size);
         }
       }

end:
      //制作response
      SetResponseStatusLine(code);
      SetResponseHeaderLine();
    }
    void ExecNonCgi(const std::string& path)
    {
      ssize_t size = req.GetFileSize();
      int ffd = open(path.c_str(), O_RDONLY);
      if(ffd < 0){
        LOG(Error, "path is not exits bug!");
        return;
      }
      sendfile(fd, ffd, nullptr, size);//直接拷入缓冲区
      close(ffd);
    }
    void ExecCgi()
    {
      
      //两个管道用于进程通信
      std::string path = req.GetPath();
      std::string content_length_env;
      std::string method = req.GetMethod();
      std::string method_env = "METHOD=";
      method_env += method;
      std::string query_string;
      std::string query_string_env;
      std::string body;

      int pipe_in[2] = {0};
      int pipe_out[2] = {0};
      
      pipe(pipe_in);
      pipe(pipe_out);

      //添加环境变量
      putenv((char*)method_env.c_str());
      pid_t id = fork();
      if(id == 0){
        //子进程
        close(pipe_in[1]);
        close(pipe_out[0]);
        //传数据方式：管道、环境变量
        dup2(pipe_in[0], 0);
        dup2(pipe_out[1], 1);
        //环境变量
        if(req.IsGet()){
          query_string = req.GetQueryString();
          query_string_env = "QUERY_STRING=";
          query_string_env += query_string;
          
          putenv((char*)query_string_env.c_str());
        }
        else if(req.IsPost()){
          content_length_env = "CONTENT-LENGTH=";
          content_length_env += std::to_string(req.GetContentLength());
          putenv((char*)content_length_env.c_str());
        }
        else{
          //TODO
        }
        //进程替换
        execl(path.c_str(), path.c_str(), nullptr);
        exit(0);
      }
      else if(id < 0){
        LOG(Fatal, "fork err!");
        return;
      }
      else{
        //父进程
        //管道
        close(pipe_in[0]);
        close(pipe_out[1]);

        char c = 'C';
        if(req.IsPost()){
          body = req.GetBody();
          for(int i = 0; i < body.size(); i++){
            write(pipe_in[1], &body[i], 1);
          }
        ssize_t s = 0;
        do{
          s = read(pipe_out[0], &c, 1);
          if(s > 0){
            send(fd, &c, 1, 0);
          }
        }while(s > 0);
        waitpid(id, nullptr, 0);
      }
    }
  }
    void SendResponse()
    {
      std::string line = rep.GetStatusLine();
      send(fd, line.c_str(), line.size(), 0);
      auto header = rep.GetResponseHander();
      for(auto it = header.begin(); it != header.end(); it++){
        send(fd, it->c_str(), it->size(), 0);
      }

      if(req.IsCgi()){
        //cgi
        LOG(Notice, "use cgi model!");
        ExecCgi();
      }
      else{
        LOG(Notice, "use non cgi model!");
        std::string path = req.GetPath();
        ExecNonCgi(path);
      }
    }
    ~EndPoint()
    {
      if(fd >= 0){
        close(fd);
      }
    }
};

class Entry{
  public:
    static void* HandlerHttp(void* arg)
    {
      int fd = *(int*)arg;
      
/*#ifdef DEBUG 
      char request[10240];
      int s = recv(fd, request, sizeof(request), 0);
      close(fd);

#else*/
    EndPoint *ep = new EndPoint(fd);
    ep->RecvRequest();
    ep->MakeRequest(fd);
    ep->SendResponse();
//#endif 

   } 
};
