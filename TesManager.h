#pragma once

#include "RequestQuiz.h"
#include "SocketUDP.h"
#include "SocketTCP.h"
#include <vector>

class TesManager{

  int _qid;
  int _port;
  bool _exit;
  std::vector<RequestQuiz> _requests;
public:
  /**
   * @description           TesManager will manage the TES server
   *                        it will receive requests and answer them
   *                        there will be several threads working
   *                        acceptRequests will be one thread, and it will accept
   *                        connections and push them to a vector
   *
   *                        next the processRequests will get those connections
   *                        and process them
   *
   *                        this way the acceptRequests thread will be always free
   *                        accept new clients requests while other thread will be
   *                        working on current requests.
   *
   *                        in the future there might be several processRequests
   *                        threads working at the same time
   */
  TesManager(int);

  /**
   * @return                current unix timestamp
   */
  int time();

  /**
   * @param                 seconds untill the deadline
   * @return                a deadline
   */
  int deadline(int);

  /**
   * @return                always returns new quiz id
   */
  int qid();

  /**
   *@description            thread method that will accept new clients
   */
  void acceptRequests();

  /**
   * @description           thread that will process requests vector
   */
  void processRequests();
};
