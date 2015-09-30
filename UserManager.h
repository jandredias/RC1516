#pragma once
#include <string>

#ifndef DEBUG
#define DEBUG 0
#endif


#define MS_BETWEEN_TRIES 2000
#define TRIES 10

class UserManager {
  int _sid;
  int _port;
  std::string _ecpname;

  std::string _tesname;
  int _tesport;
public:

  /**
   * @param int             student's id
   * @param int             port to connect with ECP
   * @param std::string     ecp hostname
   */
  UserManager(int, int, std::string);

  /**
   * @description           lists available topics
   * @return std::string will return the string containg the topics received
   * from the ecp server
   */
  std::string list();

  /**
   * @description          requests
   * @return std::string will return the questionnaire id, that is also the pdf
   * file name
   */
  std::string request(int);

  /**
   * @description          submits answers
   */
  void submit(int, char*);
};
