#include "ECPManager.h"

#include <sstream>
#include <fstream>      // std::ifstream
#include <iostream>
#include "Dialog.h"

ECPManager::ECPManager(int port) : _tqrSemaphore(new sem_t()),
	_terSemaphore(new sem_t()), _iqrSemaphore(new sem_t()),
	_answerSemaphore(new sem_t()), _port(port),
	_exit(false), _maxAcceptingThreads(3), _maxProcessTQRThreads(1),
	_maxProcessTERThreads(1), _maxProcessIQRThreads(1), _maxSendAnswerThreads(5),
	_topicsFile("topics.txt"),_statsFile("stats.txt")
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

  #if DEBUG
	UI::Dialog::IO->println("[ [GREEN]ECPManager::acceptRequests[REGULAR]  ] Creating socket");
	#endif

	try{
  _socketUDP = SocketUDP(_port);
  _senderSocketUDP = SocketUDP(_port);
	}catch(std::string s){
		UI::Dialog::IO->println(s);
		return;
	}

  #if DEBUG
	UI::Dialog::IO->println("[ [GREEN]ECPManager::acceptRequests[REGULAR]  ] Socket created");
	#endif

  while(!_exit){
    try{

      #if DEBUG
			UI::Dialog::IO->println("[ [GREEN]ECPManager::acceptRequests[REGULAR]  ] Waiting for messages");
			#endif

      _UDPMutex.lock();
      std::string message = _socketUDP.receive();


      struct sockaddr_in client = _socketUDP.client();
      _UDPMutex.unlock();

      UI::Dialog::IO->println(message + std::string(" from ") + _socketUDP.ip() +\
        std::string(" on port ") + _socketUDP.port());

      #if DEBUG
			UI::Dialog::IO->println(
        std::string("[ [GREEN]ECPManager::acceptRequests[REGULAR]  ] Size of Message: ") +\
        std::to_string(message.size()));
			#endif

      if(message == std::string("TQR")){
        RequestECP request(message, client);
        _tqrMutex.lock();           //Lock the queue to insert a request
        _tqrRequests.push(request);
        _tqrMutex.unlock();         //Unlock the queue so other threads can use it

        #if DEBUG
				UI::Dialog::IO->println("[ [GREEN]ECPManager::acceptRequests[REGULAR]  ] Task inserted in TQR queue");
				#endif

        sem_post(_tqrSemaphore);    //Post semaphore so a thread is called
      }

	  else if(message.substr(0,3) == std::string("TER")){
        RequestECP request(message, client);
        _terMutex.lock();           //Lock the queue to insert a request
        _terRequests.push(request);
        _terMutex.unlock();         //Unlock the queue so other threads can use it

        #if DEBUG
				UI::Dialog::IO->println("[ [GREEN]ECPManager::acceptRequests[REGULAR]  ] Task inserted in TER queue");
				#endif

        sem_post(_terSemaphore);    //Post semaphore so a thread is called
      }

	  else if(message.substr(0,3) == std::string("IQR")){
        RequestECP request(message, client);
        _iqrMutex.lock();           //Lock the queue to insert a request
        _iqrRequests.push(request);
        _iqrMutex.unlock();         //Unlock the queue so other threads can use it

        #if DEBUG
				UI::Dialog::IO->println("[ [GREEN]ECPManager::acceptRequests[REGULAR]  ] Task inserted in IQR queue");
				#endif

        sem_post(_iqrSemaphore);    //Post semaphore so a thread is called
      }

	  else{
        RequestECP request(message, client);
        request.answer("ERR\n");
        _answerMutex.lock();        //Lock the queue to insert a request
        _answers.push(request);
        _answerMutex.unlock();      //Unlock the queue so other threads can use it

        #if DEBUG
				UI::Dialog::IO->println("[ [GREEN]ECPManager::acceptRequests[REGULAR]  ] Task inserted in Answer queue");
				#endif

        sem_post(_answerSemaphore); //Post semaphore so a thread is called
      }
    }catch(std::string s){
      UI::Dialog::IO->println(std::string("[ [GREEN]ECPManager::acceptRequests[REGULAR]  ] [RED][ERROR][REGULAR]") + s);
      _UDPMutex.unlock();
    }
  }
}

void ECPManager::processTQR(){

  #if DEBUG
	UI::Dialog::IO->println("[ [MAGENT]ECPManager::processTQR[REGULAR]      ] Begin");
	#endif

  while(!_exit){

    #if DEBUG
		UI::Dialog::IO->println("[ [MAGENT]ECPManager::processTQR[REGULAR]      ] I'm waiting for requests to process");
		#endif

    sem_wait(_tqrSemaphore);

	#if DEBUG
	UI::Dialog::IO->println(
	  std::string("[ [MAGENT]ECPManager::processTQR[REGULAR]      ] Requests size: ") + \
	  std::to_string(_tqrRequests.size()));
	#endif
    _tqrMutex.lock();           //Lock the queue to remove a request
                               //MultiThreading requires this mutex
    #if DEBUG
UI::Dialog::IO->println("[ [MAGENT]ECPManager::processTQR[REGULAR]      ] Getting TQR Request from the queue");
#endif
    RequestECP r = _tqrRequests.front();
    _tqrRequests.pop();

		#if DEBUG
		UI::Dialog::IO->println("[ [MAGENT]ECPManager::processTQR[REGULAR]      ] TQR Deleted from the queue");
		#endif

    _tqrMutex.unlock();         //Lock the queue to remove a request

		#if DEBUG
		UI::Dialog::IO->println(std::string("Request: ").append(r.read()));
		#endif

    std::pair <std::string,int> topicsList;
    std::string answer;


	// Request beeing handled
    try{
      topicsList = topics();
      answer = "AWT ";
      answer += std::to_string(topicsList.second);
      answer += std::string(" ");
      answer += topicsList.first;
      answer += std::string("\n");
    }catch(std::string s){
      if(s == std::string("EOF")){
        answer = s;
      }
    }
	// End of Handle

    r.answer(answer);
    _answerMutex.lock();

		#if DEBUG
		UI::Dialog::IO->println("[ [MAGENT]ECPManager::processTQR[REGULAR]      ] Inserting Request on Answer Queue");
		#endif

    _answers.push(r);

		#if DEBUG
		UI::Dialog::IO->println("[ [MAGENT]ECPManager::processTQR[REGULAR]      ] Inserted Request on Answer Queue");
		#endif

    _answerMutex.unlock();

		#if DEBUG
		UI::Dialog::IO->println("[ [MAGENT]ECPManager::processTQR[REGULAR]      ] Posting Answer Semaphore");
		#endif

    sem_post(_answerSemaphore); //So answers thread can know that there is an
                                //answer to send
		#if DEBUG
		UI::Dialog::IO->println("[ [MAGENT]ECPManager::processTQR[REGULAR]      ] Answer Semaphore Post");
		#endif

  }
}


void ECPManager::processTER(){

	#if DEBUG
	UI::Dialog::IO->println("[ [YELLOW]ECPManager::processTER[REGULAR]      ] Begin");
	#endif

  while(!_exit){

	  #if DEBUG
		UI::Dialog::IO->println("[ [YELLOW]ECPManager::processTER[REGULAR]      ] I'm waiting for requests to process");
		#endif

    sem_wait(_terSemaphore);

		#if DEBUG
		UI::Dialog::IO->println(
      std::string("[ [YELLOW]ECPManager::processTER[REGULAR]      ] Requests size: ") + \
      std::to_string(_terRequests.size()));
		#endif

    _terMutex.lock();           //Lock the queue to remove a request
                               //MultiThreading requires this mutex

	  #if DEBUG
		UI::Dialog::IO->println("[ [YELLOW]ECPManager::processTER[REGULAR]      ] Getting TER Request from the queue");
		#endif

    RequestECP r = _terRequests.front();
    _terRequests.pop();

	  #if DEBUG
		UI::Dialog::IO->println("[ [YELLOW]ECPManager::processTER[REGULAR]      ] TER Deleted from the queue");
		#endif

    _terMutex.unlock();         //Lock the queue to remove a request

	  #if DEBUG
		UI::Dialog::IO->println(std::string("Request: ").append(r.read()));
		#endif

    std::string answer;

	// Request beeing handled
    std::stringstream stream(r.read()); //TODO??
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

			#if DEBUG
			UI::Dialog::IO->print("[ [YELLOW]ECPManager::processTER[REGULAR]      ] It's not a number. Size of input: ");
			UI::Dialog::IO->print(std::to_string(tIDstr.size()));
			UI::Dialog::IO->print(" | input: ");
			UI::Dialog::IO->println(tIDstr);
			#endif

      answer = "ERR";
    }else {

			#if DEBUG
			UI::Dialog::IO->print("[ [YELLOW]ECPManager::processTER[REGULAR]      ] It's a topic number. Size of input: ");
			UI::Dialog::IO->print(std::to_string(tIDstr.size()));
			UI::Dialog::IO->print(" | input: ");
			UI::Dialog::IO->println(tIDstr);
			#endif

      tID = atoi(tIDstr.data());
      std::pair <std::string, int> data = topicData(tID);
      answer = std::string("AWTES ") + data.first + std::string(" ") + \
       std::to_string(data.second);
    }
	// End of Handle

    r.answer(answer);
    _answerMutex.lock();

		#if DEBUG
		UI::Dialog::IO->println("[ [YELLOW]ECPManager::processTER[REGULAR]      ] Inserting Request on Answer Queue");
		#endif

    _answers.push(r);

		#if DEBUG
		UI::Dialog::IO->println("[ [YELLOW]ECPManager::processTER[REGULAR]      ] Inserted Request on Answer Queue");
		#endif

    _answerMutex.unlock();

		#if DEBUG
		UI::Dialog::IO->println("[ [YELLOW]ECPManager::processTER[REGULAR]      ] Posting Answer Semaphore");
		#endif

    sem_post(_answerSemaphore); //So answers thread can know that there is an
                                //answer to send

		#if DEBUG
		UI::Dialog::IO->println("[ [YELLOW]ECPManager::processTER[REGULAR]      ] Answer Semaphore Post");
		#endif

  }
}

void ECPManager::processIQR(){

   #if DEBUG
		UI::Dialog::IO->println("[ [CYAN]ECPManager::processIQR[REGULAR]      ] Begin");
		#endif

  while(!_exit){

    #if DEBUG
		UI::Dialog::IO->println("[ [CYAN]ECPManager::processIQR[REGULAR]      ] I'm waiting for requests to process");
		#endif

    sem_wait(_iqrSemaphore);

    #if DEBUG
		UI::Dialog::IO->println(
	    std::string("[ [CYAN]ECPManager::processIQR[REGULAR]      ] Requests size: ") + \
	    std::to_string(_iqrRequests.size()));
		#endif

    _iqrMutex.lock();           //Lock the queue to remove a request
                                //MultiThreading requires this mutex
    #if DEBUG
		UI::Dialog::IO->println("[ [CYAN]ECPManager::processIQR[REGULAR]      ] Getting IQR Request from the queue");
		#endif

    RequestECP r = _iqrRequests.front();
    _iqrRequests.pop();

    #if DEBUG
		UI::Dialog::IO->println("[ [CYAN]ECPManager::processIQR[REGULAR]      ] IQR Deleted from the queue");
		#endif
    _iqrMutex.unlock();         //Lock the queue to remove a request

    #if DEBUG
		UI::Dialog::IO->println(std::string("Request: ").append(r.read()));
		#endif


    std::string answer;




	// Request beeing handled
	//TODO
	// create score
	//
	// Check for repeated request
	//
	// Add counter to the topic
	// Update counter on stats.txt
  	std::ofstream iFile;
	iFile.open(_statsFile,std::fstream::out);
	if(iFile.fail()){
		iFile.open(_statsFile, std::fstream::out);
	}

	/*
	iFile.open (_statsFile, std::fstream::app);*/
	iFile << "Writing this to a file.\n";
	iFile.close();
	//
	// End of Handle

    r.answer(answer);
    _answerMutex.lock();

		#if DEBUG
		UI::Dialog::IO->println("[ [CYAN]ECPManager::processIQR[REGULAR]      ] Inserting Request on Answer Queue");
		#endif

    _answers.push(r);

		#if DEBUG
		UI::Dialog::IO->println("[ [CYAN]ECPManager::processIQR[REGULAR]      ] Inserted Request on Answer Queue");
		#endif

    _answerMutex.unlock();

		#if DEBUG
		UI::Dialog::IO->println("[ [CYAN]ECPManager::processIQR[REGULAR]      ] Posting Answer Semaphore");
		#endif

    sem_post(_answerSemaphore); //So answers thread can know that there is an
                                //answer to send

		#if DEBUG
		UI::Dialog::IO->println("[ [CYAN]ECPManager::processIQR[REGULAR]      ] Answer Semaphore Post");
		#endif


  }
}

void ECPManager::sendAnswer(){

  #if DEBUG
	UI::Dialog::IO->println("[ [BLUE]ECPManager::sendAnswer[REGULAR]      ] Begin");
	#endif

  while(!_exit){

    #if DEBUG
		UI::Dialog::IO->println("[ [BLUE]ECPManager::sendAnswer[REGULAR]      ] I'm waiting for requests to process");
		#endif

    sem_wait(_answerSemaphore);

    #if DEBUG
		UI::Dialog::IO->println(
		  std::string("[ [BLUE]ECPManager::sendAnswer[REGULAR]      ] Requests size: ") + \
		  std::to_string(_answers.size()));
		#endif

    _answerMutex.lock();          //Lock the queue to remove a request
                                 //MultiThreading requires this mutex
    RequestECP r = _answers.front();

    _answers.pop();
    _answerMutex.unlock();         //Lock the queue to remove a request

    #if DEBUG
		UI::Dialog::IO->println(
      std::string("[ [BLUE]ECPManager::sendAnswer[REGULAR]      ] Sending message: ") + \
      r.answer());
		#endif

    #if DEBUG
		UI::Dialog::IO->println("[ [BLUE]ECPManager::sendAnswer[REGULAR]      ] Waiting for UDP Socket to be free");
		#endif

    _UDPSenderMutex.lock();

    #if DEBUG
		UI::Dialog::IO->println("[ [BLUE]ECPManager::sendAnswer[REGULAR]      ] UDP socket is free, will use it now");
		#endif

    _socketUDP.client(r.client());

    #if DEBUG
		UI::Dialog::IO->println(
	    std::string("[ [BLUE]ECPManager::sendAnswer[REGULAR]      ] Sending message: ") + \
	    r.answer());
		#endif

    _socketUDP.send(r.answer());

    #if DEBUG
		UI::Dialog::IO->println("[ [BLUE]ECPManager::sendAnswer[REGULAR]      ] Message sent by UDP Socket");
		UI::Dialog::IO->println(
      std::string("[ [BLUE]ECPManager::sendAnswer[REGULAR]      ] Answers left: ") + \
      std::to_string(_answers.size()));
		#endif

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
  return std::make_pair(topics.substr(0,topics.size()-1),i);
}
