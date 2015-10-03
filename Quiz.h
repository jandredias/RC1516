#pragma once
#include <string>

class Quiz{
  int _deadline;
  std::string _filename;
public:
  Quiz(){}
  Quiz(int deadline, std::string filename);
  int deadline();
  std::string filename();
  void deadline(int deadline);
  void filename(std::string filename);
};
