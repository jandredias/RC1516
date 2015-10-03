#include "TesManager.h"
#include "Dialog.h"
#include "Exception.h"
#include "Quiz.h"
#include <sstream>
#include <fstream>      // std::ifstream
#include <utility>
#include <cstdlib>
#include <ctime>
#include <ostream>

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

int TesManager::time(){ std::time_t t = std::time(0); return t; }

int TesManager::deadline(int s){ return time() + s; }

std::string TesManager::qid(){
  time_t t = time();   // get time now
 struct tm * now = localtime( & t );
  std::string qid = std::to_string(now->tm_year + 1900) + ":";
  qid += (now->tm_mon + 1 < 9) ? "0" + std::to_string(now->tm_mon + 1) : std::to_string(now->tm_mon + 1);
  qid += ":";
  qid += (now->tm_mday < 9) ? "0" + std::to_string(now->tm_mday) : std::to_string(now->tm_mday);
  qid += "|HH:MM" + std::to_string(_qid++);
  return qid;
}

void TesManager::acceptRequestsTCP(){

  #if DEBUG
  UI::Dialog::IO->println("[ [BLUE]TesManager::acceptRequestsTC[REGULAR]] Creating socket");
  #endif

  try{
    SocketTCP _socketTCP(_port);

    #if DEBUG
    UI::Dialog::IO->println("[ [BLUE]TesManager::acceptRequestsTC[REGULAR]] Socket created ");
    UI::Dialog::IO->println(
      std::string("[ [BLUE]TesManager::acceptRequestsTC[REGULAR]] Listening on port ")\
      + std::to_string(_port));
    #endif

    _socketTCP.listen(10);

    while(!_exit){
      #if DEBUG
      UI::Dialog::IO->println("[ [BLUE]TesManager::acceptRequestsTC[REGULAR]] Waiting for clients");
      UI::Dialog::IO->println(
        std::string("[ [BLUE]TesManager::acceptRequestsTC[REGULAR]]  Requests size ")\
        + std::to_string(_rqtRequests.size()));
      #endif

      SocketTCP s = _socketTCP.accept();

      #if DEBUG
      UI::Dialog::IO->println("[ [BLUE]TesManager::acceptRequestsTC[REGULAR]] Reading from client");
      UI::Dialog::IO->println("[ [BLUE]TesManager::acceptRequestsTC[REGULAR]] Client request read");
      #endif

      RequestTES r = RequestTES(s);

      _reqMutex.lock();
      _requests.push(r);
      _reqMutex.unlock();

      #if DEBUG
      UI::Dialog::IO->println("[ [BLUE]TesManager::acceptRequestsTC[REGULAR]] Client connected");
      #endif

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
  #if DEBUG
  UI::Dialog::IO->println("Leaving thread acceptRequestsTCP. Bye!");
  #endif
}

void TesManager::acceptRequestsUDP(){
  #if DEBUG
  UI::Dialog::IO->println("[[CYAN]TesManager::acceptRequestsUDP[REGULAR]] Creating socket");
  #endif

  try{
    SocketUDP _socketUDP(_port);
    #if DEBUG
    UI::Dialog::IO->println("[[CYAN]TesManager::acceptRequestsUDP[REGULAR]] Socket created ");
    UI::Dialog::IO->println(
      std::string("[[CYAN]TesManager::acceptRequestsUDP[REGULAR]] Waiting for message on port ")\
      + std::to_string(_port));
    #endif
    while(!_exit){
      #if DEBUG
      UI::Dialog::IO->println(
        std::string("[[CYAN]TesManager::acceptRequestsUDP[REGULAR]] Waiting for message on port ")\
        + std::to_string(_port));
      #endif

      _receiverSocketUDPMutex.lock();
      std::string message = _socketUDP.receive();
      _receiverSocketUDPMutex.unlock();

      #if DEBUG
      UI::Dialog::IO->println(
        std::string("[[CYAN]TesManager::acceptRequestsUDP[REGULAR]]  Queue size ")\
        + std::to_string(_rqtRequests.size()));
      #endif

      #if DEBUG
      UI::Dialog::IO->println(
        std::string("[ [GREEN]ECPManager::acceptRequestsUDP[REGULAR]  ] Size of Message: ") +\
        std::to_string(message.size()));
      #endif

      RequestTES r(message);

      #if DEBUG
      UI::Dialog::IO->println("[ [CYAN]TesManager::acceptRequestsUDP[REGULAR]] "
                              "Waiting for mutex to be unlocked and inserting request on queue");
      #endif
      _awiMutex.lock();
      _awiRequests.push(r);
      _awiMutex.unlock();
      #if DEBUG
      UI::Dialog::IO->println("[ [CYAN]TesManager::acceptRequestsUDP[REGULAR]] "
                              "Request inserted on queue and mutex unlocked");
      #endif

      sem_post(_awiRequestsSem);
      #if DEBUG
      UI::Dialog::IO->println("[ [CYAN]TesManager::acceptRequestsUDP[REGULAR]] Semaphore posted");
      #endif
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
  }catch(MessageTooLongUDP s){
    UI::Dialog::IO->println("Message too long to be processed");
  }
  #if DEBUG
  UI::Dialog::IO->println("Leaving thread acceptRequestsUDP. Bye!");
  #endif
}

void TesManager::processTCP(){

  #if DEBUG
  UI::Dialog::IO->println("[ [GREEN]TesManager::processTCP[REGULAR]      ] processRequests");
  #endif

  while(!_exit){
    sem_wait(_requestsSem);
    if(_exit){
      #if DEBUG
      UI::Dialog::IO->println("Exiting the thread processTCP due to exit flag");
      #endif
      return;
    }
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

      UI::Dialog::IO->println("RQT from " + r.client().ip());


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

      UI::Dialog::IO->println("RQS from " + r.client().ip());
      _rqsRequests.push(r);
      sem_post(_rqsRequestsSem);
      _rqsMutex.unlock();

      #if DEBUG
      UI::Dialog::IO->println("[ [GREEN]TesManager::processTCP[REGULAR]      ] Request inserted in RQS queue");
      #endif

    }else {

      #if DEBUG
      UI::Dialog::IO->println("[ [GREEN]TesManager::processTCP[REGULAR]      ] Type of request unknown");
      #endif
	    UI::Dialog::IO->println("[ [RED]DONT CARE -- ERROR[REGULAR]          ] Type of request unknown");
      std::string answer = std::string("ERR");

      UI::Dialog::IO->println("ERR from " + r.client().ip());
      r.answer(answer);
      _answerMutex.lock();

      #if DEBUG
      UI::Dialog::IO->println("[ [GREEN]TesManager::processTCP[REGULAR]      ] Inserting request on answer queue");
      #endif

      _answers.push(r);
      sem_post(_answerSem);

      #if DEBUG
      UI::Dialog::IO->println("[ [GREEN]TesManager::processTCP[REGULAR]      ] Request inserted in answer queue");
      #endif

      _answerMutex.unlock();
    }
  }
  #if DEBUG
  UI::Dialog::IO->println("Leaving thread processTCP. Bye!");
  #endif
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
    if(_exit){
      #if DEBUG
      UI::Dialog::IO->println("Exiting the thread processRQT due to exit flag");
      #endif
      return;
    }

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

    if(SIDstr.size() == 0 || !is_number || trash != std::string("")){
      r.answer("ERR");
    }else{
      SID = atoi(SIDstr.data());
      r.sid(SID);
      r.qid(qid());
      r.deadline(deadline());
      std::string answer;
      answer  = std::string("AQT ");
      answer += r.qid();
      answer += std::string(" ");
      answer += std::to_string(deadline());
      answer += std::string(" ");

      std::string filename = std::to_string(rand() % 5 + 1) + std::string(".pdf");
      UI::Dialog::IO->println("Sending file " + filename);

      _questionariesMutex.lock();
      _questionaries[r.qid()] = Quiz(r.deadline(), filename);
      _questionariesMutex.unlock();

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
  #if DEBUG
  UI::Dialog::IO->println("Leaving thread processRQT. Bye!");
  #endif
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
    if(_exit){
      #if DEBUG
      UI::Dialog::IO->println("Exiting the thread processRQS due to exit flag");
      #endif
      return;
    }

    #if DEBUG
    UI::Dialog::IO->println("[ [MAGENT]TesManager::processRQS[REGULAR]      ] Client is waiting for answer");
    UI::Dialog::IO->println(
      std::string("[ [MAGENT]TesManager::processRQS[REGULAR]      ] Requests size: ") + \
      std::to_string(_rqsRequests.size()));
    #endif


    #if DEBUG
    UI::Dialog::IO->println("[ [MAGENT]TesManager::processRQS[REGULAR]      ] Removing request from the RQS queue");
    #endif

    _rqsMutex.lock();
    RequestTES r = _rqsRequests.front();
    _rqsRequests.pop();
    _rqsMutex.unlock();

    #if DEBUG
    UI::Dialog::IO->println("[ [MAGENT]TesManager::processRQS[REGULAR]      ] Removed request from the RQS queue");
    UI::Dialog::IO->println("[ [MAGENT]TesManager::processRQS[REGULAR]      ] Message:" + r.message());
    #endif

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
  	stream >> tmp; answers[1] = tmp.data()[0];
  	stream >> tmp; answers[2] = tmp.data()[0];
  	stream >> tmp; answers[3] = tmp.data()[0];
  	stream >> tmp; answers[4] = tmp.data()[0];


    #if DEBUG
  	for(int i=0;i<5;i++){
  		UI::Dialog::IO->print(std::string("[[MAGENT]TesManager::processRQS[REGULAR]] answers:") + answers[i]+ '\n') ;
  	}
    #endif

    try{
      if(sid == std::string("") || qid == std::string("")) throw UnknownFormatProtocol();
      for(int i = 0; i < 5; i++)
        if(answers[i] < 'A' && answers[i] > 'D' && answers[i] != 'N') throw UnknownFormatProtocol();

      _questionariesMutex.lock();
      Quiz quiz = _questionaries.find(qid)->second;
      _questionariesMutex.unlock();

      std::string file = quiz.filename();

    	int scr = score(answers,file.c_str());
    	r.answer("AQS " + qid + " " + std::to_string(scr) + "\n");

      RequestTES iqrRequest = RequestTES("");
      iqrRequest.answer(
        "IQR " + std::string(sid) + std::string(" ") +
       std::string(qid) + std::string(" ") + std::string(_topicName) +
       std::string(" ") + std::to_string(scr) + std::string("\n"));

      _answerUDPMutex.lock();
      _answersUDP.insert(std::pair<std::string, RequestTES>(iqrRequest.qid(), iqrRequest));
      _answerUDPMutex.unlock();
      sem_post(_answerUDPSem);
    }catch(UnknownFormatProtocol s){
      r.answer("ERR\n");
    }

    #if DEBUG
    UI::Dialog::IO->println("[ [MAGENT]TesManager::processRQS[REGULAR]      ] Inserting request on Answer queue");
    #endif

    _answerMutex.lock();
    _answers.push(r);
    _answerMutex.unlock();

    sem_post(_answerSem);

    #if DEBUG
    UI::Dialog::IO->println("[ [MAGENT]TesManager::processRQS[REGULAR]      ] Request inserted in Answer queue");
    #endif
  }
  #if DEBUG
  UI::Dialog::IO->println("Leaving thread processRQS. Bye!");
  #endif
}

void TesManager::processQID(){
  return;
  /*
  #if DEBUG
  //Bgin string
  UI::Dialog::IO->println("[ [WHITE]TesManager::processQID[REGULAR]      ] BEGIN");
  #endif
  while(!_exit){

    #if DEBUG
    UI::Dialog::IO->println("[ [WHITE]TesManager::processQID[REGULAR]      ] I'm waiting for requests to process");
    #endif

    //sem_wait(_questionariesSem);
    if(_exit) return;

    #if DEBUG
    UI::Dialog::IO->println("[ [WHITE]TesManager::processQID[REGULAR]      ] Client is waiting for answer");
    UI::Dialog::IO->println(
      std::string("[ [WHITE]TesManager::processQID[REGULAR]      ] Requests size: ") + \
      std::to_string(_awiRequests.size()));
    #endif


    #if DEBUG
    UI::Dialog::IO->println("[ [WHITE]TesManager::processQID[REGULAR]      ] Removing request from the AWI queue");
    #endif

    _questionariesMutex.lock();
    RequestTES r = _awiRequests.front();
    _questionariesMutex.unlock();
    //TODO

    //update last time stamp
    #if DEBUG
    UI::Dialog::IO->println("[ [WHITE]TesManager::processQID[REGULAR]      ] IQR message resent to ECP server");
    UI::Dialog::IO->println("[ [WHITE]TesManager::processQID[REGULAR]      ] Message:" + r.message());
    #endif


    _answerMutex.lock();
    _answers.push(r);
    sem_post(_answerSem);

    #if DEBUG
    UI::Dialog::IO->println("[ [MAGENT]TesManager::processRQS[REGULAR]      ] Request inserted in Answer queue");
    #endif

    _answerMutex.unlock();
  }*/
}

void TesManager::processAWI(){
  //sendIQR("78865","Q0156","The_topic_is_Real",45);
  //sendIQR("78865","Q0156","The_topic_is_Real",95);
  //TODO


  #if DEBUG
  UI::Dialog::IO->println("[ [CYAN]TesManager::processAWI[REGULAR]      ] BEGIN");
  #endif

  while(!_exit){

    #if DEBUG
    UI::Dialog::IO->println("[ [CYAN]TesManager::processAWI[REGULAR]      ] I'm waiting for requests to process");
    #endif

    sem_wait(_awiRequestsSem);
    if(_exit){
      #if DEBUG
      UI::Dialog::IO->println("Exiting the thread processTCP due to exit flag");
      #endif
      return;
    }

    #if DEBUG
    UI::Dialog::IO->println("[ [CYAN]TesManager::processAWI[REGULAR]      ] Client is waiting for answer");
    UI::Dialog::IO->println(
      std::string("[ [CYAN]TesManager::processAWI[REGULAR]      ] Requests size: ") + \
      std::to_string(_awiRequests.size()));
    #endif

    _awiMutex.lock();

    #if DEBUG
    UI::Dialog::IO->println("[ [CYAN]TesManager::processAWI[REGULAR]      ] Removing request from the AWI queue");
    #endif
    RequestTES r = _awiRequests.front();
    _awiRequests.pop();

    #if DEBUG
    UI::Dialog::IO->println("[ [CYAN]TesManager::processAWI[REGULAR]      ] Removed request from the AWI queue");
    UI::Dialog::IO->println("[ [CYAN]TesManager::processAWI[REGULAR]      ] Message:" + r.message());
    #endif

    _awiMutex.unlock();

    std::stringstream stream(r.message());
    std::string message;
    std::string qid;
    std::string trash;
    stream >> message;
    stream >> qid;
    stream >> trash;

    if(message == std::string("AWI") && qid != std::string("") &&
       qid.size() < 24 && qid.size() > 0 && trash == std::string("")){
      _answerUDPMutex.lock();
      if(_answersUDP.count(qid) != 0)
        _answersUDP.erase(_answersUDP.find(qid));
      _answerUDPMutex.unlock();
    }else if(message == std::string("ERR") || message != std::string("AWI") || trash != std::string("")){
      UI::Dialog::IO->println("[RED][ERR][REGULAR] There was an error in the communication with the server.");
      UI::Dialog::IO->println("Try again.");
    }
  }
  #if DEBUG
  UI::Dialog::IO->println("Leaving thread processAWI. Bye!");
  #endif
}

void TesManager::answerUDP(){

  #if DEBUG
  UI::Dialog::IO->println("[ [BLUE]TesManager::answerUDP[REGULAR]       ] BEGIN");
  #endif

  while(!_exit){

    #if DEBUG
    UI::Dialog::IO->println("[ [BLUE]TesManager::answerUDP[REGULAR]       ] I'm waiting for requests to process");
    #endif

    sem_wait(_answerUDPSem);
    if(_exit){
      #if DEBUG
      UI::Dialog::IO->println("Exiting the thread answerUDP due to exit flag");
      #endif
      return;
    }

    #if DEBUG
    UI::Dialog::IO->println(
      std::string("[ [BLUE]TesManager::answerUDP[REGULAR]       ] Requests size: ") + \
      std::to_string(_answersUDP.size()));
    UI::Dialog::IO->println("[ [BLUE]TesManager::answerUDP[REGULAR]       ] Removing request from the ANSWER queue");
    #endif

    _answerUDPMutex.lock();
    RequestTES r = _answersUDP.begin()->second;
    _answersUDP.erase(_answersUDP.begin()); //Removing from the unordered_map
    _answerUDPMutex.unlock();

    #if DEBUG
    UI::Dialog::IO->println("[ [BLUE]TesManager::answerUDP[REGULAR]       ] Removed request from the ANSWER queue");
    if(r.answer().size() < 100) UI::Dialog::IO->println(r.answer());
    #endif

    _senderSocketUDP->send(r.answer());


    _answerUDPMutex.lock();
    _answersUDP.insert(std::pair<std::string, RequestTES>(r.qid(), r));
    _answerUDPMutex.unlock();
    usleep(500);
  }
  #if DEBUG
  UI::Dialog::IO->println("Leaving thread answerUDP. Bye!");
  #endif
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
    if(_exit){
      #if DEBUG
      UI::Dialog::IO->println("Exiting the thread answerTCP due to exit flag");
      #endif
      return;
    }


    #if DEBUG
    UI::Dialog::IO->println("[ [BLUE]TesManager::answerTCP[REGULAR]       ] Client is waiting for answer");
    #endif

    #if DEBUG
    UI::Dialog::IO->println(
      std::string("[ [BLUE]TesManager::answerTCP[REGULAR]       ] Requests size: ") + \
      std::to_string(_rqtRequests.size()));
    UI::Dialog::IO->println("[ [BLUE]TesManager::answerTCP[REGULAR]       ] Removing request from the ANSWER queue");
    #endif

    _answerMutex.lock();
    RequestTES r = _answers.front();
    _answers.pop();
    _answerMutex.unlock();

    #if DEBUG
    UI::Dialog::IO->println("[ [BLUE]TesManager::answerTCP[REGULAR]       ] Removed request from the ANSWER queue");
    if(r.answer().size() < 100) UI::Dialog::IO->println(r.answer());
    #endif

    r.write();

    r.disconnect();
  }
  #if DEBUG
  UI::Dialog::IO->println("Leaving thread answerTCP. Bye!");
  #endif
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

  #if DEBUG
  UI::Dialog::IO->println(            "[ TesManager::sendIQR             ] ECP server data:");
  UI::Dialog::IO->println(std::string("[ TesManager::sendIQR             ] Server Name: ").append(_ecpname.data()));
  UI::Dialog::IO->println(std::string("[ TesManager::sendIQR             ] Server Port: ").append(std::to_string(_ecpport)));
  #endif

  SocketUDP ecp = SocketUDP(_ecpname.data(), _ecpport );
  std::string message;
  std::string QID_Received;

  #if DEBUG
  UI::Dialog::IO->println("[ TesManager::sendIQR             ] Socket created");
  UI::Dialog::IO->println("[ TesManager::sendIQR             ] Sending message");
  #endif

  std::stringstream stream;
  for(auto i = 0; i < __TRIES__; i++){
    ecp.send(std::string("IQR ") + std::string(SID) + std::string(" ") + std::string(QID) + std::string(" ") + std::string(topic_name) + std::string(" ") + std::to_string(scr) + std::string("\n"));
    try{
      ecp.timeout(__MS_BETWEEN_TRIES__);
      stream << ecp.receive();

      #if DEBUG
      UI::Dialog::IO->println("[ TesManager::sendIQR             ] Message sent to ECP");
      UI::Dialog::IO->println("[ TesManager::sendIQR             ] Receiving message from ECP");
      UI::Dialog::IO->println("[ TesManager::sendIQR             ] Message received from ECP");
      #endif

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
