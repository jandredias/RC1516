#pragma once

#include <ctime>
#include "SocketTCP.h"
#include "SocketUDP.h"
#include <string>

class RequestQuiz{
  SocketTCP _client;
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
  RequestQuiz(SocketTCP, int, int, int);

  void write(std::string text){ _client.write(text); }
  std::string read(){ return _client.read(); }
  void disconnect(){ _client.disconnect(); }
  bool finished(){ return !_client.connected(); }

};
