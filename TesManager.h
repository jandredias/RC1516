#pragma once

#include "RequestTES.h"
#include "SocketUDP.h"
#include "SocketTCP.h"
#include <queue>
#include <semaphore.h>
#include <mutex>
#include <map>
#include <unordered_map>
#include "Quiz.h"
#ifndef DEBUG
#define DEBUG 0
#endif
#define __MS_BETWEEN_TRIES__ 2000
#define __TRIES__ 10

#define ANSWER_NO 10

class TesManager{
  std::string _ecpname;
  int         _ecpport;
  std::queue<RequestTES> _requests;    ///Queue that contains the requests received to be processed
  std::queue<RequestTES> _rqtRequests; ///RQT requests to be processed by processRQT
  std::queue<RequestTES> _rqsRequests; ///RQS requests to be processed by processRQS
  std::queue<RequestTES> _awiRequests; ///AQI requests to be processed by processAWI
  std::queue<RequestTES> _answers;     ///Answers TCP to be sent to clients

  std::unordered_map<std::string, RequestTES> _answersUDP;  ///Answers UDP to be sent to ECP server

  std::queue<std::string> _pendingQID; ///No fucking idea

  std::map<std::string, Quiz> _questionaries; ///questionnaire map to save data from students

  sem_t * _requestsSem;
  sem_t * _rqtRequestsSem;
  sem_t * _rqsRequestsSem;
  sem_t * _awiRequestsSem;
  sem_t * _answerSem;
  sem_t * _answerUDPSem;
  sem_t * _questionariesSem;

  std::mutex _questionariesMutex;
  std::mutex _reqMutex;
  std::mutex _rqtMutex;
  std::mutex _rqsMutex;
  std::mutex _awiMutex;
  std::mutex _answerMutex;
  std::mutex _answerUDPMutex;

  std::mutex _receiverSocketUDPMutex;
  std::mutex _senderSocketUDPMutex;

  SocketUDP _socketUDP;
  SocketUDP *_senderSocketUDP;

  int _qid;
  int _port;
  bool _exit;

  int _requestID;

  std::string _topicName;
public:
  /**
   * @description           TesManager will manage the TES server
   *                        it will receive requests and answer them
   *                        there will be several threads working
   *                        acceptRequests will be one thread, and it will accept
   *                        connections and push them to a vector
   *
   *                        next the processRequests will get those connections
   *                        and process them
   *
   *                        this way the acceptRequests thread will be always free
   *                        accept new clients requests while other thread will be
   *                        working on current requests.
   *
   *                        in the future there might be several processRequests
   *                        threads working at the same time
   */
   /**
    * @param int             port TES is listening (TCP)
    * @param int             port to connect with ECP (UDP)
    * @param std::string     ecp hostname
    */
  TesManager(int, int, std::string);
  ~TesManager();

  /**
   * @return                current unix timestamp
   * @param int timestamp
   */
  int time(int = 0);

  /**
   * @param                 seconds untill the deadline
   * @return                a deadline
   */
  int deadline(int = 300);

  /**
   * @description will parse a date and return True if the string
   * is a correctly formated date
   * @param std::string input date to process
   * @return bool
   */
  bool deadline(std::string);

  /**
   * @return                always returns new quiz id
   */
  std::string qid(int = 99999);

  /**
   *@description            thread method that will accept new TCP clients
   */
  void acceptRequestsTCP();

  /**
   *@description            thread method that will accept new UDP clients
   */
  void acceptRequestsUDP();

  /**
   * @description  will process TCP requests and filter them
   */
  void processTCP();

  /**
   * @description           will process RQT requests
   */
  void processRQT();

  /**
   * @description           will process RQS requests
   */
  void processRQS();

  /**
   * @description           will process AWI requests
   */
  void processAWI();

  /**
   * @description           will send answers to TCP clients
   */
  void answerTCP();


  /**
   * @description           will send answers to ECP server using UDP protocol
   */
  void answerUDP();
  /**
   * @description will return the score for a questionnaire and a set of answers
   * @param char[] answers
   * @param const char file name
   * @return int score
   */
  int score(char[], const char[]);

  /**
   * @description
   * @return int pdf size
   */
  int pdfSize(std::string);

  /**
   * @description will convert a deadline timestamp to a String
   * @int timestamp
   * @return std::string
   */
  std::string toStringDeadline(int s);
};
