#include <iostream>

#include <thread>         // std::thread
#define __PORT__ 58023

#ifndef DEBUG
#define DEBUG 1
#endif

#include "ECPManager.h"
int main(int argc, char* argv[]){
  int port = __PORT__;
  if(argc != 1 && argc != 3){ std::cout <<"wrong number of parameteres" << std::endl; return 1; }
  if(argc == 3){	
	if(std::string(argv[1]) != std::string("-p")){
	  std::cout << "wrong format of parameters" << std::endl;
	  return 1;
	}
	else{
	  //should redefine ecp port
	  port = atoi(argv[2]);
	}
  }
  std::cout << "port:    " << port << std::endl;

  try{
    ECPManager *manager = new ECPManager(port);

    std::thread acceptRequests(&ECPManager::acceptRequests, manager);
    std::thread processTQR(&ECPManager::processTQR, manager);
    std::thread processTER(&ECPManager::processTER, manager);
    std::thread processIQR(&ECPManager::processIQR, manager);
    std::thread sendAnswer(&ECPManager::sendAnswer, manager);

    acceptRequests.join();
    processTQR.join();
    processTER.join();
    processIQR.join();
    sendAnswer.join();

  }catch(std::string s){
    std::cout << s << std::endl;
  }
  return 0;
}
