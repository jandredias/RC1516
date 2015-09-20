#pragma once

#include <ctime>
#include "SocketTCP.h"
#include "SocketUDP.h"
#include <string>

class RequestTES{
  SocketTCP _client;
  std::string _message;
  std::string _answer;
  int       _qid;
  int       _sid;
  int       _deadline;
  int       _fileSize;

public:
  /**
   * @param SocketTCP
   * @param int             student's id
   * @param int             quiz' id
   * @param int             deadline timestamp
   */
  RequestTES(SocketTCP, int = 0, int = 0, int = 0);
  void sid(int);
  int qid();
  void qid(int);
  void deadline(int);
  void fileSize(int);
  void message(std::string);
  std::string message();
  void answer(std::string);
  std::string answer();
  void write(std::string text);
  void write();
  std::string read();
  void disconnect();
  bool finished();
};
