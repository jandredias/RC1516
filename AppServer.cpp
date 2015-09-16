#include "SocketTCP.h"
#include <iostream>
#include <string>

int main(){
  std::cout << "creating socket" << std::endl;
  SocketTCP socket(59000);
    std::cout << "listening on port 59000" << std::endl;
  socket.listen(10);
  while(1){
    std::cout << "waiting for clients" << std::endl;
    SocketTCP newSocket = socket.accept();
    if(fork() == 0){
      socket.disconnect();
      std::cout << "reading message" << std::endl;
      std::string message = newSocket.read();
        std::cout << "message received" << std::endl;
      std::cout << message << std::endl;
      std::string output = std::string("your input has ") + std::to_string(message.size()) \
      + std::string(" characters");

      std::cout << output << std::endl;
      std::cout << "sending message" << std::endl;
      newSocket.write(output);
      std::cout << "message sent" << std::endl;
      return 0;
    }
    newSocket.disconnect();
  }
  return 0;
}
