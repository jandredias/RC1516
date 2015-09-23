#pragma once
#include <string>

#ifndef DEBUG
#define DEBUG 0
#endif

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
   */
  void list();

  /**
   * @description          requests
   */
  void request(int);

  /**
   * @description          submits answers
   */
  void submit(int, char*);
};
