#include "ECPManager.h"

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
  socketUDP = SocketUDP(_port);
  if(__DEBUG__) UI::Dialog::IO->println("[ ECPManager::acceptRequests  ] Socket created");
  while(!_exit){
    try{
      if(__DEBUG__) UI::Dialog::IO->println("[ ECPManager::acceptRequests  ] Waiting for messages");

      _UDPMutex.lock();
      std::string message = socketUDP.receive();

      UI::Dialog::IO->print("[ ECPManager::acceptRequests  ] Request received from");
      UI::Dialog::IO->println(socketUDP.ip());

      struct sockaddr_in client = socketUDP.client();
      _UDPMutex.unlock();

      UI::Dialog::IO->println(
        std::string("[ ECPManager::acceptRequests  ] Message:") +\
        message);

      UI::Dialog::IO->println(
        std::string("[ ECPManager::acceptRequests  ] Size of Message:") +\
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

  //TODO
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
    RequestECP r = _tqrRequests.front();
    _tqrRequests.pop();
    _tqrMutex.unlock();         //Lock the queue to remove a request

    std::cout << r.read() << std::endl;
    std::pair <std::string,int> topicsList = topics();
    std::string answer = "AQT ";
    answer += std::to_string(topicsList.second);
    answer += std::string(" ");
    answer += topicsList.first;
    answer += std::string("\n");
    r.answer(answer);

    _answerMutex.lock();
    _answers.push(r);

    sem_post(_answerSemaphore); //So answers thread can know that there is an
                                //answer to send

    _answerMutex.unlock();
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
      RequestECP r = _tqrRequests.front();
      _terRequests.pop();
      _terMutex.unlock();         //Lock the queue to remove a request

      //TODO

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
  if(__DEBUG__) UI::Dialog::IO->println("[ ECPManager::sendAnswer      ] Begin");
  while(!_exit){
    if(__DEBUG__) UI::Dialog::IO->println("[ ECPManager::sendAnswer      ] I'm waiting for requests to process");
    sem_wait(_answerSemaphore);

    if(__DEBUG__) UI::Dialog::IO->println(
                    std::string("[ ECPManager::sendAnswer      ] Requests size: ") + \
                    std::to_string(_answers.size()));

    _answerMutex.lock();           //Lock the queue to remove a request
                                //MultiThreading requires this mutex
    RequestECP r = _answers.front();

    _answers.pop();
    _answerMutex.unlock();         //Lock the queue to remove a request

    if(__DEBUG__) UI::Dialog::IO->println(
      std::string("[ ECPManager::sendAnswer      ] Sending message: ") + \
      r.answer());

    _UDPMutex.lock();

    socketUDP.client(r.client());
    socketUDP.send(r.answer());
    if(__DEBUG__) UI::Dialog::IO->println(
                    std::string("[ ECPManager::sendAnswer      ] Answers left: ") + \
                    std::to_string(_answers.size()));
    _UDPMutex.unlock();

  }
}

std::pair <std::string,int> ECPManager::topics(){
  std::string topics;
  std::string topic;
  std::ifstream iFile(_topicsFile);
  int i = 0;
  while(iFile >> topic){
    i++;
    topics += topic + std::string(" ");
  }

  return std::make_pair(topics,i);;
}
