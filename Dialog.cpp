#include "Dialog.h"

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
    print(prompt);
    while(true){
      std::string a;
      try{
        std::cin >> a;
        return std::stoi(a);
      }catch(std::invalid_argument e){
        println(std::string("Invalid input. Expecting a number."));
      }
    }
  }
  std::string Dialog::filterString(std::string s){
    boost::replace_all(s, "[REGULAR]", Dialog::REGULAR);
    boost::replace_all(s, "[RED]", Dialog::RED);
    boost::replace_all(s, "[GREEN]", Dialog::GREEN);
    boost::replace_all(s, "[YELLOW]", Dialog::YELLOW);
    boost::replace_all(s, "[BLUE]", Dialog::BLUE);
    boost::replace_all(s, "[MAGENTA]", Dialog::MAGENT);
    boost::replace_all(s, "[CYAN]", Dialog::CYAN);
    boost::replace_all(s, "[WHITE]", Dialog::WHITE);
    return s;
  }
  void Dialog::print(std::string text){
    std::cout << filterString(text);
  }
  void Dialog::println(std::string text){
    if(text != "") print(text);
    std::cout << std::endl;
  }

  void Dialog::XMLTag(std::ofstream &file, std::string tag, std::string value){
    file  << "<"  <<  tag  << ">"
                  << value
          << "</" <<  tag  << ">" << std::endl;
  }
  void Dialog::XMLTag(std::ofstream &file, std::string tag, int value){
    XMLTag(file, tag, std::to_string(value));
  }
  void Dialog::XMLTag(std::ofstream &file, const char tag[], std::string value){
    XMLTag(file, std::string(tag), value);
  }
  void Dialog::XMLTag(std::ofstream &file, const char tag[], int value){
    XMLTag(file, std::string(tag), std::to_string(value));
  }
  void Dialog::XMLTag(std::ofstream &file, std::string tag, const char value[]){
    XMLTag(file, tag, std::string(value));
  }
  void Dialog::XMLTag(std::ofstream &file, const char tag[], const char value[]){
    XMLTag(file, std::string(tag), std::string(value));
  }
  std::pair<std::string,std::string> Dialog::XMLTag(std::ifstream &myfile){
    std::pair<std::string,std::string> data;
    char c;
    /**
     * States:
     *  0 Initial
     *  1 Inside open Tag
     *  2 Inside Value
     *  3 Inside Close Tag
     *  4 Done
     */
    int state = 0;
    if(myfile.is_open()){
      std::string value = "";
      while(1){
        myfile.read(&c, sizeof(char));
        if(myfile.eof()) throw std::string("EOF");
        if(c == ' ' || c == '\t' || c == '\n') continue;
        if(c == '<'){
          if(state == 2){
            data.second = value;
            value = "";
            myfile.read(&c, sizeof(char));
            if(c != '/') throw std::string("InputFile not formated correctly");
          }
          state++;
          continue;
        }
        if(c == '>'){
          if(state == 1){
            data.first = value;
            value = "";
          }
          if(state == 3){
            if(data.first != value){
              throw std::string("InputFile not formated correctly");
            }
            return data;
          }
          state++;
          continue;
        }
        value += c;
      }
    }else{
      throw std::string("Unable to open file");
    }
  }


}
