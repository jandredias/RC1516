#include "SocketTCP.h"
#include <iostream>
#include <string>
#include <cstdlib>

#define __PORT__ 58023

int main(int argc, char* argv[]){
  std::cout << argc<<std::endl;
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
  std::cout << "port:    " << port << std::endl;
  return 0;
}
