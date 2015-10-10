#include "MenuBuilder.h"
#include "Debug.h"

#include "Menu.h"
#include "Command.h"

#include "List.h"
#include "Request.h"
#include "Submit.h"

void RC_User::MenuBuilder::menuFor(UserManager *manager){
  UI::Command<UserManager> *command[] = {
    new List(manager),
    new Request(manager),
    new Submit(manager)
  };
  UI::Menu<UserManager> *menu = new UI::Menu<UserManager>("Online Questionnaire", command, 3);
  menu->open();
}
