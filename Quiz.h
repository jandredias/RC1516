#pragma once
#include <string>

class Quiz{
  int _deadline;
  int _sid;
  std::string _filename;

public:
  Quiz(){}
  Quiz(int sid, int deadline, std::string filename);
  int deadline();
  std::string filename();
  void deadline(int deadline);
  void filename(std::string filename);
  int sid();
};
