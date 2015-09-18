#include <iostream>
#include "MenuBuilder.h"
#include "UserManager.h"
#include "Dialog.h"

#define __PORT__ 58023
#define __HOST__ "localhost"
#define __DEBUG__ 0
int main(int argc, char* argv[]){
  int sid = 0;
  int port = __PORT__;
  std::string ecpname = __HOST__;

  if(argc < 2){
    UI::Dialog::IO->println("you need to write a SID composed of 5 digits (e.g. 76543)");
    UI::Dialog::IO->println("Try 'user --help' for more options.");
    return 1;
  }

  if(argc == 2 && (std::string(argv[1]) == std::string("--help") || std::string(argv[1]) == std::string("-H"))){
    UI::Dialog::IO->println("Online Questionnaire 1.0.0");
    UI::Dialog::IO->println("Usage: user [sid] [OPTION]");
    UI::Dialog::IO->println();
    UI::Dialog::IO->println("  sid                           you need to write a SID composed of 5 digits (e.g. 76543)");
    UI::Dialog::IO->println();
    UI::Dialog::IO->println("Options:");
    UI::Dialog::IO->println();
    UI::Dialog::IO->println("  -H,  --help                   display this screen message");
    UI::Dialog::IO->println("  -n,  --hostname               specify an ECP hostname to use");
    UI::Dialog::IO->println("  -p,  --port                   specify an ECP port to use");
    UI::Dialog::IO->println();
    return 0;
  }
  if(argc % 2 == 1){
    UI::Dialog::IO->println("wrong number of parameteres");
    return 1;
  }

  sid = atoi(argv[1]);

  if(sid < 10000 || sid > 99999){
    UI::Dialog::IO->println("wrong format of identity number! it should be composed of 5 digits (e.g. 76543)");
    return 1;
  }
  for(int i = 2; i < argc; i += 2){
    if(std::string(argv[i]) != std::string("-n") && \
       std::string(argv[i]) != std::string("-p")){
      UI::Dialog::IO->println("wrong format of parameters");
    }
    else if(std::string(argv[i]) == std::string("-n")){
      //should redefine ecp hostname
      ecpname = argv[i+1];
    }else{
      //should redefine ecp port
      port = atoi(argv[i+1]);
    }
  }
  UserManager manager(sid, port, ecpname);
  RC_User::MenuBuilder::menuFor(&manager);
  UI::Dialog::IO->println("Bye!");
  return 0;
}
