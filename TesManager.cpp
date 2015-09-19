#include "TesManager.h"




TesManager::TesManager(int port) : _qid(1), _port(port),
_exit(false) {
  _requestsSem = new sem_t();
  sem_init(_requestsSem, 0, 0);
}

TesManager::~TesManager(){
  sem_destroy(_requestsSem);
}

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
    std::cout << "Requests size: " << _requests.size() << std::endl;

    _requests.push(RequestQuiz(socket.accept(), 0, 0, 0));

    sem_post(_requestsSem);
    std::cout << "Requests size: " << _requests.size() << std::endl;
    std::cout << "Accepted" << std::endl;
  }
}
void TesManager::processRequests(){
  std::cout << "\tprocessRequests" << std::endl;
  while(!_exit){
    std::cout << "\tI'm waiting for requests to process" << std::endl;
    sem_wait(_requestsSem);

    std::cout << "\tClient is waiting for answer" << std::endl;
    std::cout << "\tRequests size: " << _requests.size() << std::endl;
    try{
      if(_requests.size() == 0) throw std::string("Request vector size should not be zero");
    }catch(std::string s){
      std::cout << s << std::endl;
    }

    RequestQuiz r = _requests.front();
    _requests.pop();

    std::cout << r.read() << std::endl;
    r.write("AQT\n");
    r.disconnect();

  }

}
