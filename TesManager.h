#pragma once

#include <map>
#include "RequestTES.h"
#include "SocketUDP.h"
#include "SocketTCP.h"
#include <queue>
#include <semaphore.h>
#include <mutex>

#ifndef DEBUG
#define DEBUG 0
#endif
#define __MS_BETWEEN_TRIES__ 2000
#define __TRIES__ 10

class TesManager{
  std::string _ecpname;
  int         _ecpport;
  std::queue<RequestTES> _requests;
  std::queue<RequestTES> _rqtRequests;
  std::queue<RequestTES> _rqsRequests;
  std::queue<RequestTES> _awiRequests;
  std::queue<RequestTES> _answers;
  std::queue<RequestTES> _answersUDP;

  std::queue<std::string> _pendingQID;

  std::map<std::string, std::string> _questionaries;

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

  SocketUDP *_receiverSocketUDP;
  SocketUDP *_senderSocketUDP;

  int _qid;
  int _port;
  bool _exit;

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
   */
  int time();

  /**
   * @param                 seconds untill the deadline
   * @return                a deadline
   */
  int deadline(int = 600);

  /**
   * @return                always returns new quiz id
   */
  std::string qid();

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
    * @description          will ensure the IQR requests are received
    */
  void processQID();

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
   * @return  std::pair <char*, int> pdf content and size
   */
  std::pair<char *, int> pdf(std::string);

  /**
    * @description will send a message to ECP server with the user score
    * @param std::string SID
    * @param std::string QID
    * @param std::string topic_name
    * @param int scr
    */
  void sendIQR(std::string SID,std::string QID,std::string topic_name,int scr);

};
