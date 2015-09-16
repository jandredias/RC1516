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

class SocketTCP{
  int _fd;
  struct hostent *_hostptr;
  struct sockaddr_in _serverAddr;
  struct sockaddr_in _clientAddr;
  int clientLen;
  bool _server;
  bool _connected;

public:
  SocketTCP() : _server(true), _connected(true){}
  SocketTCP(const char[] addr, int port) : _server(false), _connected(false){
    _fd = socket(AF_INET, SOCK_STREAM, 0);
    if(_fd < 0) throw std::string("couldn't create socket");
    _hostptr = gethostbyname(addr);

    memset((void *) &_serverAddr, (int) '\0', sizeof(_serverAddr));

    _serverAddr.sin_family = AF_INET;
    _serverAddr.sin_port = htons((u_short) port);
    _serverAddr.sin_addr.s_addr = ((struct in_addr *) (_hostptr->h_addr_list[0]))->s_addr;
  }
  SocketTCP(int port) : _server(true), _connected(false){
    _fd = socket(AF_INET, SOCK_STREAM, 0);
    _hostptr = gethostbyname(addr);

    memset((void *) &_serverAddr, (int) '\0', sizeof(_serverAddr));

    _serverAddr.sin_family = AF_INET;
    _serverAddr.sin_port = htons((u_short) port);
    _serverAddr.sin_addr.s_addr = htonl(INADDR_ANY);
  }
  void fd(int fd){ _fd = fd; }
  /**
   * @return int file descriptor
   */
  int fd(){ return _fd; }
  /**
   * @description connects to socket
   */
  void connect(){
    if(_server) throw std::string("error! you can't connect from a server side socket");
    if(::connect(_fd,(struct sockaddr *) &_serverAddr, sizeof(_serverAddr)) < 0)
      throw std::string("error connecting internet socket");
    _connected = true;
  }
  void disconnect(){
    if(::close(_fd))
      throw std::string("Error closing socket");
    _connected = false;
  }
  void write(std::string text){
    if(!_connected) throw std::string("Socket is not connected");
    ::write(_fd, text.data(), text.size() + 1);
  }

  void listen(int max = 5){ ::listen(_fd, max); }

  /**
   * @return int file descriptor
   */
  SocketTCP accept(){
    int newSocket = -1;

    socklen_t sizeofClient;

    newSocket = ::accept(_sockfd, (struct sockaddr*) &_clientAddr, &sizeofClient);
    if(newSocket < 0) throw std::string("error creating dedicate connection");

    SocketTCP s = SocketTCP();

    s.fd(newSocket);
    return s;
  }

};
