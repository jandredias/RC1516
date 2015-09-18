#include "UserManager.h"

#include "SocketUDP.h"

UserManager::UserManager(int sid, int port, std::string ecpname) : _sid(sid), _port(port),
_ecpname(ecpname){
}

void UserManager::list(){
  SocketUDP s = SocketUDP(_ecpname.data(), _port);
  s.send("TQR\n");

}

void UserManager::request(){
  //TODO
}

void UserManager::submit(){
  //TODO
}
