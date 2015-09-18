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

  /**
   * //FIXME
   */
  SocketUDP();

  /**
   * //FIXME
   */
  SocketUDP(const char[], int );
  /**
   * //FIXME
   */
  SocketUDP(int);
  /**
   * @param  std::string   string text that will be send to the socket
   */
  void send(std::string);

  /**
   * @return std::string  string read from the socket
   */
  std::string receive();

  /**
   * @description   closes the socket
   *                on success nothing will be return
   *                on failure it will throw a std::string exception
   */
  void close();
};
