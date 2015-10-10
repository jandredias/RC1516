#pragma once
#include <string>
#include <vector>
#include <utility>      // std::pair, std::make_pair

#define MS_BETWEEN_TRIES 2000
#define TRIES 10

class UserManager {
  int         _sid;
  int         _port;
  std::string _ecpname;

  std::string _qid;
  std::string _tesname;
  int         _tesport;

public:

  /**
   * @param int             student's id
   * @param int             port to connect with ECP
   * @param std::string     ecp hostname
   */
  UserManager(int, int, std::string);

  /**
   * @description           lists available topics
   * @return std::vector<std::string> will return the vector containing
   * strings with topics received from the ecp server
   */
  std::vector<std::string> list();

  /**
   * @description          requests
   * @param int
   * @return std::pair<std::string, std::string> will return the questionnaire id, that is also the pdf
   * file name and the deadline for submiting an answer.
   */
  std::pair<std::string, std::string> request(int);

  /**
   * @description          submits answers
   * @param std::string answers
   * @return std::string
   */
  std::pair<std::string, int> submit(std::string);

  /**
   * @description
   * @return std::string
   */
  std::string qid();
};
