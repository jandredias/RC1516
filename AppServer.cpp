#include "SocketTCP.h"
#include <iostream>
#include <string>

int main(){
  std::cout << "creating socket" << std::endl;
  SocketTCP socket(59000);
    std::cout << "listening on port 59000" << std::endl;
  socket.listen(10);
  while(1){
    SocketTCP newSocket = socket.accept();
    if(fork() == 0){
      socket.disconnect();
      std::string message = newSocket.read();
      std::cout << message << std::endl;
      std::string output = std::string("your input has ") + std::to_string(message.size()) \
      + std::string(" characters") + "\0";


      newSocket.write(output);
      return 0;
    }
    newSocket.disconnect();
  }
  return 0;
}
