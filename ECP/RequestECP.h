#pragma once

#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <string.h>
#include <string>

#include <sys/un.h>
#include <stdio.h>
#include <arpa/inet.h>
class RequestECP{
  std::string _message;
  std::string _answer;
  bool _processed;
  bool _sent;
  struct sockaddr_in _clientAddr;
public:
  /**
   * @param std::string string that will contain the message received and
   * that will be processed by one thread
   *
   * @param struct sockaddr_in struct that identifies the client socket and will
   * allow to answer to the client later
   */
  RequestECP(std::string, struct sockaddr_in);

  /**
   * @param std::string message that will contain the message received and that
   * that will be processed by one thread
   */
  RequestECP(std::string);

  /**
   * @return std::string message
   */
  std::string message();
  /**
   * @param std::string message that will be sent to the client as an answer
   */
  void answer(std::string);

  /**
   * @return std::string will return the answer to send to client
   */
  std::string answer();
  /**
   * @return std::string message;
   */
  std::string read();

  /**
   * @reteurn struct sockaddr_in corresponding to the client
   */
  struct sockaddr_in client();


};
