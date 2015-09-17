#include <iostream>
#define __PORT__ 58023
#define __HOST__ "localhost"
int main(int argc, char* argv[]){
  int sid = 0; 
  int port = __PORT__;
  std::string ecpname = __HOST__;
  if(argc < 2){ std::cout << "you need to write a SID composed of 5 digits (e.g. 76543)" << std::endl; return 1; }
  
  if(argc % 2 == 1){ std::cout <<"wrong number of parameteres" << std::endl; return 1; }
  
  sid = atoi(argv[1]);
  
  if(sid < 10000 || sid > 99999){
    std::cout << "wrong format of identity number! it should be composed of 5 digits (e.g. 76543)" << std::endl;
    return 1;
  }
  for(int i = 2; i < argc; i += 2){
    if(std::string(argv[i]) != std::string("-n") && \
       std::string(argv[i]) != std::string("-p")){
      std::cout << "wrong format of parameters" << std::endl;
    }
    else if(std::string(argv[i]) == std::string("-n")){
      //should redefine ecp hostname
      ecpname = argv[i+1];
    }else{
      //should redefine ecp port
      port = atoi(argv[i+1]);
    }
  }
  while(1){
    std::string command;
    std::cin >> command;
    if(command == std::string("exit")) break;
    else if(command == std::string("list")){
      //list command
      //TODO
    }else if(command == std::string("request")){
      //request command
      //TODO
    }else if(command == std::string("submit")){
      //submit command
      //TODO
    }else std::cout << "Option unknown: " << command << std::endl;
  }
  std::cout << "user id: " << sid << std::endl;
  std::cout << "ecpname: " << ecpname << std::endl;
  std::cout << "port:    " << port << std::endl;
  return 0;
}
