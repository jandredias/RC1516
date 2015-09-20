#pragma once

#include "RequestECP.h"
#include "SocketUDP.h"
#include "SocketTCP.h"
#include <queue>
#include <semaphore.h>
#include <mutex>          // std::mutex

#include <utility>      // std::pair, std::make_pair

#ifndef __DEBUG__
#define __DEBUG__ 1
#endif

class ECPManager{

  std::queue<RequestECP> _tqrRequests;
  std::queue<RequestECP> _terRequests;
  std::queue<RequestECP> _iqrRequests;
  std::queue<RequestECP> _answers;

  //std::vector<int> _results;

  sem_t *_tqrSemaphore;
  sem_t *_terSemaphore;
  sem_t *_iqrSemaphore;
  sem_t *_answerSemaphore;

  std::mutex _tqrMutex;
  std::mutex _iqrMutex;
  std::mutex _terMutex;
  std::mutex _answerMutex;

  std::mutex _UDPMutex;

  SocketUDP socketUDP;

  int _port;
  bool _exit;

  int _maxAcceptingThreads;
  int _maxProcessTQRThreads;
  int _maxProcessTERThreads;
  int _maxProcessIQRThreads;
  int _maxSendAnswerThreads;

  std::string _topicsFile;
public:
  ECPManager(int);
  ~ECPManager();

  /**
   * @description           will get the UDP requests and redirect them
   */
  void acceptRequests();

  /**
   * @description           will process TQR requests
   */
  void processTQR();

  /**
   * @description           will process TER requests
   */
  void processTER();

  /**
   * @description           will process IQR requests
   */
  void processIQR();

  /**
   * @description           will send answers to clients and TES
   */
  void sendAnswer();

  /**
   * @return std::pair <std::string,int> will contain the topics separated by
   * spaces and the number of topics
   */
  std::pair <std::string,int> topics();


};
