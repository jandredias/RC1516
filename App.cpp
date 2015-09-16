#include "SocketTCP.h"
#include <iostream>
#include <string>

int main(){
  try{
    SocketTCP socket("jdiastk.com", 59000);
    socket.connect();
    socket.write("olaadeus");
    //std::string message = socket.read();
    //std::cout << message << std::endl;
  }catch(std::string s){
    std::cout << s << std::endl;
  }
  return 0;
}
