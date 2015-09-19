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

  if(__DEBUG__) std::cout << "[ acceptRequests  ] Creating socket" << std::endl;
  SocketTCP socket(_port);
  if(__DEBUG__) std::cout << "[ acceptRequests  ] Socket created" << std::endl;
  if(__DEBUG__) std::cout << "[ acceptRequests  ] Listening on port " << _port << std::endl;
  socket.listen(10);
  while(!_exit){
    if(__DEBUG__) std::cout << "[ acceptRequests  ] Waiting for clients" << std::endl;
    if(__DEBUG__) std::cout << "[ acceptRequests  ] Requests size: " << _requests.size() << std::endl;

    _requests.push(RequestQuiz(socket.accept(), 0, 0, 0));
    sem_post(_requestsSem);
  }
}
void TesManager::processRequests(){
  if(__DEBUG__) std::cout << "[ processRequests ] tprocessRequests" << std::endl;
  while(!_exit){
    if(__DEBUG__) std::cout << "[ processRequests ] I'm waiting for requests to process" << std::endl;
    sem_wait(_requestsSem);

    if(__DEBUG__) std::cout << "[ processRequests ] Client is waiting for answer" << std::endl;
    if(__DEBUG__) std::cout << "[ processRequests ] Requests size: " << _requests.size() << std::endl;
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
