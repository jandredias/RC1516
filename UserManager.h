#pragma once
#include <string>

class UserManager {
  int _sid;
  int _port;
  std::string _ecpname;
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
  void submit();
};
