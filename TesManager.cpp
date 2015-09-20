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
  //FIXME
  if(__DEBUG__) UI::Dialog::IO->println("[ [BLUE]TesManager::acceptRequests[REGULAR]  ] Creating socket");
  SocketTCP _socketTCP(_port);
  if(__DEBUG__) UI::Dialog::IO->println("[ [BLUE]TesManager::acceptRequests[REGULAR]  ] Socket created ");
  if(__DEBUG__) UI::Dialog::IO->println(
                  std::string("[ [BLUE]TesManager::acceptRequests[REGULAR]  ] Listening on port ")\
                  + std::to_string(_port));

  _socketTCP.listen(10);

  while(!_exit){
    if(__DEBUG__) UI::Dialog::IO->println("[ [BLUE]TesManager::acceptRequests[REGULAR]  ] Waiting for clients");
    if(__DEBUG__) UI::Dialog::IO->println(
                    std::string("[ [BLUE]TesManager::acceptRequests[REGULAR]  ]  Requests size ")\
                    + std::to_string(_rqtRequests.size()));

    _requests.push(RequestTES(_socketTCP.accept()));
    if(__DEBUG__) UI::Dialog::IO->println("[ [BLUE]TesManager::acceptRequests[REGULAR]  ] Client connected");
    sem_post(_requestsSem);
  }
}

void TesManager::acceptRequestsUDP(){
  //TODO
}

void TesManager::processTCP(){
  if(__DEBUG__) UI::Dialog::IO->println("[ [GREEN]TesManager::processTCP[REGULAR]      ] processRequests");
  while(!_exit){
    sem_wait(_requestsSem);
    if(__DEBUG__) UI::Dialog::IO->println("[ [GREEN]TesManager::processTCP[REGULAR]      ] Client is waiting for answer");
    if(__DEBUG__) UI::Dialog::IO->println(
                    std::string("[ [GREEN]TesManager::processTCP[REGULAR]      ] Requests size: ") + \
                    std::to_string(_requests.size()));

    _reqMutex.lock();
    if(__DEBUG__) UI::Dialog::IO->println("[ [GREEN]TesManager::processTCP[REGULAR]      ] Getting first in the queue");
    RequestTES r = _requests.front();
    _requests.pop();
    if(__DEBUG__) UI::Dialog::IO->println("[ [GREEN]TesManager::processTCP[REGULAR]      ] Removing first Request from the queue");
    if(__DEBUG__) UI::Dialog::IO->println(
                    std::string("[ [GREEN]TesManager::processTCP[REGULAR]      ] Requests size: ") + \
                    std::to_string(_requests.size()));
    _reqMutex.unlock();
    r.message(r.read());
    std::stringstream stream(r.message());
    std::string typeOfRequest;
    stream >> typeOfRequest;
    if(typeOfRequest == std::string("RQT")){
      if(__DEBUG__) UI::Dialog::IO->println("[ [GREEN]TesManager::processTCP[REGULAR]      ] Type of request is RQT");
      _rqtMutex.lock();
      if(__DEBUG__) UI::Dialog::IO->println("[ [GREEN]TesManager::processTCP[REGULAR]      ] Inserting request on RQT queue");
      _rqtRequests.push(r);
      sem_post(_rqtRequestsSem);
      _rqtMutex.unlock();
      if(__DEBUG__) UI::Dialog::IO->println("[ [GREEN]TesManager::processTCP[REGULAR]      ] Request inserted in RQT queue");
    }else if(typeOfRequest == std::string("RQS")){
      if(__DEBUG__) UI::Dialog::IO->println("[ [GREEN]TesManager::processTCP[REGULAR]      ] Type of request is RQS");
      _rqsMutex.lock();
      if(__DEBUG__) UI::Dialog::IO->println("[ [GREEN]TesManager::processTCP[REGULAR]      ] Inserting request on RQS queue");
      _rqsRequests.push(r);
      sem_post(_rqsRequestsSem);
      _rqsMutex.unlock();
      if(__DEBUG__) UI::Dialog::IO->println("[ [GREEN]TesManager::processTCP[REGULAR]      ] Request inserted in RQS queue");
    }else{
      if(__DEBUG__) UI::Dialog::IO->println("[ [GREEN]TesManager::processTCP[REGULAR]      ] Type of request unknown");
      r.message("ERR\n");
      _answerMutex.lock();
      if(__DEBUG__) UI::Dialog::IO->println("[ [GREEN]TesManager::processTCP[REGULAR]      ] Inserting request on RQS queue");
      _answers.push(r);
      sem_post(_answerSem);
      if(__DEBUG__) UI::Dialog::IO->println("[ [GREEN]TesManager::processTCP[REGULAR]      ] Request inserted in RQS queue");
      _answerMutex.unlock();
    }
  }
}

void TesManager::processRQT(){
  if(__DEBUG__) UI::Dialog::IO->println("[ [YELLOW]TesManager::processRQT[REGULAR]      ] BEGIN");
  while(!_exit){
    if(__DEBUG__) UI::Dialog::IO->println("[ [YELLOW]TesManager::processRQT[REGULAR]      ] I'm waiting for requests to process");

    sem_wait(_rqtRequestsSem);

    if(__DEBUG__) UI::Dialog::IO->println("[ [YELLOW]TesManager::processRQT[REGULAR]      ] Client is waiting for answer");
    if(__DEBUG__) UI::Dialog::IO->println(
                    std::string("[ [YELLOW]TesManager::processRQT[REGULAR]      ] Requests size: ") + \
                    std::to_string(_rqtRequests.size()));

    _rqtMutex.lock();
    if(__DEBUG__) UI::Dialog::IO->println("[ [YELLOW]TesManager::processTCP[REGULAR]      ] Removing request from the RQT queue");
    RequestTES r = _rqtRequests.front();
    _rqtRequests.pop();
    if(__DEBUG__) UI::Dialog::IO->println("[ [YELLOW]TesManager::processTCP[REGULAR]      ] Removed request from the RQT queue");
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

      std::pair <std::string, int> pair = pdf(std::to_string(rand() % 5 + 1) + std::string(".pdf"));
      answer += std::to_string(pair.second);
      answer += std::string(" ");
      answer += pair.first;
      answer += std::string("\n");
      r.message(answer);
    }
    _answerMutex.lock();
    if(__DEBUG__) UI::Dialog::IO->println("[ [YELLOW]TesManager::processRQT[REGULAR]      ] Inserting request on Answer queue");
    _answers.push(r);
    sem_post(_answerSem);
    if(__DEBUG__) UI::Dialog::IO->println("[ [YELLOW]TesManager::processRQT[REGULAR]      ] Request inserted in Answer queue");
    _answerMutex.unlock();
  }
}

void TesManager::processRQS(){
  if(__DEBUG__) UI::Dialog::IO->println("[ [MAGENT]TesManager::processRQS[REGULAR]      ] BEGIN");
  while(!_exit){
    if(__DEBUG__) UI::Dialog::IO->println("[ [MAGENT]TesManager::processRQS[REGULAR]      ] I'm waiting for requests to process");

    sem_wait(_rqsRequestsSem);

    if(__DEBUG__) UI::Dialog::IO->println("[ [MAGENT]TesManager::processRQS[REGULAR]      ] Client is waiting for answer");
    if(__DEBUG__) UI::Dialog::IO->println(
                    std::string("[ [MAGENT]TesManager::processRQS[REGULAR]      ] Requests size: ") + \
                    std::to_string(_rqsRequests.size()));

    _rqsMutex.lock();
    if(__DEBUG__) UI::Dialog::IO->println("[ [MAGENT]TesManager::processRQS[REGULAR]      ] Removing request from the RQS queue");
    RequestTES r = _rqsRequests.front();
    _rqsRequests.pop();
    if(__DEBUG__) UI::Dialog::IO->println("[ [MAGENT]TesManager::processRQS[REGULAR]      ] Removed request from the RQS queue");
    _rqsMutex.unlock();

    std::cout << r.read() << std::endl;
    r.message("AQS \n");

    //FIXME

    _answerMutex.lock();
    if(__DEBUG__) UI::Dialog::IO->println("[ [MAGENT]TesManager::processRQS[REGULAR]      ] Inserting request on Answer queue");
    _answers.push(r);
    sem_post(_answerSem);
    if(__DEBUG__) UI::Dialog::IO->println("[ [MAGENT]TesManager::processRQS[REGULAR]      ] Request inserted in Answer queue");
    _answerMutex.unlock();
  }
}

void TesManager::processAWI(){
  //TODO
}

void TesManager::answerTCP(){
  if(__DEBUG__) UI::Dialog::IO->println("[ [BLUE]TesManager::answerTCP[REGULAR]       ] BEGIN");
  while(!_exit){
    if(__DEBUG__) UI::Dialog::IO->println("[ [BLUE]TesManager::answerTCP[REGULAR]       ] I'm waiting for requests to process");

    sem_wait(_answerSem);

    if(__DEBUG__) UI::Dialog::IO->println("[ [BLUE]TesManager::answerTCP[REGULAR]       ] Client is waiting for answer");
    if(__DEBUG__) UI::Dialog::IO->println(
                    std::string("[ [BLUE]TesManager::answerTCP[REGULAR]       ] Requests size: ") + \
                    std::to_string(_rqtRequests.size()));

    _answerMutex.lock();
    if(__DEBUG__) UI::Dialog::IO->println("[ [BLUE]TesManager::answerTCP[REGULAR]       ] Removing request from the RQT queue");
    RequestTES r = _answers.front();
    _answers.pop();
    if(__DEBUG__) UI::Dialog::IO->println("[ [BLUE]TesManager::answerTCP[REGULAR]       ] Removed request from the RQT queue");
    _answerMutex.unlock();

    r.write();
    r.disconnect();
  }
}

std::pair <std::string, int> TesManager::pdf(std::string filename){
  std::ifstream file(filename, std::ifstream::binary);
  if(file){
    file.seekg(0, file.end);
    int length = file.tellg();
    file.seekg(0, file.beg);

    char *buffer = new char[length];

    file.read(buffer, length);
    if (file)
      std::cout << "all characters read successfully.";
    else
      std::cout << "error: only " << file.gcount() << " could be read";
    std::string content(buffer, length);
    delete[] buffer;
    file.close();
    return std::make_pair(content,length);
  }
  return std::make_pair(std::string(), 0);
}
