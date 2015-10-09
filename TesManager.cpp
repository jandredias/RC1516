#include "TesManager.h"
#include "Debug.h"
#include "Dialog.h"
#include "Exception.h"
#include "Quiz.h"
#include <sstream>
#include <fstream>      // std::ifstream
#include <utility>
#include <cstdlib>
#include <ctime>
#include <ostream>
#include <signal.h>

TesManager::TesManager(int port, int ecpPort, std::string ecpName) :
_questionariesSem(new sem_t()),
_requestsSem(new sem_t()), _rqtRequestsSem(new sem_t()),
_rqsRequestsSem(new sem_t()), _awiRequestsSem(new sem_t()),
_answerSem(new sem_t()), _answerUDPSem(new sem_t()), _qid(1), _port(port),
_ecpport(ecpPort), _ecpname(ecpName), _exit(false), _topicName("Hosts"), _senderSocketUDP(new SocketUDP(_ecpname.data(), _ecpport)) {

  sem_init(_requestsSem, 0, 0);
  sem_init(_rqtRequestsSem, 0, 0);
  sem_init(_rqsRequestsSem, 0, 0);
  sem_init(_awiRequestsSem, 0, 0);
  sem_init(_answerUDPSem, 0, 0);
  sem_init(_questionariesSem, 0, 0);

}

TesManager::~TesManager(){
  sem_destroy(_requestsSem);
  sem_destroy(_answerSem);
  sem_destroy(_rqtRequestsSem);
  sem_destroy(_rqsRequestsSem);
  sem_destroy(_awiRequestsSem);
  sem_destroy(_questionariesSem);
  sem_destroy(_answerUDPSem);
}

int TesManager::time(int a){ std::time_t t = std::time((time_t*)&a); return t; }

int TesManager::deadline(int s){ return time() + s; }

std::string TesManager::qid(int sid){
  time_t t = time();   // get time now
  struct tm * now = localtime( & t );
  std::string qid = std::to_string(sid) + "_";
  qid += std::to_string(now->tm_year + 1900);
  if(now->tm_mon < 8) qid += "0";
  qid += std::to_string(now->tm_mon + 1);
  if(now->tm_mday < 9) qid += "0";
  qid += std::to_string(now->tm_mday + 1) + "_";
  if(now->tm_hour < 9) qid += "0";
  qid += std::to_string(now->tm_hour + 1) + ":";
  if(now->tm_min < 9) qid += "0";
  qid += std::to_string(now->tm_min + 1) + ":";
  if(now->tm_sec < 9) qid += "0";
  qid += std::to_string(now->tm_sec + 1);
  return qid;
}

std::string TesManager::toStringDeadline(int s){
  time_t t = time(s);   // get time now
  struct tm * now = localtime( & t );
  std::string qid;

  if(now->tm_mday < 9) qid += "0";
  qid += std::to_string(now->tm_mday + 1);
  switch(now->tm_mon + 1){
  case 1:
    qid += "JAN";
    break;
  case 2:
    qid += "FEB";
    break;
  case 3:
    qid += "MAR";
    break;
  case 4:
    qid += "APR";
    break;
  case 5:
    qid += "MAY";
    break;
  case 6:
    qid += "JUN";
    break;
  case 7:
    qid += "JUL";
    break;
  case 8:
    qid += "AUG";
    break;
  case 9:
    qid += "SEP";
    break;
  case 10:
    qid += "OCT";
    break;
  case 11:
    qid += "NOV";
    break;
  case 12:
    qid += "DEC";
  }

  qid += std::to_string(now->tm_year + 1900) + "_";
  if(now->tm_hour < 9) qid += "0";
  qid += std::to_string(now->tm_hour + 1) + ":";
  if(now->tm_min < 9) qid += "0";
  qid += std::to_string(now->tm_min + 1) + ":";
  if(now->tm_sec < 9) qid += "0";
  qid += std::to_string(now->tm_sec + 1);
  return qid;
}
bool TesManager::deadline(std::string s){

  //TODO
  return true;
}

void TesManager::acceptRequestsTCP(){

  debug("[ [BLUE]TesManager::acceptRequestsTC[REGULAR]] Creating socket");

  try{
    SocketTCP _socketTCP(_port);

    debug("[ [BLUE]TesManager::acceptRequestsTC[REGULAR]] Socket created ");
    debug(
      std::string("[ [BLUE]TesManager::acceptRequestsTC[REGULAR]] Listening on port ")\
      + std::to_string(_port));

    _socketTCP.listen(10);

    while(!_exit){
      debug("[ [BLUE]TesManager::acceptRequestsTC[REGULAR]] Waiting for clients");
      debug(
        std::string("[ [BLUE]TesManager::acceptRequestsTC[REGULAR]]  Requests size ")\
        + std::to_string(_rqtRequests.size()));

      SocketTCP s = _socketTCP.accept();

      debug("[ [BLUE]TesManager::acceptRequestsTC[REGULAR]] Reading from client");
      debug("[ [BLUE]TesManager::acceptRequestsTC[REGULAR]] Client request read");

      RequestTES r = RequestTES(s);

      _reqMutex.lock();
      _requests.push(r);
      _reqMutex.unlock();

      debug("[ [BLUE]TesManager::acceptRequestsTC[REGULAR]] Client connected");

      sem_post(_requestsSem);
    }
  }catch(SocketAlreadyInUse s){
    UI::Dialog::IO->println(s.message());
    _exit = true;
    sem_post(_requestsSem);
    for(int i = 0; i <= ANSWER_NO; i++) sem_post(_answerSem);
    sem_post(_rqtRequestsSem);
    sem_post(_rqsRequestsSem);
    sem_post(_awiRequestsSem);
  }catch(std::string s){
    UI::Dialog::IO->println(s);
    _exit = 1;
    sem_post(_requestsSem);
    for(int i = 0; i < ANSWER_NO; i++) sem_post(_answerSem);
    sem_post(_rqtRequestsSem);
    sem_post(_rqsRequestsSem);
    sem_post(_awiRequestsSem);
    return;
  }
  debug("Leaving thread acceptRequestsTCP. Bye!");
}

void TesManager::acceptRequestsUDP(){
  debug("[[CYAN]TesManager::acceptRequestsUDP[REGULAR]] Creating socket");

  try{
    SocketUDP _socketUDP(_port);
    debug("[[CYAN]TesManager::acceptRequestsUDP[REGULAR]] Socket created ");
    debug(
      std::string("[[CYAN]TesManager::acceptRequestsUDP[REGULAR]] Waiting for message on port ")\
      + std::to_string(_port));
    while(!_exit){
      debug(
        std::string("[[CYAN]TesManager::acceptRequestsUDP[REGULAR]] Waiting for message on port ")\
        + std::to_string(_port));

      _receiverSocketUDPMutex.lock();
      std::string message = _socketUDP.receive(); //Throws exception
      _receiverSocketUDPMutex.unlock();

      debug(
        std::string("[[CYAN]TesManager::acceptRequestsUDP[REGULAR]]  Queue size ")\
        + std::to_string(_rqtRequests.size()));

      debug(
        std::string("[ [GREEN]ECPManager::acceptRequestsUDP[REGULAR]  ] Size of Message: ") +\
        std::to_string(message.size()));

      RequestTES r(message);

      debug("[ [CYAN]TesManager::acceptRequestsUDP[REGULAR]] "
                              "Waiting for mutex to be unlocked and inserting request on queue");
      _awiMutex.lock();
      _awiRequests.push(r);
      _awiMutex.unlock();
      debug("[ [CYAN]TesManager::acceptRequestsUDP[REGULAR]] "
                              "Request inserted on queue and mutex unlocked");

      sem_post(_awiRequestsSem);
      debug("[ [CYAN]TesManager::acceptRequestsUDP[REGULAR]] Semaphore posted");
    }
  }catch(std::string s){
    UI::Dialog::IO->println(s);
  }catch(SocketAlreadyInUse s){
    UI::Dialog::IO->println(s.message());
    _exit = true;
    sem_post(_requestsSem);
    for(int i = 0; i <= ANSWER_NO; i++) sem_post(_answerSem);
    sem_post(_rqtRequestsSem);
    sem_post(_rqsRequestsSem);
    sem_post(_awiRequestsSem);
  }catch(UnknownFormatProtocol s){
    UI::Dialog::IO->println("Message received using an unknown protocol");
    _receiverSocketUDPMutex.unlock();
  }catch(MessageTooLongUDP s){
    UI::Dialog::IO->println("Message too long to be processed");
    _receiverSocketUDPMutex.unlock();
  }
  debug("Leaving thread acceptRequestsUDP. Bye!");
}

void TesManager::processTCP(){

  debug("[ [GREEN]TesManager::processTCP[REGULAR]      ] processRequests");

  while(!_exit){
    sem_wait(_requestsSem);
    if(_exit){
      debug("Exiting the thread processTCP due to exit flag");
      return;
    }
    debug("[ [GREEN]TesManager::processTCP[REGULAR]      ] Client is waiting for answer");
    debug(
      std::string("[ [GREEN]TesManager::processTCP[REGULAR]      ] Requests size: ") + \
      std::to_string(_requests.size()));

    debug("[ [GREEN]TesManager::processTCP[REGULAR]      ] Locking ReqMutex");
    _reqMutex.lock();
    debug("[ [GREEN]TesManager::processTCP[REGULAR]      ] Locked ReqMutex");

    debug("[ [GREEN]TesManager::processTCP[REGULAR]      ] Getting first in the queue");
    RequestTES r = _requests.front();
    debug("[ [GREEN]TesManager::processTCP[REGULAR]      ] Got first in the queue");

    debug("[ [GREEN]TesManager::processTCP[REGULAR]      ] Removing first from the queue");
    _requests.pop();
    debug("[ [GREEN]TesManager::processTCP[REGULAR]      ] Removed first from the queue");

    debug("[ [GREEN]TesManager::processTCP[REGULAR]      ] Requests size: " +
      std::to_string(_requests.size()));

    debug("[ [GREEN]TesManager::processTCP[REGULAR]      ] Unlocking ReqMutex");
    _reqMutex.unlock();
    debug("[ [GREEN]TesManager::processTCP[REGULAR]      ] Unlocked ReqMutex");

    std::string typeOfRequest;

    try{
      debug("[ [GREEN]TesManager::processTCP[REGULAR]      ] Reading message from Request");
      r.message(r.read());
      debug("[ [GREEN]TesManager::processTCP[REGULAR]      ] Message read from Request");

      debug("[ [GREEN]TesManager::processTCP[REGULAR]      ] Converting R.Message() to stringstream");
      std::stringstream stream(r.message());
      debug("[ [GREEN]TesManager::processTCP[REGULAR]      ] R.Message to Stream Complete");

      stream >> typeOfRequest;

    }catch(ConnectionTCPTimedOut s){
      typeOfRequest = "ERR";
    }

    if(typeOfRequest == std::string("RQT")){
      debug("[ [GREEN]TesManager::processTCP[REGULAR]      ] Type of request is RQT");
      debug("[ [GREEN]TesManager::processTCP[REGULAR]      ] Locking RQTMutex");

      _rqtMutex.lock();

      debug("[ [GREEN]TesManager::processTCP[REGULAR]      ] Locked RQTMutex");
      debug("[ [GREEN]TesManager::processTCP[REGULAR]      ] Inserting request on RQT queue");

      UI::Dialog::IO->println("RQT from " + r.client().ip());

      _rqtRequests.push(r);
      sem_post(_rqtRequestsSem);

      debug("[ [GREEN]TesManager::processTCP[REGULAR]      ] Request inserted in RQT queue");
      debug("[ [GREEN]TesManager::processTCP[REGULAR]      ] Unlocking RQTMutex");
      _rqtMutex.unlock();
      debug("[ [GREEN]TesManager::processTCP[REGULAR]      ] Unlocked RQTMutex");

    }else if(typeOfRequest == std::string("RQS")){
      debug("[ [GREEN]TesManager::processTCP[REGULAR]      ] Type of request is RQS");
      debug("[ [GREEN]TesManager::processTCP[REGULAR]      ] Locking RQSMutex");

      _rqsMutex.lock();

      debug("[ [GREEN]TesManager::processTCP[REGULAR]      ] Locked RQSMutex");
      debug("[ [GREEN]TesManager::processTCP[REGULAR]      ] Inserting request on RQS queue");

      UI::Dialog::IO->println("RQS from " + r.client().ip());

      _rqsRequests.push(r);
      sem_post(_rqsRequestsSem);

      debug("[ [GREEN]TesManager::processTCP[REGULAR]      ] Request inserted in RQS queue");
      debug("[ [GREEN]TesManager::processTCP[REGULAR]      ] Unlocking RQSMutex");
      _rqsMutex.unlock();

      debug("[ [GREEN]TesManager::processTCP[REGULAR]      ] Unlocked RQSMutex");

    }else{

      debug("[ [GREEN]TesManager::processTCP[REGULAR]      ] Type of request unknown");

      std::string answer = std::string("ERR\n");

      UI::Dialog::IO->println("ERR from " + r.client().ip());
      r.answer(answer);
      _answerMutex.lock();

      debug("[ [GREEN]TesManager::processTCP[REGULAR]      ] Inserting request on answer queue");

      _answers.push(r);
      sem_post(_answerSem);

      debug("[ [GREEN]TesManager::processTCP[REGULAR]      ] Request inserted in answer queue");

      _answerMutex.unlock();
    }
  }
  debug("Leaving thread processTCP. Bye!");
}

void TesManager::processRQT(){
  debug("[ [YELLOW]TesManager::processRQT[REGULAR]      ] BEGIN");

  void (*old_handler)(int);//interrupt handler
  if((old_handler=signal(SIGPIPE,SIG_IGN))==SIG_ERR) UI::Dialog::IO->println("ERRO PIPE");

  while(!_exit){

    debug("[ [YELLOW]TesManager::processRQT[REGULAR]      ] I'm waiting for requests to process");


    sem_wait(_rqtRequestsSem);
    if(_exit){
      debug("Exiting the thread processRQT due to exit flag");
      return;
    }

    debug("[ [YELLOW]TesManager::processRQT[REGULAR]      ] Client is waiting for answer");
    debug(
      std::string("[ [YELLOW]TesManager::processRQT[REGULAR]      ] Requests size: ") + \
      std::to_string(_rqtRequests.size()));

    _rqtMutex.lock();

    debug("[ [YELLOW]TesManager::processTCP[REGULAR]      ] Removing request from the RQT queue");

    RequestTES r = _rqtRequests.front();
    _rqtRequests.pop();

    debug("[ [YELLOW]TesManager::processTCP[REGULAR]      ] Removed request from the RQT queue");

    _rqtMutex.unlock();

   /* UI::Dialog::IO->println("TQR" + std::string(" from ") + r.client().ip() +\
      std::string(" on port "));// + r.client().port());*/

    std::stringstream stream(r.message());

    std::string code;
    std::string SIDstr;
    std::string trash;
    int SID;

    stream >> code;
    stream >> SIDstr;
    stream >> trash;
    //int tID;
    bool is_number = true;
    for(int index = 0; index < (int) SIDstr.size(); index++)
      if(SIDstr[index] < '0' || SIDstr[index] > '9') is_number = false;
    if(is_number) {
      SID = atoi(SIDstr.data());
      if(SID < 10000 || SID > 99999) is_number = false;
    }

    if(SIDstr.size() == 0 || !is_number || trash != std::string("")){
      r.answer("ERR\n");
    }else{

      r.sid(SID);
      r.qid(qid(SID));
      r.deadline(deadline());
      std::string answer;
      answer  = std::string("AQT ");
      answer += r.qid();
      answer += std::string(" ");
      answer += toStringDeadline(deadline());
      answer += std::string(" ");

      std::string filename = std::to_string(rand() % 5 + 1) + std::string(".pdf");
      UI::Dialog::IO->println("Sending file " + filename);

      _questionariesMutex.lock();
      _questionaries[r.qid()] = Quiz(r.sid(), r.deadline(), filename);
      _questionariesMutex.unlock();

      debug("[ [YELLOW]TesManager::processRQT[REGULAR]      ] Reading file: " + filename);
      std::string fileName = std::to_string(rand() % 5 + 1) + std::string(".pdf");
      int fileSize = pdfSize(fileName);

      debug("[ [YELLOW]TesManager::processRQT[REGULAR]      ] File read");
      answer += std::to_string(fileSize);
      answer += std::string(" ");
      r.fileSize(fileSize);
      r.fileName(fileName);

      debug("[ [YELLOW]TesManager::processRQT[REGULAR]      ] Answer before data: " + answer);
      r.answer(answer);

      debug(r.answer());

    }
    _answerMutex.lock();

    debug("[ [YELLOW]TesManager::processRQT[REGULAR]      ] Inserting request on Answer queue");

    _answers.push(r);
    sem_post(_answerSem);

    debug("[ [YELLOW]TesManager::processRQT[REGULAR]      ] Request inserted in Answer queue");

    _answerMutex.unlock();
  }
  debug("Leaving thread processRQT. Bye!");
}

void TesManager::processRQS(){

  debug("[ [MAGENT]TesManager::processRQS[REGULAR]      ] BEGIN");
  while(!_exit){

    debug("[ [MAGENT]TesManager::processRQS[REGULAR]      ] I'm waiting for requests to process");

    sem_wait(_rqsRequestsSem);
    if(_exit){
      debug("Exiting the thread processRQS due to exit flag");
      return;
    }

    debug("[ [MAGENT]TesManager::processRQS[REGULAR]      ] Client is waiting for answer");
    debug(
      std::string("[ [MAGENT]TesManager::processRQS[REGULAR]      ] Requests size: ") + \
      std::to_string(_rqsRequests.size()));



    debug("[ [MAGENT]TesManager::processRQS[REGULAR]      ] Removing request from the RQS queue");

    _rqsMutex.lock();
    RequestTES r = _rqsRequests.front();
    _rqsRequests.pop();
    _rqsMutex.unlock();

    debug("[ [MAGENT]TesManager::processRQS[REGULAR]      ] Removed request from the RQS queue");
    debug("[ [MAGENT]TesManager::processRQS[REGULAR]      ] Message:" + r.message());

  	std::string req;
  	std::string sid;
  	std::string qid;
  	std::string tmp;
  	char answers[5];

  	std::stringstream stream(r.message());

  	stream >> req;
  	stream >> sid;
    stream >> qid;

  	stream >> tmp; answers[0] = tmp.data()[0];
    if(tmp.size() != 1) throw UnknownFormatProtocol();
  	stream >> tmp; answers[1] = tmp.data()[0];
    if(tmp.size() != 1) throw UnknownFormatProtocol();
  	stream >> tmp; answers[2] = tmp.data()[0];
    if(tmp.size() != 1) throw UnknownFormatProtocol();
  	stream >> tmp; answers[3] = tmp.data()[0];
    if(tmp.size() != 1) throw UnknownFormatProtocol();
  	stream >> tmp; answers[4] = tmp.data()[0];
    if(tmp.size() != 1) throw UnknownFormatProtocol();

    debug("[ [MAGENT]TesManager::processRQS[REGULAR]      ] Message received and is going to be parsed");
    try{
      debug("[ [MAGENT]TesManager::processRQS[REGULAR]      ] Parsing message");
      if(sid == std::string("") || qid == std::string("")) throw UnknownFormatProtocol();
      for(int i = 0; i < 5; i++)
        if((answers[i] < 'A' || answers[i] > 'D') && answers[i] != 'N') throw UnknownFormatProtocol();

      debug("[ [MAGENT]TesManager::processRQS[REGULAR]      ] Message parsed");

      std::map<std::string,Quiz>::iterator it;

      debug("[ [MAGENT]TesManager::processRQS[REGULAR]      ] Searching for questionnaire on queue");
      debug("[ [MAGENT]TesManager::processRQS[REGULAR]      ] Locking mutex");

      _questionariesMutex.lock();
      debug("[ [MAGENT]TesManager::processRQS[REGULAR]      ] Mutex locked");
      debug("[ [MAGENT]TesManager::processRQS[REGULAR]      ] Finding qid");

      it = _questionaries.find(qid);

      debug("[ [MAGENT]TesManager::processRQS[REGULAR]      ] Search finished");

      Quiz quiz;

      if (it == _questionaries.end())
        throw InvalidQIDvsSID();

      debug("[ [MAGENT]TesManager::processRQS[REGULAR]      ] QID found");
      debug("[ [MAGENT]TesManager::processRQS[REGULAR]      ] Search finished");

      quiz = it->second;
      _questionaries.erase(it);
      debug("[ [MAGENT]TesManager::processRQS[REGULAR]      ] Unlocking Mutex");
      _questionariesMutex.unlock();
      debug("[ [MAGENT]TesManager::processRQS[REGULAR]      ] Mutex unlocked");
      std::string file = quiz.filename();
      int deadline = quiz.deadline();

      int scr;
      if(atoi(sid.data()) != quiz.sid()){
        scr = -2;
      }
      else if(time() < deadline)
    	   scr = score(answers,file.c_str());
      else
         scr = -1;
    	r.answer("AQS " + qid + " " + std::to_string(scr) + "\n");
      if (scr >= 0){
        RequestTES iqrRequest = RequestTES("");
        iqrRequest.answer(
          "IQR " + std::string(sid) + std::string(" ") +
         std::string(qid) + std::string(" ") + std::string(_topicName) +
         std::string(" ") + std::to_string(scr) + std::string("\n"));

        _answerUDPMutex.lock();
        _answersUDP.insert(std::pair<std::string, RequestTES>(iqrRequest.qid(), iqrRequest));
        _answerUDPMutex.unlock();
        sem_post(_answerUDPSem);
      }
    }catch(UnknownFormatProtocol s){
      debug("[ [MAGENT]TesManager::processRQS[REGULAR]      ] Unknown format protocol");
      r.answer("ERR\n");
      debug("[ [MAGENT]TesManager::processRQS[REGULAR]      ] Unlocking Mutex");
      _questionariesMutex.unlock();
      debug("[ [MAGENT]TesManager::processRQS[REGULAR]      ] Mutex unlocked");
    }catch(InvalidQIDvsSID s){
      r.answer("AQS " + qid + " -2\n");
		debug("[ [MAGENT]TesManager::processRQS[REGULAR]      ] Unlocking Mutex");
      _questionariesMutex.unlock();
		debug("[ [MAGENT]TesManager::processRQS[REGULAR]      ] Mutex unlocked");
    }

    debug("[ [MAGENT]TesManager::processRQS[REGULAR]      ] Inserting request on Answer queue");

    _answerMutex.lock();
    _answers.push(r);
    _answerMutex.unlock();

    sem_post(_answerSem);

    debug("[ [MAGENT]TesManager::processRQS[REGULAR]      ] Request inserted in Answer queue");
  }
  debug("Leaving thread processRQS. Bye!");
}
void TesManager::processAWI(){


  debug("[ [CYAN]TesManager::processAWI[REGULAR]      ] BEGIN");

  while(!_exit){

    debug("[ [CYAN]TesManager::processAWI[REGULAR]      ] I'm waiting for requests to process");

    sem_wait(_awiRequestsSem);
    if(_exit){
      debug("Exiting the thread processTCP due to exit flag");
      return;
    }

    debug("[ [CYAN]TesManager::processAWI[REGULAR]      ] Client is waiting for answer");
    debug(
      std::string("[ [CYAN]TesManager::processAWI[REGULAR]      ] Requests size: ") + \
      std::to_string(_awiRequests.size()));

    _awiMutex.lock();

    debug("[ [CYAN]TesManager::processAWI[REGULAR]      ] Removing request from the AWI queue");
    RequestTES r = _awiRequests.front();
    _awiRequests.pop();

    debug("[ [CYAN]TesManager::processAWI[REGULAR]      ] Removed request from the AWI queue");
    debug("[ [CYAN]TesManager::processAWI[REGULAR]      ] Message:" + r.message());

    _awiMutex.unlock();

    std::stringstream stream(r.message());
    std::string message;
    std::string qid;
    std::string trash;
    stream >> message;
    stream >> qid;
    stream >> trash;

    if(message == std::string("AWI") && qid != std::string("") &&
       qid.size() <= 24 && qid.size() > 0 && trash == std::string("")){
      _answerUDPMutex.lock();
      if(_answersUDP.count(qid) != 0)
        _answersUDP.erase(_answersUDP.find(qid));
      _answerUDPMutex.unlock();
    }else if(message == std::string("ERR") || message != std::string("AWI") || trash != std::string("")){
      UI::Dialog::IO->println("[RED][ERR][REGULAR] There was an error in the communication with the server.");
      UI::Dialog::IO->println("Try again.");
    }
  }
  debug("Leaving thread processAWI. Bye!");
}

void TesManager::answerUDP(){

  debug("[ [BLUE]TesManager::answerUDP[REGULAR]       ] BEGIN");

  while(!_exit){

    debug("[ [BLUE]TesManager::answerUDP[REGULAR]       ] I'm waiting for requests to process");

    sem_wait(_answerUDPSem);
    if(_exit){
      debug("Exiting the thread answerUDP due to exit flag");
      return;
    }

    debug(
      std::string("[ [BLUE]TesManager::answerUDP[REGULAR]       ] Requests size: ") + \
      std::to_string(_answersUDP.size()));
    debug("[ [BLUE]TesManager::answerUDP[REGULAR]       ] Removing request from the ANSWER queue");

    _answerUDPMutex.lock();
    RequestTES r = _answersUDP.begin()->second;
    _answersUDP.erase(_answersUDP.begin()); //Removing from the unordered_map
    _answerUDPMutex.unlock();

    debug("[ [BLUE]TesManager::answerUDP[REGULAR]       ] Removed request from the ANSWER queue");
    if(r.answer().size() < 100) debug(r.answer());

    _senderSocketUDP->send(r.answer());


    _answerUDPMutex.lock();
    _answersUDP.insert(std::pair<std::string, RequestTES>(r.qid(), r));
    _answerUDPMutex.unlock();
    usleep(500);
  }
  debug("Leaving thread answerUDP. Bye!");
}
void TesManager::answerTCP(){

  debug("[ [BLUE]TesManager::answerTCP[REGULAR]       ] BEGIN");

  while(!_exit){

    debug("[ [BLUE]TesManager::answerTCP[REGULAR]       ] I'm waiting for requests to process");

    sem_wait(_answerSem);
    if(_exit){
      debug("Exiting the thread answerTCP due to exit flag");
      return;
    }


    debug("[ [BLUE]TesManager::answerTCP[REGULAR]       ] Client is waiting for answer");


    debug(
      std::string("[ [BLUE]TesManager::answerTCP[REGULAR]       ] Requests size: ") + \
      std::to_string(_rqtRequests.size()));
    debug("[ [BLUE]TesManager::answerTCP[REGULAR]       ] Removing request from the ANSWER queue");

    _answerMutex.lock();
    RequestTES r = _answers.front();
    _answers.pop();
    _answerMutex.unlock();

    debug("[ [BLUE]TesManager::answerTCP[REGULAR]       ] Removed request from the ANSWER queue");
    if(r.answer().size() < 100) debug(r.answer());
    try{
      r.write();
    }catch(SocketClosed e){
      UI::Dialog::IO->println("[RED]Socket closed by client!");
    }
    r.disconnect();
  }
  debug("Leaving thread answerTCP. Bye!");
}

int TesManager::pdfSize(std::string filename){
  std::ifstream file(filename, std::ifstream::binary);
  if(file){
    file.seekg(0, file.end);
    int length = file.tellg();
    file.seekg(0, file.beg);

    file.close();
    return length;
  }
  throw ErrorOpeningFile();
}

int TesManager::score(char answers[], const char filename[]){
  std::ifstream in("answers.txt");
  int scoreValue = 0;
  if(in){
    while(1){
      std::string name;
      std::string aux;
      char ans[5];
      in >> name;
      for(int i = 0; i < 5; i++){
        in >> aux;
        ans[i] = aux.data()[0];
      }
      if(name == std::string(filename)){
        for(int i = 0; i < 5; i++){
          if(answers[i] != 'N' && answers[i] != 'n')
            scoreValue += (answers[i] == ans[i]) ? 20 : -5;
        }
        scoreValue = (scoreValue < 0) ? 0 : scoreValue;
        UI::Dialog::IO->print("Pontuacao: ");
        UI::Dialog::IO->println(std::to_string(scoreValue));
        break;
      }
      if(in.eof()) break;
    }
  }
  in.close();
  return scoreValue;
}

void TesManager::sendIQR(std::string SID,std::string QID,std::string topic_name,int scr){

  debug("[ TesManager::sendIQR             ] ECP server data:");
  debug("[ TesManager::sendIQR             ] Server Name: " + _ecpname);
  debug("[ TesManager::sendIQR             ] Server Port: " + std::to_string(_ecpport));

  SocketUDP ecp = SocketUDP(_ecpname.data(), _ecpport );
  std::string message;
  std::string QID_Received;

  debug("[ TesManager::sendIQR             ] Socket created");
  debug("[ TesManager::sendIQR             ] Sending message");

  std::stringstream stream;
  for(auto i = 0; i < __TRIES__; i++){
    ecp.send(std::string("IQR ") + std::string(SID) + std::string(" ") + std::string(QID) + std::string(" ") + std::string(topic_name) + std::string(" ") + std::to_string(scr) + std::string("\n"));
    try{
      ecp.timeout(__MS_BETWEEN_TRIES__);
      stream << ecp.receive();

      debug("[ TesManager::sendIQR             ] Message sent to ECP");
      debug("[ TesManager::sendIQR             ] Receiving message from ECP");
      debug("[ TesManager::sendIQR             ] Message received from ECP");
      break;
    }catch(std::string s){
      if(errno == 11){
        UI::Dialog::IO->print(". ");
        UI::Dialog::IO->flush();
        if(i < __TRIES__ - 1)
          continue;
        UI::Dialog::IO->println();
        UI::Dialog::IO->println();
      }else{
        UI::Dialog::IO->println("error sending message to ECP server");
      }
    }
    UI::Dialog::IO->println();
    UI::Dialog::IO->println("Could not connect to ECP Server!");
    UI::Dialog::IO->println("Try again later.");
    return;
  }


}
