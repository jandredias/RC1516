#include "ECPManager.h"

ECPManager::ECPManager(int port) : _requestsSem(new sem_t()), _port(port),
_exit(false){
  sem_init(_requestsSem, 0, 0);
}
ECPManager::~ECPManager(){
  sem_destroy(_requestsSem);
}


int _maxAcceptingThreads;
int _maxProcessTQRThreads;
int _maxProcessTERThreads;
int _maxProcessIQRThreads;
int _maxSendAnswerThreads;

void ECPManager::acceptRequests(){
  //TODO
}

void ECPManager::processTQR(){
  //TODO
}

void ECPManager::processTER(){
  //TODO
}

void ECPManager::processIQR(){
  //TODO
}

void ECPManager::sendAnswer(){
  //TODO
}
