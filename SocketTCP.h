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

#ifndef DEBUG
#define DEBUG 0
#endif

class SocketTCP{
  int _fd;
  struct hostent *_hostptr;
  struct sockaddr_in _serverAddr;
  struct sockaddr_in _clientAddr;
  int clientLen;
  bool _server;
  bool _connected;


public:
  SocketTCP();
  /**
   * @description           constructor used to accept new clients while
                            listening on server side
   */
  SocketTCP(int, struct sockaddr_in);

  /**
   * @description           this constructor will be used by clients to connect
   *                        to a server socket
   * @param const char[]    server's hostname
   * @param int             server's port
   */
  SocketTCP(const char[], int);

  /**
   * @description           this constructor will be used by servers to create
   *                        a socket and wait for clients to connect
   * @param int             server's port that will be listening
   */
  SocketTCP(int);

  /**
   * @description           set socket's file descriptor
   *                        this will be used when a client's connection
   *                        accepted
   * @param int             client's socket fd
   */
  void fd(int fd);

  /**
   * @return int file descriptor
   */
  int fd();

  /**
   * @description           connects to socket
   *                        if a server side socket tries to connect to some
   *                        other socket it will throw an exception
   *                        only client side should connect!
   * @throws std::string
   */
  void connect();

  /**
   * @description           closes the socket
   *                        on success returns void
   *                        on failure throws an exception
   * @throws std::string
   */
  void disconnect();

  /**
   * @return                boolean that states if socket is connected
   */
  bool connected();
  /**
   * @param std::string
   */
  void write(std::string);

  /**
   * @description
   * @param char*
   * @param int
   */
  void write(char*, int);

  /**
   * @description
   * @param const char*
   * @param int
   */
  void write(const char*, int);

  /**
   * @description the function will write a single character to the socket
   * @param const char the character to write
   */
  void write(const char);

  /**
   * @description
   * @return
   */
  std::string read();

  /**
   * @description
   * @param int
   * @return char*
   */
  char * read(int);

  /**
   * @description
   * @return std::string
   */
  std::string readWord();

  /**
   * @description
   * @return int
   */
  int rawRead();

  /**
   * @description
   * @param int = 5
   */
  void listen(int = 5);

  /**
   * @return int file descriptor
   */
  SocketTCP accept();

  /**
   * @description
   * @return std::string
   */
  std::string ip();

  /**
   * @description
   * @return std::string
   */
  std::string hostname();

  /**
   * @description set timeout while reading messages
   * @param int milisseconds to wait for message to receive
   */
  void timeout(int);
};
