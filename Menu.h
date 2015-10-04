#pragma once

#include <boost/algorithm/string.hpp>

#include "Command.h"
#include "Dialog.h"
#include <iostream>
#include <string>
#include <vector>

namespace UI{
  template <class T>
  class Menu {
    static const std::string EXIT_OPTION;
    static const std::string SELECT_OPTION;
    static const std::string INVALID_OPTION;

    std::string _title;
    std::vector<Command<T> *> _commands;

  public:
    /**
     * @param title
     *            menu title.
     * @param commands
     *            list of commands managed by the menu
     */
    Menu(std::string title, Command<T> *commands[], int size) : _title(title){
     for(auto i = 0; i < size; i++)
       _commands.push_back(commands[i]);
    }

    /**
     * Main function: the menu interacts with the user and Executes
     * the appropriate commands
     */
    void open(){
      auto option = -1;
      while(true){
        UI::Dialog::IO->println();  //Just to be cute :P
        UI::Dialog::IO->println(_title);
        for(auto i = 0; i < _commands.size(); i++){
          //FIXME if(_commands[i].isValid())
          UI::Dialog::IO->print(std::to_string(i+1));
          UI::Dialog::IO->print(std::string(" - "));
          UI::Dialog::IO->println(_commands[i]->title());

        }
        UI::Dialog::IO->println("0 - Exit");

        std::string option_str = UI::Dialog::IO->readString(SELECT_OPTION);


        bool is_number = true;
        for(int index = 0; index < option_str.size(); index++)
          if(option_str[index] < '0' || option_str[index] > '9') is_number = false;
        if(is_number) option = atoi(option_str.data());
        if(!is_number){
         boost::to_upper(option_str);
         if(option_str == boost::to_upper_copy<std::string>(std::string("Exit"))){
           option = 0;
         }
         else
           for(auto i = 0; i < _commands.size(); i++)
              if(option_str == boost::to_upper_copy<std::string>(std::string(_commands[i]->title()))){
                option = i + 1;
                break;
              }
        }
        //IO.readInteger(SELECT_OPTION);
        if(option == 0) return;
        if(option < 0 ||
           option > _commands.size() // ||
           //!_commands[option - 1]->isValid()
          ){
          UI::Dialog::IO->println(INVALID_OPTION);
        }
        else{
          _commands[option - 1]->execute();
          if(_commands[option -1]->isLast()) return;
        }


      }
    }
  };

  template <class T>
  const std::string Menu<T>::EXIT_OPTION = "0 - Exit";
  template <class T>
  const std::string Menu<T>::SELECT_OPTION = "Choose one option: ";
  template <class T>
  const std::string Menu<T>::INVALID_OPTION = "Invalid option!";
}
