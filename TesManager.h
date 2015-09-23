#pragma once

#include "RequestTES.h"
#include "SocketUDP.h"
#include "SocketTCP.h"
#include <queue>
#include <semaphore.h>
#include <mutex>

#ifndef DEBUG
#define DEBUG 0
#endif
#include <map>

class TesManager{
  std::queue<RequestTES> _requests;
  std::queue<RequestTES> _rqtRequests;
  std::queue<RequestTES> _rqsRequests;
  std::queue<RequestTES> _awiRequests;
  std::queue<RequestTES> _answers;

  std::map<int, std::string> _questionaries;

  sem_t * _requestsSem;
  sem_t * _rqtRequestsSem;
  sem_t * _rqsRequestsSem;
  sem_t * _awiRequestsSem;
  sem_t * _answerSem;

  std::mutex _questionariesMutex;
  std::mutex _reqMutex;
  std::mutex _rqtMutex;
  std::mutex _rqsMutex;
  std::mutex _awiMutex;
  std::mutex _answerMutex;

  std::mutex _receiverSocketUDPMutex;
  std::mutex _senderSocketUDPMutex;

  SocketUDP *_receiverSocketUDP;
  SocketUDP *_senderSocketUDP;


    int _qid;
    int _port;
    bool _exit;

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
  ~TesManager();

  /**
   * @return                current unix timestamp
   */
  int time();

  /**
   * @param                 seconds untill the deadline
   * @return                a deadline
   */
  int deadline(int = 600);

  /**
   * @return                always returns new quiz id
   */
  int qid();

  /**
   *@description            thread method that will accept new TCP clients
   */
  void acceptRequestsTCP();

  /**
   *@description            thread method that will accept new UDP clients
   */
  void acceptRequestsUDP();

  /**
   * @description  will process TCP requests and filter them
   */
  void processTCP();

  /**
   * @description           will process RQT requests
   */
  void processRQT();

  /**
   * @description           will process RQS requests
   */
  void processRQS();

  /**
   * @description           will process AWI requests
   */
  void processAWI();

  /**
   * @description           will send answers to TCP clients
   */
  void answerTCP();


  /**
   * @return  std::pair <char*, int> pdf content and size
   */
  std::pair <char *, int> pdf(std::string);

};
