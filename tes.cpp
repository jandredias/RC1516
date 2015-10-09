#include "SocketTCP.h"
#include "Debug.h"
#include "SocketUDP.h"
#include <iostream>
#include <string>
#include <cstdlib>
#include <thread>         // std::thread
#include <signal.h>

typedef void (*sighandler_t)(int);
//sighandler_t signal(int signum, sighandler_t handler);

#define __PORT_TES__ 59000
#define __PORT_ECP__ 58023
#define __HOST__ "localhost"
#define ANSWER_NO 1 //CHANGE ALSO TesManager.h!!!!!!!!!!!!!!!!!!!!!
#ifndef DEBUG
#define DEBUG 0
#endif

#include "TesManager.h"


int main(int argc, char* argv[]){
  int tesPort = __PORT_TES__;

  /* Definir dados do Servidor ECP */
  int ecpPort = __PORT_ECP__;
  std::string ecpName = __HOST__;

  bool flag_p = false;
  bool flag_n = false;
  bool flag_e = false;
  bool flag_good;

  //void (*old_handler)(int);//interrupt handler
 // if((old_handler=signal(SIGPIPE,SIG_IGN))==SIG_ERR) debug("ERRO PIPE");

  if(!(argc == 1 || argc == 3 || argc == 5 || argc == 7)){ std::cout <<"wrong number of parameteres" << std::endl; return 1; }
  for(int i = 1; i < argc; i += 2){
    flag_good = true;
    //Flag p - TESport
    if(!flag_p && std::string(argv[i]) == std::string("-p")){
      //should redefine tes port
      tesPort = atoi(argv[i+1]);
      flag_p = true;
      flag_good = false;
    }

    //Flag n - ECPname
    else if(!flag_n && std::string(argv[i]) == std::string("-n")){
      //should redefine ecp name
      ecpName = argv[i+1];
      flag_n = true;
      flag_good = false;
    }

    //Flag e - ECPport
    else if(!flag_e && std::string(argv[i]) == std::string("-e")){
      //should redefine ecp port
      ecpPort = atoi(argv[i+1]);
      flag_e = true;
      flag_good = false;
    }

    if(flag_good){
      std::cout << "wrong format of parameters" << std::endl;
      return -1;
    }
  }
  UI::Dialog::IO->print(  "port:    ");
  UI::Dialog::IO->println(std::to_string(tesPort));
  try{
    TesManager *manager = new TesManager(tesPort,ecpPort,ecpName);
    std::vector<std::thread> threads;
    threads.push_back(std::thread(&TesManager::acceptRequestsTCP, manager));
    threads.push_back(std::thread(&TesManager::acceptRequestsUDP, manager));
    threads.push_back(std::thread(&TesManager::answerUDP, manager));
    threads.push_back(std::thread(&TesManager::processTCP, manager));
    threads.push_back(std::thread(&TesManager::processRQT, manager));
    threads.push_back(std::thread(&TesManager::processRQS, manager));
    threads.push_back(std::thread(&TesManager::processAWI, manager));
    for(int i = 0; i < ANSWER_NO; i++)
      threads.push_back(std::thread(&TesManager::answerTCP, manager));


    for(std::thread &a : threads) a.join();

  }catch(std::string s){
    std::cout << s << std::endl;
  }
  return 0;
}
