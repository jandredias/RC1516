#include "TesManager.h"
#include "Dialog.h"

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
  if(__DEBUG__) UI::Dialog::IO->println("[ TesManager::acceptRequests  ] Creating socket");
  SocketTCP socket(_port);
  if(__DEBUG__) UI::Dialog::IO->println("[ TesManager::acceptRequests  ] Socket created ");
  if(__DEBUG__) UI::Dialog::IO->println(
                  std::string("[ TesManager::acceptRequests  Listening on port ")\
                  + std::to_string(_port));

  socket.listen(10);

  while(!_exit){
    if(__DEBUG__) UI::Dialog::IO->println("[ TesManager::acceptRequests  ] Waiting for clients");
    if(__DEBUG__) UI::Dialog::IO->println(
                    std::string("[ TesManager::acceptRequests  ]  Requests size ")\
                    + std::to_string(_requests.size()));

    _requests.push(RequestQuiz(socket.accept(), 0, 0, 0));
    sem_post(_requestsSem);
  }
}
void TesManager::processRequests(){
  if(__DEBUG__) UI::Dialog::IO->println("[ TesManager::processRequests ] tprocessRequests");
  while(!_exit){
    if(__DEBUG__) UI::Dialog::IO->println("[ TesManager::processRequests ] I'm waiting for requests to process");

    sem_wait(_requestsSem);

    if(__DEBUG__) UI::Dialog::IO->println("[ TesManager::processRequests ] Client is waiting for answer");
    if(__DEBUG__) UI::Dialog::IO->println(
                    std::string("[ TesManager::processRequests ] Requests size: ") + \
                    std::to_string(_requests.size()));

    RequestQuiz r = _requests.front();
    _requests.pop();

    std::cout << r.read() << std::endl;

    r.write("AQT\n");
    r.disconnect();

  }

}
void TesManager::quiz(){

}
void TesManager::answers(){

}
