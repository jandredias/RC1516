#include "ECPManager.h"
#include "Debug.h"

#include <sstream>
#include <fstream>      // std::ifstream
#include <iostream>
#include "Exception.h"
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

  debug("[ [GREEN]ECPManager::acceptRequests[REGULAR]  ] Creating socket");

	try{
  _socketUDP = SocketUDP(_port);
  //_senderSocketUDP = SocketUDP(_port);
	}catch(SocketAlreadyInUse s){
		UI::Dialog::IO->println(s.message());
		exit(0);
	}catch(std::string s){
		UI::Dialog::IO->println(s);
		return;
	}

  debug("[ [GREEN]ECPManager::acceptRequests[REGULAR]  ] Socket created");


  while(!_exit){
	  std::string message;
	  struct sockaddr_in client;
    try{

      debug("[ [GREEN]ECPManager::acceptRequests[REGULAR]  ] Waiting for messages");

      _UDPMutex.lock();
      message = _socketUDP.receive();


      client = _socketUDP.client();
      _UDPMutex.unlock();

      UI::Dialog::IO->println(message + std::string(" from ") + _socketUDP.ip() +\
        std::string(" on port ") + _socketUDP.port());

      debug(
        std::string("[ [GREEN]ECPManager::acceptRequests[REGULAR]  ] Size of Message: ") +\
        std::to_string(message.size()));

      if(message == std::string("TQR")){
        RequestECP request(message, client);
        _tqrMutex.lock();           //Lock the queue to insert a request
        _tqrRequests.push(request);
        _tqrMutex.unlock();         //Unlock the queue so other threads can use it

        debug("[ [GREEN]ECPManager::acceptRequests[REGULAR]  ] Task inserted in TQR queue");

        sem_post(_tqrSemaphore);    //Post semaphore so a thread is called
      }

	  else if(message.substr(0,3) == std::string("TER")){
        RequestECP request(message, client);
        _terMutex.lock();           //Lock the queue to insert a request
        _terRequests.push(request);
        _terMutex.unlock();         //Unlock the queue so other threads can use it

        debug("[ [GREEN]ECPManager::acceptRequests[REGULAR]  ] Task inserted in TER queue");

        sem_post(_terSemaphore);    //Post semaphore so a thread is called
      }

	  else if(message.substr(0,3) == std::string("IQR")){
        RequestECP request(message, client);
        _iqrMutex.lock();           //Lock the queue to insert a request
        _iqrRequests.push(request);
        _iqrMutex.unlock();         //Unlock the queue so other threads can use it

        debug("[ [GREEN]ECPManager::acceptRequests[REGULAR]  ] Task inserted in IQR queue");

        sem_post(_iqrSemaphore);    //Post semaphore so a thread is called
      }

	  else{
        RequestECP request(message, client);
        request.answer("ERR\n");
        _answerMutex.lock();        //Lock the queue to insert a request
        _answers.push(request);
        _answerMutex.unlock();      //Unlock the queue so other threads can use it

        debug("[ [GREEN]ECPManager::acceptRequests[REGULAR]  ] Task inserted in Answer queue");

        sem_post(_answerSemaphore); //Post semaphore so a thread is called
      }
    }catch(SocketAlreadyInUse s){
			UI::Dialog::IO->println(s.message());
	}catch(std::string s){
      UI::Dialog::IO->println(std::string("[ [GREEN]ECPManager::acceptRequests[REGULAR]  ] [RED][ERROR][REGULAR]") + s);
      _UDPMutex.unlock();
    }catch(UnknownFormatProtocol s){
	  client = _socketUDP.client();
      _UDPMutex.unlock();



      UI::Dialog::IO->println("ERR " + _socketUDP.ip() +\
        std::string(" on port ") + _socketUDP.port());

	  RequestECP request(message, client);
		request.answer("ERR\n");
		_answerMutex.lock();        //Lock the queue to insert a request
		_answers.push(request);
		_answerMutex.unlock();      //Unlock the queue so other threads can use it

		debug("[ [GREEN]ECPManager::acceptRequests[REGULAR]  ] Task inserted in Answer queue");

		sem_post(_answerSemaphore); //Post semaphore so a thread is called


	}
  }
}

void ECPManager::processTQR(){
  debug("[ [MAGENT]ECPManager::processTQR[REGULAR]      ] Begin");
  while(!_exit){
    debug("[ [MAGENT]ECPManager::processTQR[REGULAR]      ] I'm waiting for requests to process");
    sem_wait(_tqrSemaphore);

		debug(
	  std::string("[ [MAGENT]ECPManager::processTQR[REGULAR]      ] Requests size: ") + \
	  std::to_string(_tqrRequests.size()));
    _tqrMutex.lock();           //Lock the queue to remove a request
                               //MultiThreading requires this mutex
    debug("[ [MAGENT]ECPManager::processTQR[REGULAR]      ] Getting TQR Request from the queue");

    RequestECP r = _tqrRequests.front();
    _tqrRequests.pop();

		debug("[ [MAGENT]ECPManager::processTQR[REGULAR]      ] TQR Deleted from the queue");

    _tqrMutex.unlock();         //Lock the queue to remove a request

		debug(std::string("Request: ").append(r.read()));

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

		debug("[ [MAGENT]ECPManager::processTQR[REGULAR]      ] Inserting Request on Answer Queue");

    _answers.push(r);

		debug("[ [MAGENT]ECPManager::processTQR[REGULAR]      ] Inserted Request on Answer Queue");

    _answerMutex.unlock();

		debug("[ [MAGENT]ECPManager::processTQR[REGULAR]      ] Posting Answer Semaphore");

    sem_post(_answerSemaphore); //So answers thread can know that there is an
                                //answer to send
		debug("[ [MAGENT]ECPManager::processTQR[REGULAR]      ] Answer Semaphore Post");

  }
}


void ECPManager::processTER(){

	debug("[ [YELLOW]ECPManager::processTER[REGULAR]      ] Begin");

  while(!_exit){

	  debug("[ [YELLOW]ECPManager::processTER[REGULAR]      ] I'm waiting for requests to process");

    sem_wait(_terSemaphore);

		debug(
      std::string("[ [YELLOW]ECPManager::processTER[REGULAR]      ] Requests size: ") + \
      std::to_string(_terRequests.size()));

    _terMutex.lock();           //Lock the queue to remove a request
                               //MultiThreading requires this mutex

	  debug("[ [YELLOW]ECPManager::processTER[REGULAR]      ] Getting TER Request from the queue");

    RequestECP r = _terRequests.front();
    _terRequests.pop();

	  debug("[ [YELLOW]ECPManager::processTER[REGULAR]      ] TER Deleted from the queue");

    _terMutex.unlock();         //Lock the queue to remove a request

	  debug(std::string("Request: ").append(r.read()));

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
		if(is_number){
			tID = atoi(tIDstr.data());
		 	if (tID > 99 || tID < 1)
				is_number = false;
		}
		if(tIDstr.data()[0] == '0' || tIDstr.size() < 1 || tIDstr.size() > 2 || !is_number || trash != std::string("")){

			debug("[ [YELLOW]ECPManager::processTER[REGULAR]      ] It's not a number. Size of input: " +
						std::to_string(tIDstr.size()) + " | input: " + tIDstr);

      answer = "ERR\n";
    }else{
			try{
				debug("[ [YELLOW]ECPManager::processTER[REGULAR]      ] It's a topic number. Size of input: " +
							std::to_string(tIDstr.size()) + " | input: " + tIDstr);

				std::pair <std::string, int> data = topicData(tID);
				answer = std::string("AWTES ") + data.first + std::string(" ") + \
				std::to_string(data.second) + "\n";
			}catch(InvalidTID s){
        debug("[ [YELLOW]ECPManager::processTER[REGULAR]      ] Invalid topic id");
        debug("[ [YELLOW]ECPManager::processTER[REGULAR]      ] answer: EOF");
				answer = "EOF\n";
			}catch(std::string s){
				debug(s);
				answer = "ERR\n";
			}
		}
		// End of Handle

    r.answer(answer);
    _answerMutex.lock();

		debug("[ [YELLOW]ECPManager::processTER[REGULAR]      ] Inserting Request on Answer Queue");
    _answers.push(r);
		debug("[ [YELLOW]ECPManager::processTER[REGULAR]      ] Inserted Request on Answer Queue");
    _answerMutex.unlock();
		debug("[ [YELLOW]ECPManager::processTER[REGULAR]      ] Posting Answer Semaphore");
    sem_post(_answerSemaphore); //So answers thread can know that there is an
                                //answer to send

		debug("[ [YELLOW]ECPManager::processTER[REGULAR]      ] Answer Semaphore Post");
  }
}

void ECPManager::processIQR(){

  debug("[ [CYAN]ECPManager::processIQR[REGULAR]      ] Begin");
  while(!_exit){

    debug("[ [CYAN]ECPManager::processIQR[REGULAR]      ] I'm waiting for requests to process");

    sem_wait(_iqrSemaphore);

    debug(
	    std::string("[ [CYAN]ECPManager::processIQR[REGULAR]      ] Requests size: ") + \
	    std::to_string(_iqrRequests.size()));

    _iqrMutex.lock();           //Lock the queue to remove a request
                                //MultiThreading requires this mutex
    debug("[ [CYAN]ECPManager::processIQR[REGULAR]      ] Getting IQR Request from the queue");

    RequestECP r = _iqrRequests.front();
    _iqrRequests.pop();

    debug("[ [CYAN]ECPManager::processIQR[REGULAR]      ] IQR Deleted from the queue");
    _iqrMutex.unlock();         //Lock the queue to remove a request

    debug(std::string("Request: ").append(r.read()));


		std::string answer;
		std::stringstream stream (r.read());
	  std::string message;
	  std::string SIDstr;
		std::string QIDstr;
	  std::string topic_name;
	  std::string score;
	  std::string trash;

		try{
			stream >> message;
			stream >> SIDstr;
			stream >> QIDstr;
			stream >> topic_name;
			stream >> score;
			stream >> trash;
			bool correctMessageFormat = true;
			if( message == "" || SIDstr == "" || QIDstr == "" || topic_name == "" || score == "" || trash != "")
				throw invalidArguments();

			if(SIDstr.size() != 5) throw invalidArguments();
			for(int index = 0; index < (int) SIDstr.size(); index++)
		    if(SIDstr[index] < '0' || SIDstr[index] > '9') throw invalidArguments();



			/* Checking SIDstr is a number */
			int sid = atoi(SIDstr.data());
			if (message != "IQR" || sid < 10000 || sid > 99999) throw invalidArguments();

			/* Checking score is a number */
			bool only_one = true;
			for(int index = 0; index < (int) score.size(); index++){
		    if((score[index] < '0' || score[index] > '9') && score[index] != '-')
					throw invalidArguments();
				if(score[index] == '-' && only_one)
					only_one = false;
				else if(score[index] == '-' && only_one == false)
				 	throw invalidArguments();
			}
			int scoreNR = std::stoi(score);

			if (scoreNR < -1 || scoreNR > 99) throw invalidArguments();
			if(QIDstr.size() > 24) throw invalidArguments();
			if(topic_name.size() > 25) throw invalidArguments();

			debug("[ [CYAN]ECPManager::processIQR[REGULAR]      ] Received Message in the correctMessageFormat");
			debug("[ [CYAN]ECPManager::processIQR[REGULAR]      ] Message:    " + message);
			debug("[ [CYAN]ECPManager::processIQR[REGULAR]      ] SIDstr:     " + SIDstr);
			debug("[ [CYAN]ECPManager::processIQR[REGULAR]      ] QIDstr:     " + QIDstr);
			debug("[ [CYAN]ECPManager::processIQR[REGULAR]      ] topic_name: " + topic_name);
			debug("[ [CYAN]ECPManager::processIQR[REGULAR]      ] score:      " + score);

			std::string stats_message = SIDstr + std::string(" ") + topic_name + std::string(" ") + score + std::string("%");

			UI::Dialog::IO->println(stats_message);

			std::ofstream iFile;
			iFile.open(_statsFile,std::fstream::app);
			iFile << stats_message.append(std::string("\n"));
			iFile.close();
			answer = std::string("AWI " + QIDstr + "\n");

		}catch(invalidArguments s){
			UI::Dialog::IO->println("[RED][ERR][REGULAR] There was an error in the communication with the server.");
			UI::Dialog::IO->println("[RED][ERR][REGULAR] Error related to the IQR's arguments format.");
			answer = "ERR\n";
		}



		// End of Handle

    r.answer(answer);
    _answerMutex.lock();

		debug("[ [CYAN]ECPManager::processIQR[REGULAR]      ] Inserting Request on Answer Queue");

    _answers.push(r);

		debug("[ [CYAN]ECPManager::processIQR[REGULAR]      ] Inserted Request on Answer Queue");

    _answerMutex.unlock();

		debug("[ [CYAN]ECPManager::processIQR[REGULAR]      ] Posting Answer Semaphore");

    sem_post(_answerSemaphore); //So answers thread can know that there is an
                                //answer to send

		debug("[ [CYAN]ECPManager::processIQR[REGULAR]      ] Answer Semaphore Post");


  }
}

void ECPManager::sendAnswer(){
  debug("[ [BLUE]ECPManager::sendAnswer[REGULAR]      ] Begin");
  while(!_exit){
    debug("[ [BLUE]ECPManager::sendAnswer[REGULAR]      ] I'm waiting for requests to process");
    sem_wait(_answerSemaphore);
    debug(
		  std::string("[ [BLUE]ECPManager::sendAnswer[REGULAR]      ] Requests size: ") + \
		  std::to_string(_answers.size()));

    _answerMutex.lock();          //Lock the queue to remove a request
                                 //MultiThreading requires this mutex
    RequestECP r = _answers.front();

    _answers.pop();
    _answerMutex.unlock();         //Lock the queue to remove a request

    debug(
      std::string("[ [BLUE]ECPManager::sendAnswer[REGULAR]      ] Sending message: ") + \
      r.answer());

    debug("[ [BLUE]ECPManager::sendAnswer[REGULAR]      ] Waiting for UDP Socket to be free");

    _UDPSenderMutex.lock();

    debug("[ [BLUE]ECPManager::sendAnswer[REGULAR]      ] UDP socket is free, will use it now");

    _socketUDP.client(r.client());

    debug(
	    std::string("[ [BLUE]ECPManager::sendAnswer[REGULAR]      ] Sending message: ") + \
	    r.answer());
    _socketUDP.send(r.answer());

	//	_socketUDP.send(r.answer().substr(0, r.answer().size() - 3));

    debug("[ [BLUE]ECPManager::sendAnswer[REGULAR]      ] Message sent by UDP Socket");
		debug(
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
  if(port == 0) throw InvalidTID();
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
