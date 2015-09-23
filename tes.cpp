#include "SocketTCP.h"
#include "SocketUDP.h"
#include <iostream>
#include <string>
#include <cstdlib>
#include <thread>         // std::thread

#define __PORT__ 59000

#ifndef DEBUG
#define DEBUG 0
#endif

#include "TesManager.h"


int main(int argc, char* argv[]){
  int port = __PORT__;
  if(!(argc == 1 || argc == 3)){ std::cout <<"wrong number of parameteres" << std::endl; return 1; }
  for(int i = 1; i < argc; i += 2){
    if(std::string(argv[i]) != std::string("-p")){
      std::cout << "wrong format of parameters" << std::endl;
    }
    else{
      //should redefine ecp port
      port = atoi(argv[i+1]);
    }
  }
  try{
    TesManager *manager = new TesManager(port);
    std::vector<std::thread> threads;
    threads.push_back(std::thread(&TesManager::acceptRequestsTCP, manager));
    threads.push_back(std::thread(&TesManager::acceptRequestsUDP, manager));
    threads.push_back(std::thread(&TesManager::processTCP, manager));
    threads.push_back(std::thread(&TesManager::processRQT, manager));
    threads.push_back(std::thread(&TesManager::processRQS, manager));
    threads.push_back(std::thread(&TesManager::processAWI, manager));
    threads.push_back(std::thread(&TesManager::answerTCP, manager));

    for(std::thread &a : threads) a.join();

  }catch(std::string s){
    std::cout << s << std::endl;
  }
  return 0;
}
