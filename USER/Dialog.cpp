#include "Dialog.h"
#include "Debug.h"

#include <boost/algorithm/string/replace.hpp>

#include <iostream>
#include <string>
#include <stdexcept>
#include <fstream>
#include <utility>

namespace UI{
  const std::string Dialog::REGULAR = "\x1B[0m";
  const std::string Dialog::GREEN = "\x1B[32m";
  const std::string Dialog::YELLOW = "\x1B[33m";
  const std::string Dialog::RED = "\x1B[31m";
  const std::string Dialog::BLUE = "\x1B[34m";
  const std::string Dialog::MAGENT = "\x1B[35m";
  const std::string Dialog::CYAN = "\x1B[36m";
  const std::string Dialog::WHITE = "\x1B[37m";


  Dialog* Dialog::IO = new Dialog();
  Dialog::Dialog(){}
  void Dialog::closeDown(){}

  bool Dialog::readBoolean(std::string prompt){
    while(true){
      std::string res = readString(prompt);
      if((res.length() == 1) && ((res[0] == BOOL_TRUE) || (res[0] == BOOL_FALSE)))
        return res[0] == BOOL_TRUE;
      println(std::string("Invalid input. Expecting 'y' or 'n'."));
    }
  }

  std::string Dialog::readString(std::string prompt){
    print(prompt);
    std::string str;
    std::cin >> str;
    return str;
  }
  double Dialog::readDouble(std::string prompt){
    print(prompt);
    while(true){
      std::string a;
      try{
        std::cin >> a;
        return std::stod(a);
      }catch(std::invalid_argument e){
        println(std::string("Invalid input. Expecting 'y' or 'n'."));
      }
    }
  }

  int Dialog::readInteger(std::string prompt){
    try{
      print(prompt);
      flush();
      while(true){
        std::string a;
        try{
          std::cin >> a;
          return std::stoi(a);
        }catch(std::invalid_argument e){
          println(std::string("Invalid input. Expecting a number."));
        }
      }
    }catch(std::out_of_range s){
      println("Invalid input. Number too large." + prompt);
      return readInteger(prompt);
    }
  }
  std::string Dialog::filterString(std::string s){
    boost::replace_all(s, "[REGULAR]", Dialog::REGULAR);
    boost::replace_all(s, "[RED]", Dialog::RED);
    boost::replace_all(s, "[GREEN]", Dialog::GREEN);
    boost::replace_all(s, "[YELLOW]", Dialog::YELLOW);
    boost::replace_all(s, "[BLUE]", Dialog::BLUE);
    boost::replace_all(s, "[MAGENT]", Dialog::MAGENT);
    boost::replace_all(s, "[CYAN]", Dialog::CYAN);
    boost::replace_all(s, "[WHITE]", Dialog::WHITE);
    return s;
  }

  void Dialog::flush(){
    std::cout << std::flush;
  }
  void Dialog::print(std::string text){
    _mutex.lock();
    std::cout << filterString(text);
    _mutex.unlock();
  }
  void Dialog::println(std::string text){
    if(text != "") print(text.append("\n"));
    else print(std::string("\n"));
  }
}
