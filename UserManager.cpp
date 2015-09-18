#include "UserManager.h"

#include "SocketUDP.h"
#include <sstream>
UserManager::UserManager(int sid, int port, std::string ecpname) : _sid(sid), _port(port),
_ecpname(ecpname){
}

void UserManager::list(){
  SocketUDP s = SocketUDP(_ecpname.data(), _port);
  s.send("TQR\n");
  std::stringstream stream(s.receive());
  std::string code;
  stream >> code;
  int nt;
  stream >> nt;
  std::cout << code << " " << nt << std::endl;

}

void UserManager::request(){
  //TODO
}

void UserManager::submit(){
  //TODO
}
