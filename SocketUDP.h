#pragma once

#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <string.h>

#include <sys/un.h>
#include <stdio.h>
#include <arpa/inet.h>

#include <iostream>   // std::cout
#include <string>     // std::string, std::to_string

#define __DEBUG__ 1

#define BUFFER_SIZE 2048

class SocketUDP {
  int _fd;
  struct hostent *_hostptr;
  struct sockaddr_in _serverAddr;
  struct sockaddr_in _clientAddr;
  socklen_t _serverLen;
  int _clientLen;

  bool _server;

public:
  SocketUDP() : _server(true){}

  SocketUDP(const char addr[], int port) : _server(false){

    _fd = socket(AF_INET, SOCK_DGRAM, 0);
    
    if(_fd < 0) throw std::string("couldn't create socket");
    
    _hostptr = gethostbyname(addr);

    if(__DEBUG__) std::cout << "official name: " << _hostptr->h_name << std::endl;
    if(__DEBUG__) std::cout << "internet address: " 
                            << inet_ntoa(* (struct in_addr*) _hostptr->h_addr_list[0]) << " "
                            <<  ntohl(((struct in_addr*) _hostptr->h_addr_list[0])->s_addr) << std::endl;

    memset((void*) &_serverAddr,(int) '\0', sizeof(_serverAddr));
    _serverAddr.sin_family      = AF_INET;
    _serverAddr.sin_addr.s_addr = ((struct in_addr *) (_hostptr->h_addr_list[0]))->s_addr;
    _serverAddr.sin_port        = htons((u_short) port);

    _clientLen = sizeof(_clientAddr);
    _serverLen = sizeof(_serverAddr);
  }

  SocketUDP(int port) : _server(true){
    _fd = socket(AF_INET, SOCK_DGRAM, 0);

    memset((void*) &_serverAddr,(int)'\0', sizeof(_serverAddr));
    _serverAddr.sin_family      = AF_INET;
    _serverAddr.sin_addr.s_addr = htonl(INADDR_ANY);
    _serverAddr.sin_port        = htons((u_short) port);

    bind(_fd, (struct sockaddr*) &_serverAddr, sizeof(_serverAddr));
  }
  /**
   * @param  std::string   string text that will be send to the socket
   */
  void send(std::string text){

    if(text[text.size() - 1] != '\n') text += '\n';

    sendto(_fd, text.data(), text.size(), 0, (struct sockaddr*) &_serverAddr, _serverLen);
  }

  /**
   * @return std::string  string read from the socket
   */
  std::string receive(){
    char buffer[BUFFER_SIZE];
    int n = recvfrom(_fd, buffer, BUFFER_SIZE, 0, (struct sockaddr*) &_serverAddr, &_serverLen);
    if(n == -1) throw std::string("error reading content from the socket");
    std::string msg(buffer);
    int pos = msg.find('\n');
    return msg.substr(0, pos);
  }

  /**
   * @description   closes the socket
   *                on success nothing will be return
   *                on failure it will throw a std::string exception
   */
  void close(){
    if(::close(_fd) == -1) throw std::string();
  }
};
