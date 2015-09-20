#include "ECPManager.h"

#include <sstream>
#include <fstream>      // std::ifstream
#include "Dialog.h"

ECPManager::ECPManager(int port) : _tqrSemaphore(new sem_t()),
_terSemaphore(new sem_t()), _iqrSemaphore(new sem_t()),
_answerSemaphore(new sem_t()), _port(port),
_exit(false), _maxAcceptingThreads(3), _maxProcessTQRThreads(1),
_maxProcessTERThreads(1), _maxProcessIQRThreads(1), _maxSendAnswerThreads(5),
_topicsFile("topics.txt")
{

  sem_init(_tqrSemaphore, 0, 0);
  sem_init(_terSemaphore, 0, 0);
  sem_init(_iqrSemaphore, 0, 0);
  sem_init(_answerSemaphore, 0, 0);
}
ECPManager::~ECPManager(){
  sem_destroy(_tqrSemaphore);
  sem_destroy(_terSemaphore);
  sem_destroy(_iqrSemaphore);
  sem_destroy(_answerSemaphore);
}

void ECPManager::acceptRequests(){
  if(__DEBUG__) UI::Dialog::IO->println("[ ECPManager::acceptRequests  ] Creating socket");
  _socketUDP = SocketUDP(_port);
  _senderSocketUDP = SocketUDP(_port);
  if(__DEBUG__) UI::Dialog::IO->println("[ ECPManager::acceptRequests  ] Socket created");
  while(!_exit){
    try{
      if(__DEBUG__) UI::Dialog::IO->println("[ ECPManager::acceptRequests  ] Waiting for messages");

      _UDPMutex.lock();
      std::string message = _socketUDP.receive();



      struct sockaddr_in client = _socketUDP.client();
      _UDPMutex.unlock();

      UI::Dialog::IO->println(message + std::string(" from ") + _socketUDP.ip() +\
        std::string(" on port ") + _socketUDP.port());

      if(__DEBUG__) UI::Dialog::IO->println(
        std::string("[ ECPManager::acceptRequests  ] Size of Message: ") +\
        std::to_string(message.size()));
      if(message == std::string("TQR")){
        RequestECP request(message, client);
        _tqrMutex.lock();           //Lock the queue to insert a request
        _tqrRequests.push(request);
        _tqrMutex.unlock();         //Unlock the queue so other threads can use it
        if(__DEBUG__) UI::Dialog::IO->println("[ ECPManager::acceptRequests  ] Task inserted in TQR queue");
        sem_post(_tqrSemaphore);    //Post semaphore so a thread is called
      }else if(message.substr(0,3) == std::string("TER")){
        RequestECP request(message, client);
        _terMutex.lock();           //Lock the queue to insert a request
        _terRequests.push(request);
        _terMutex.unlock();         //Unlock the queue so other threads can use it
        if(__DEBUG__) UI::Dialog::IO->println("[ ECPManager::acceptRequests  ] Task inserted in TER queue");
        sem_post(_terSemaphore);    //Post semaphore so a thread is called
      }else if(message.substr(0,3) == std::string("IQR")){
        RequestECP request(message, client);
        _iqrMutex.lock();           //Lock the queue to insert a request
        _iqrRequests.push(request);
        _iqrMutex.unlock();         //Unlock the queue so other threads can use it
        if(__DEBUG__) UI::Dialog::IO->println("[ ECPManager::acceptRequests  ] Task inserted in IQR queue");
        sem_post(_iqrSemaphore);    //Post semaphore so a thread is called
      }else{
        RequestECP request(message, client);
        request.answer("ERR\n");
        _answerMutex.lock();        //Lock the queue to insert a request
        _answers.push(request);
        _answerMutex.unlock();      //Unlock the queue so other threads can use it
        if(__DEBUG__) UI::Dialog::IO->println("[ ECPManager::acceptRequests  ] Task inserted in Answer queue");
        sem_post(_answerSemaphore); //Post semaphore so a thread is called
      }
    }catch(std::string s){
      UI::Dialog::IO->println(std::string("[ ECPManager::acceptRequests  ] [RED][ERROR][REGULAR]") + s);
      _UDPMutex.unlock();
    }
  }
}

void ECPManager::processTQR(){
  if(__DEBUG__) UI::Dialog::IO->println("[ ECPManager::processTQR      ] Begin");
  while(!_exit){
    if(__DEBUG__) UI::Dialog::IO->println("[ ECPManager::processTQR      ] I'm waiting for requests to process");
    sem_wait(_tqrSemaphore);
    if(__DEBUG__) UI::Dialog::IO->println(
                    std::string("[ ECPManager::processTQR      ] Requests size: ") + \
                    std::to_string(_tqrRequests.size()));

    _tqrMutex.lock();           //Lock the queue to remove a request
                               //MultiThreading requires this mutex
    if(__DEBUG__) UI::Dialog::IO->println("[ ECPManager::processTQR      ] Getting TQR Request from the queue");
    RequestECP r = _tqrRequests.front();
    _tqrRequests.pop();
    if(__DEBUG__) UI::Dialog::IO->println("[ ECPManager::processTQR      ] TQR Deleted from the queue");
    _tqrMutex.unlock();         //Lock the queue to remove a request
    if(__DEBUG__) UI::Dialog::IO->println(std::string("Request: ").append(r.read()));
    std::pair <std::string,int> topicsList;
    std::string answer;
    try{
      topicsList = topics();
      answer = "AQT ";
      answer += std::to_string(topicsList.second);
      answer += std::string(" ");
      answer += topicsList.first;
      answer += std::string("\n");
    }catch(std::string s){
      if(s == std::string("EOF")){
        answer = s;
      }
    }
    r.answer(answer);
    _answerMutex.lock();
    if(__DEBUG__) UI::Dialog::IO->println("[ ECPManager::processTQR      ] Inserting Request on Answer Queue");
    _answers.push(r);
    if(__DEBUG__) UI::Dialog::IO->println("[ ECPManager::processTQR      ] Inserted Request on Answer Queue");
    _answerMutex.unlock();
    if(__DEBUG__) UI::Dialog::IO->println("[ ECPManager::processTQR      ] Posting Answer Semaphore");
    sem_post(_answerSemaphore); //So answers thread can know that there is an
                                //answer to send
    if(__DEBUG__) UI::Dialog::IO->println("[ ECPManager::processTQR      ] Answer Semaphore Post");
  }
}


void ECPManager::processTER(){
  if(__DEBUG__) UI::Dialog::IO->println("[ ECPManager::processTER      ] Begin");
  while(!_exit){
    if(__DEBUG__) UI::Dialog::IO->println("[ ECPManager::processTER      ] I'm waiting for requests to process");
    sem_wait(_terSemaphore);
    if(__DEBUG__) UI::Dialog::IO->println(
                    std::string("[ ECPManager::processTER      ] Requests size: ") + \
                    std::to_string(_terRequests.size()));

    _terMutex.lock();           //Lock the queue to remove a request
                               //MultiThreading requires this mutex
    if(__DEBUG__) UI::Dialog::IO->println("[ ECPManager::processTER      ] Getting TER Request from the queue");
    RequestECP r = _terRequests.front();
    _terRequests.pop();
    if(__DEBUG__) UI::Dialog::IO->println("[ ECPManager::processTER      ] TER Deleted from the queue");
    _terMutex.unlock();         //Lock the queue to remove a request
    if(__DEBUG__) UI::Dialog::IO->println(std::string("Request: ").append(r.read()));
    std::string answer;
    std::stringstream stream(r.read());
    std::string code;
    std::string tIDstr;
    std::string trash;
    stream >> code;
    stream >> tIDstr;
    stream >> trash;
    int tID;
    bool is_number = true;
    for(int index = 0; index < (int) tIDstr.size(); index++)
      if(tIDstr[index] < '0' || tIDstr[index] > '9') is_number = false;

    if(tIDstr.size() == 0 || !is_number || trash != std::string("")){
      r.answer("ERR");
    }else{
      tID = atoi(tIDstr.data());
      std::pair <std::string, int> data = topicData(tID);
      answer = std::string("AWTES ") + data.first + std::string(" ") + \
       std::to_string(data.second);
    }
    r.answer(answer);
    _answerMutex.lock();
    if(__DEBUG__) UI::Dialog::IO->println("[ ECPManager::processTER      ] Inserting Request on Answer Queue");
    _answers.push(r);
    if(__DEBUG__) UI::Dialog::IO->println("[ ECPManager::processTER      ] Inserted Request on Answer Queue");
    _answerMutex.unlock();
    if(__DEBUG__) UI::Dialog::IO->println("[ ECPManager::processTER      ] Posting Answer Semaphore");
    sem_post(_answerSemaphore); //So answers thread can know that there is an
                                //answer to send
    if(__DEBUG__) UI::Dialog::IO->println("[ ECPManager::processTER      ] Answer Semaphore Post");
  }
}

void ECPManager::processIQR(){
  if(__DEBUG__) UI::Dialog::IO->println("[ ECPManager::processIQR      ] Begin");
  while(!_exit){
    if(__DEBUG__) UI::Dialog::IO->println("[ ECPManager::processIQR      ] I'm waiting for requests to process");
    sem_wait(_iqrSemaphore);

    if(__DEBUG__) UI::Dialog::IO->println(
                    std::string("[ ECPManager::processIQR      ] Requests size: ") + \
                    std::to_string(_iqrRequests.size()));

    _iqrMutex.lock();           //Lock the queue to remove a request
                                //MultiThreading requires this mutex
    RequestECP r = _iqrRequests.front();
    _iqrRequests.pop();
    _iqrMutex.unlock();         //Lock the queue to remove a request

    //TODO

  }
}

void ECPManager::sendAnswer(){
  if(__DEBUG__) UI::Dialog::IO->println("[ [BLUE]ECPManager::sendAnswer[REGULAR]      ] Begin");
  while(!_exit){
    if(__DEBUG__) UI::Dialog::IO->println("[ [BLUE]ECPManager::sendAnswer[REGULAR]      ] I'm waiting for requests to process");
    sem_wait(_answerSemaphore);

    if(__DEBUG__) UI::Dialog::IO->println(
                    std::string("[ [BLUE]ECPManager::sendAnswer[REGULAR]      ] Requests size: ") + \
                    std::to_string(_answers.size()));

    _answerMutex.lock();           //Lock the queue to remove a request
                                //MultiThreading requires this mutex
    RequestECP r = _answers.front();

    _answers.pop();
    _answerMutex.unlock();         //Lock the queue to remove a request

    if(__DEBUG__) UI::Dialog::IO->println(
      std::string("[ [BLUE]ECPManager::sendAnswer[REGULAR]      ] Sending message: ") + \
      r.answer());

    if(__DEBUG__) UI::Dialog::IO->println("[ [BLUE]ECPManager::sendAnswer[REGULAR]      ] Waiting for UDP Socket to be free");
    _UDPSenderMutex.lock();
    if(__DEBUG__) UI::Dialog::IO->println("[ [BLUE]ECPManager::sendAnswer[REGULAR]      ] UDP socket is free, will use it now");
    _socketUDP.client(r.client());
    if(__DEBUG__) UI::Dialog::IO->println(
                    std::string("[ [BLUE]ECPManager::sendAnswer[REGULAR]      ] Sending message: ") + \
                    r.answer());
    _socketUDP.send(r.answer());

    if(__DEBUG__) UI::Dialog::IO->println("[ [BLUE]ECPManager::sendAnswer[REGULAR]      ] Message sent by UDP Socket");

    if(__DEBUG__) UI::Dialog::IO->println(
                    std::string("[ [BLUE]ECPManager::sendAnswer[REGULAR]      ] Answers left: ") + \
                    std::to_string(_answers.size()));
    _UDPSenderMutex.unlock();

  }
}
std::pair <std::string, int> ECPManager::topicData(int index){
  std::string topic;
  std::string hostname("");
  int port = 0;
  std::ifstream iFile(_topicsFile);
  int i = 1;
  while((iFile >> topic)){
    if(index == i){
      iFile >> hostname;
      iFile >> port;
      break;
    }
    i++;
    iFile >> topic; //Read hostname
    iFile >> topic; //Read port
  }
  if(port == 0) throw std::string("EOF");
  return std::make_pair(hostname,port);;
}
std::pair <std::string,int> ECPManager::topics(){
  std::string topics;
  std::string topic;
  std::ifstream iFile(_topicsFile);
  int i = 0;
  while(iFile >> topic){
    i++;
    topics += topic.append(" ");
    iFile >> topic; iFile >> topic;
  }
  if(i == 0) throw std::string("EOF");
  return std::make_pair(topics,i);;
}
