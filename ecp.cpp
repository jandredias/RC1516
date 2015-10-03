#include <iostream>

#include <thread>         // std::thread
#define __PORT__ 58023

#ifndef DEBUG
#define DEBUG 0
#endif

#include "ECPManager.h"
#include "Exception.h"
#include "Dialog.h"
int main(int argc, char* argv[]){
  int port = __PORT__;
  if(argc != 1 && argc != 3){
    UI::Dialog::IO->println("wrong number of parameteres");
    return 1;
  }
  if(argc == 3){
  	if(std::string(argv[1]) != std::string("-p")){
  	  UI::Dialog::IO->println("wrong format of parameters");
  	  return 1;
  	}
  	else{
  	  //should redefine ecp port
  	  port = atoi(argv[2]);
  	}
  }
  UI::Dialog::IO->print(  "port:    ");
  UI::Dialog::IO->println(std::to_string(port));

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

  }catch(SocketAlreadyInUse s){
    UI::Dialog::IO->println(s.message());
  }catch(std::string s){
    UI::Dialog::IO->println(s);
  }
  return 0;
}
