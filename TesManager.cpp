#include "TesManager.h"

TesManager::TesManager(int port) : _qid(1), _port(port),
_exit(false) {}

int TesManager::time(){ std::time_t t = std::time(0); return t; }

int TesManager::deadline(int s){ return time() + s; }

int TesManager::qid(){ return _qid++; }

void TesManager::acceptRequests(){
  std::cout << "Creating socket" << std::endl;
  SocketTCP socket(_port);
  std::cout << "Socket created" << std::endl;
  std::cout << "Listening on port " << _port << std::endl;
  socket.listen(10);
  while(!_exit){
    std::cout << "Waiting for clients" << std::endl;
    _requests.push_back(RequestQuiz(socket.accept(), 0, 0, 0));
    std::cout << "Accepted" << std::endl;
  }
}
void TesManager::processRequests(){
  std::cout << "processRequests" << std::endl;
  while(1){
    for(RequestQuiz r : _requests){
      r.write("AQT\n");
      r.disconnect();
    }
    for(auto i = 0; i < (int) _requests.size(); i++)
      if(_requests[i].finished())
        _requests.erase(_requests.begin() + i++);
  }

}
