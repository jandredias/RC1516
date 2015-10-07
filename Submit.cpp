#include "Submit.h"
#include "Debug.h"

#include <iostream>

#include "Dialog.h"
#include "Exception.h"

namespace RC_User{
  Submit::Submit(UserManager *manager) :
       Command<UserManager>("Submit", manager){}
    void Submit::execute(){
      try{
        std::string answers = "";
        for(int i = 0; i < 5; i++){
          UI::Dialog::IO->print("Answer to question " +
                                std::to_string(i + 1) + "? ");
          answers += UI::Dialog::IO->readString();
          if(i < 4) answers += " ";
        }

        std::pair <std::string,int> score = _receiver->submit(answers);
        UI::Dialog::IO->println("You got " + std::to_string(score.second) +
                                "% on questionnaire " + score.first);

      }catch(NoRequestAsked s){
        UI::Dialog::IO->println("You should first request a questionnaire.");
      }catch(ErrorOnMessage s){
        UI::Dialog::IO->println("Invalid request");
      }catch(UnknownFormatProtocol s){
        UI::Dialog::IO->println(std::string("Error during communication with the Server"));
      }catch(AfterDeadlineSubmit s){
        UI::Dialog::IO->println("Questionaire submitted after deadline, therefore no score.");
      }catch(InvalidQIDvsSID s){
        UI::Dialog::IO->println("The submitted questionnaire id is not valid with your student id");
      }catch(std::string s){
        UI::Dialog::IO->println(s);
      }
    }
}
