#pragma once

#include "SocketUDP.h"
#include <queue>
#include <semaphore.h>

class ECPManager{

  std::queue<RequestsECP> _tqrRequests;
  std::queue<RequestsECP> _terRequests;
  std::queue<RequestsECP> _iqrRequests;
  std::queue<RequestsECP> _answers;

  std::vector<Result> _results;

  sem_t *_tqrSemaphore;
  sem_t *_terSemaphore;
  sem_t *_iqrSemaphore;
  sem_t *_answerSemaphore;

  int _port;
  bool _exit;

  int _maxAcceptingThreads;
  int _maxProcessTQRThreads;
  int _maxProcessTERThreads;
  int _maxProcessIQRThreads;
  int _maxSendAnswerThreads;

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

};
