#include "TesManager.h"
#include "Dialog.h"
#include <sstream>
#include <fstream>      // std::ifstream
#include <utility>
TesManager::TesManager(int port) :  _requestsSem(new sem_t()),
_rqtRequestsSem(new sem_t()), _rqsRequestsSem(new sem_t()),
_awiRequestsSem(new sem_t()), _answerSem(new sem_t()), _qid(1), _port(port),
_exit(false) {
  sem_init(_requestsSem, 0, 0);
  sem_init(_rqtRequestsSem, 0, 0);
  sem_init(_rqsRequestsSem, 0, 0);
  sem_init(_awiRequestsSem, 0, 0);
}

TesManager::~TesManager(){
  sem_destroy(_requestsSem);
  sem_destroy(_answerSem);
  sem_destroy(_rqtRequestsSem);
  sem_destroy(_rqsRequestsSem);
  sem_destroy(_awiRequestsSem);
}

int TesManager::time(){ std::time_t t = std::time(0); return t; }

int TesManager::deadline(int s){ return time() + s; }

int TesManager::qid(){ return _qid++; }

void TesManager::acceptRequestsTCP(){
  #if DEBUG
  UI::Dialog::IO->println("[ [BLUE]TesManager::acceptRequests[REGULAR]  ] Creating socket");
  #endif
  try{
    SocketTCP _socketTCP(_port);

    #if DEBUG
    UI::Dialog::IO->println("[ [BLUE]TesManager::acceptRequests[REGULAR]  ] Socket created ");
    UI::Dialog::IO->println(
      std::string("[ [BLUE]TesManager::acceptRequests[REGULAR]  ] Listening on port ")\
      + std::to_string(_port));
    #endif

    _socketTCP.listen(10);

    while(!_exit){
      #if DEBUG
      UI::Dialog::IO->println("[ [BLUE]TesManager::acceptRequests[REGULAR]  ] Waiting for clients");
      UI::Dialog::IO->println(
        std::string("[ [BLUE]TesManager::acceptRequests[REGULAR]  ]  Requests size ")\
        + std::to_string(_rqtRequests.size()));
      #endif

      SocketTCP s = _socketTCP.accept();

      #if DEBUG
      UI::Dialog::IO->println("[ [BLUE]TesManager::acceptRequests[REGULAR]  ] Reading from client");
      UI::Dialog::IO->println("[ [BLUE]TesManager::acceptRequests[REGULAR]  ] Client request read");
      #endif

      RequestTES r = RequestTES(s);

      _reqMutex.lock();
      _requests.push(r);
      _reqMutex.unlock();

      #if DEBUG
      UI::Dialog::IO->println("[ [BLUE]TesManager::acceptRequests[REGULAR]  ] Client connected");
      #endif

      sem_post(_requestsSem);
    }
  }catch(std::string s){
    UI::Dialog::IO->println(s);
    _exit = 1;
    sem_post(_requestsSem);
    sem_post(_answerSem);
    sem_post(_rqtRequestsSem);
    sem_post(_rqsRequestsSem);
    sem_post(_awiRequestsSem);
    return;
  }
}

void TesManager::acceptRequestsUDP(){
  //TODO
}

void TesManager::processTCP(){

  #if DEBUG
  UI::Dialog::IO->println("[ [GREEN]TesManager::processTCP[REGULAR]      ] processRequests");
  #endif

  while(!_exit){
    sem_wait(_requestsSem);
    if(_exit) return;
    #if DEBUG
    UI::Dialog::IO->println("[ [GREEN]TesManager::processTCP[REGULAR]      ] Client is waiting for answer");
    UI::Dialog::IO->println(
      std::string("[ [GREEN]TesManager::processTCP[REGULAR]      ] Requests size: ") + \
      std::to_string(_requests.size()));
    #endif

    _reqMutex.lock();

    #if DEBUG
    UI::Dialog::IO->println("[ [GREEN]TesManager::processTCP[REGULAR]      ] Getting first in the queue");
    #endif

    RequestTES r = _requests.front();
    _requests.pop();

    #if DEBUG
    UI::Dialog::IO->println("[ [GREEN]TesManager::processTCP[REGULAR]      ] Removing first Request from the queue");
    UI::Dialog::IO->println(
      std::string("[ [GREEN]TesManager::processTCP[REGULAR]      ] Requests size: ") + \
      std::to_string(_requests.size()));
    #endif

    _reqMutex.unlock();

    #if DEBUG
    UI::Dialog::IO->println("[ [GREEN]TesManager::processTCP[REGULAR]      ] Reading message from Request");
    #endif

    r.message(r.read());

    #if DEBUG
    UI::Dialog::IO->println("[ [GREEN]TesManager::processTCP[REGULAR]      ] Message read from Request");
    #endif

    std::stringstream stream(r.message());
    std::string typeOfRequest;
    stream >> typeOfRequest;
    if(typeOfRequest == std::string("RQT")){
      #if DEBUG
      UI::Dialog::IO->println("[ [GREEN]TesManager::processTCP[REGULAR]      ] Type of request is RQT");
      #endif

      _rqtMutex.lock();

      #if DEBUG
      UI::Dialog::IO->println("[ [GREEN]TesManager::processTCP[REGULAR]      ] Inserting request on RQT queue");
      #endif

      _rqtRequests.push(r);
      sem_post(_rqtRequestsSem);
      _rqtMutex.unlock();

      #if DEBUG
      UI::Dialog::IO->println("[ [GREEN]TesManager::processTCP[REGULAR]      ] Request inserted in RQT queue");
      #endif

    }else if(typeOfRequest == std::string("RQS")){

      #if DEBUG
      UI::Dialog::IO->println("[ [GREEN]TesManager::processTCP[REGULAR]      ] Type of request is RQS");
      #endif

      _rqsMutex.lock();

      #if DEBUG
      UI::Dialog::IO->println("[ [GREEN]TesManager::processTCP[REGULAR]      ] Inserting request on RQS queue");
      #endif

      _rqsRequests.push(r);
      sem_post(_rqsRequestsSem);
      _rqsMutex.unlock();

      #if DEBUG
      UI::Dialog::IO->println("[ [GREEN]TesManager::processTCP[REGULAR]      ] Request inserted in RQS queue");
      #endif

    }else{

      #if DEBUG
      UI::Dialog::IO->println("[ [GREEN]TesManager::processTCP[REGULAR]      ] Type of request unknown");
      #endif

      r.message("ERR\n");
      _answerMutex.lock();

      #if DEBUG
      UI::Dialog::IO->println("[ [GREEN]TesManager::processTCP[REGULAR]      ] Inserting request on RQS queue");
      #endif

      _answers.push(r);
      sem_post(_answerSem);

      #if DEBUG
      UI::Dialog::IO->println("[ [GREEN]TesManager::processTCP[REGULAR]      ] Request inserted in RQS queue");
      #endif

      _answerMutex.unlock();
    }
  }
}

void TesManager::processRQT(){

  #if DEBUG
  UI::Dialog::IO->println("[ [YELLOW]TesManager::processRQT[REGULAR]      ] BEGIN");
  #endif

  while(!_exit){

    #if DEBUG
    UI::Dialog::IO->println("[ [YELLOW]TesManager::processRQT[REGULAR]      ] I'm waiting for requests to process");
    #endif

    sem_wait(_rqtRequestsSem);
    if(_exit) return;

    #if DEBUG
    UI::Dialog::IO->println("[ [YELLOW]TesManager::processRQT[REGULAR]      ] Client is waiting for answer");
    UI::Dialog::IO->println(
      std::string("[ [YELLOW]TesManager::processRQT[REGULAR]      ] Requests size: ") + \
      std::to_string(_rqtRequests.size()));
    #endif

    _rqtMutex.lock();

    #if DEBUG
    UI::Dialog::IO->println("[ [YELLOW]TesManager::processTCP[REGULAR]      ] Removing request from the RQT queue");
    #endif

    RequestTES r = _rqtRequests.front();
    _rqtRequests.pop();

    #if DEBUG
    UI::Dialog::IO->println("[ [YELLOW]TesManager::processTCP[REGULAR]      ] Removed request from the RQT queue");
    #endif

    _rqtMutex.unlock();

    std::stringstream stream(r.message());

    std::string code;
    std::string SIDstr;
    std::string trash;
    int SID;

    stream >> code;
    stream >> SIDstr;
    stream >> trash;
    int tID;
    bool is_number = true;
    for(int index = 0; index < (int) SIDstr.size(); index++)
      if(SIDstr[index] < '0' || SIDstr[index] > '9') is_number = false;

    if(SIDstr.size() == 0 || !is_number || trash != std::string("")){
      r.answer("ERR");
    }else{
      SID = atoi(SIDstr.data());
      r.sid(SID);
      r.qid(qid());
      r.deadline(deadline());
      std::string answer;
      answer  = std::string("AQT ");
      answer += std::to_string(r.qid());
      answer += std::string(" ");
      answer += std::to_string(deadline());
      answer += std::string(" ");
      std::string filename = std::to_string(rand() % 5 + 1) + std::string(".pdf");

      #if DEBUG
      UI::Dialog::IO->print("[ [YELLOW]TesManager::processRQT[REGULAR]      ] Reading file: ");
      UI::Dialog::IO->println(filename);
      #endif

      std::pair <char*, int> pair = pdf(std::to_string(rand() % 5 + 1) + std::string(".pdf"));

      #if DEBUG
      UI::Dialog::IO->println("[ [YELLOW]TesManager::processRQT[REGULAR]      ] File read");
      #endif

      answer += std::to_string(pair.second);
      answer += std::string(" ");
      r.fileSize(pair.second);

      #if DEBUG
      UI::Dialog::IO->print("[ [YELLOW]TesManager::processRQT[REGULAR]      ] Answer before data: ");
      UI::Dialog::IO->println(answer);
      #endif

      r.file(pair.first);
      r.answer(answer);

      #if DEBUG
      UI::Dialog::IO->println(r.answer());
      #endif

    }
    _answerMutex.lock();

    #if DEBUG
    UI::Dialog::IO->println("[ [YELLOW]TesManager::processRQT[REGULAR]      ] Inserting request on Answer queue");
    #endif

    _answers.push(r);
    sem_post(_answerSem);

    #if DEBUG
    UI::Dialog::IO->println("[ [YELLOW]TesManager::processRQT[REGULAR]      ] Request inserted in Answer queue");
    #endif

    _answerMutex.unlock();
  }
}

void TesManager::processRQS(){

  #if DEBUG
  UI::Dialog::IO->println("[ [MAGENT]TesManager::processRQS[REGULAR]      ] BEGIN");
  #endif

  while(!_exit){

    #if DEBUG
    UI::Dialog::IO->println("[ [MAGENT]TesManager::processRQS[REGULAR]      ] I'm waiting for requests to process");
    #endif

    sem_wait(_rqsRequestsSem);
    if(_exit) return;

    #if DEBUG
    UI::Dialog::IO->println("[ [MAGENT]TesManager::processRQS[REGULAR]      ] Client is waiting for answer");
    UI::Dialog::IO->println(
      std::string("[ [MAGENT]TesManager::processRQS[REGULAR]      ] Requests size: ") + \
      std::to_string(_rqsRequests.size()));
    #endif

    _rqsMutex.lock();

    #if DEBUG
    UI::Dialog::IO->println("[ [MAGENT]TesManager::processRQS[REGULAR]      ] Removing request from the RQS queue");
    #endif
    RequestTES r = _rqsRequests.front();
    _rqsRequests.pop();

    #if DEBUG
    UI::Dialog::IO->println("[ [MAGENT]TesManager::processRQS[REGULAR]      ] Removed request from the RQS queue");
    #endif
    _rqsMutex.unlock();

    std::cout << r.read() << std::endl;
    r.message("AQS \n");

    //FIXME

    _answerMutex.lock();

    #if DEBUG
    UI::Dialog::IO->println("[ [MAGENT]TesManager::processRQS[REGULAR]      ] Inserting request on Answer queue");
    #endif

    _answers.push(r);
    sem_post(_answerSem);

    #if DEBUG
    UI::Dialog::IO->println("[ [MAGENT]TesManager::processRQS[REGULAR]      ] Request inserted in Answer queue");
    #endif

    _answerMutex.unlock();
  }
}

void TesManager::processAWI(){
  //TODO
}

void TesManager::answerTCP(){

  #if DEBUG
  UI::Dialog::IO->println("[ [BLUE]TesManager::answerTCP[REGULAR]       ] BEGIN");
  #endif

  while(!_exit){

    #if DEBUG
    UI::Dialog::IO->println("[ [BLUE]TesManager::answerTCP[REGULAR]       ] I'm waiting for requests to process");
    #endif

    sem_wait(_answerSem);
    if(_exit) return;


    #if DEBUG
    UI::Dialog::IO->println("[ [BLUE]TesManager::answerTCP[REGULAR]       ] Client is waiting for answer");
    #endif

    #if DEBUG
    UI::Dialog::IO->println(
      std::string("[ [BLUE]TesManager::answerTCP[REGULAR]       ] Requests size: ") + \
      std::to_string(_rqtRequests.size()));
    #endif

    _answerMutex.lock();

    #if DEBUG
    UI::Dialog::IO->println("[ [BLUE]TesManager::answerTCP[REGULAR]       ] Removing request from the RQT queue");
    #endif

    RequestTES r = _answers.front();
    _answers.pop();

    #if DEBUG
    UI::Dialog::IO->println("[ [BLUE]TesManager::answerTCP[REGULAR]       ] Removed request from the RQT queue");
    #endif

    _answerMutex.unlock();

    #if DEBUG
    if(r.answer().size() < 100) UI::Dialog::IO->println(r.answer());
    #endif

    r.write();

    r.disconnect();
  }
}

std::pair <char *, int> TesManager::pdf(std::string filename){
  std::ifstream file(filename, std::ifstream::binary);
  if(file){
    file.seekg(0, file.end);
    int length = file.tellg();
    file.seekg(0, file.beg);

    char *buffer = new char[length];

    file.read(buffer, length);
    #if DEBUG
      if (file)
        UI::Dialog::IO->println("[ TesManager::pdf             ] All characters read from file");
      else{
        UI::Dialog::IO->print  ("[ TesManager::pdf             ] Only ");
        UI::Dialog::IO->print  (std::to_string(file.gcount()));
        UI::Dialog::IO->println(" could be read");
      }
    #endif

    file.close();
    return std::make_pair(buffer,length);
  }
  return std::make_pair((char*)NULL, 0);
}
