#include <iostream>
#include <memory>
#include <sstream>
#include <string>
#include "command.hpp"
#include "manager.hpp"
#include "utils.hpp"

std::unique_ptr<Command> CreatCommand(std::string type) {
  if(type == "exit" || type == "quit") {
    return std::make_unique<Exit>();
  }
  if(type == "su") {
    return std::make_unique<Login>();
  }
  if(type == "logout") {
    return std::make_unique<Logout>();
  }
  if(type == "register") {
    return std::make_unique<Register>();
  }
  if(type == "passwd") {
    return std::make_unique<Passwd>();
  }
  if(type == "useradd") {
    return std::make_unique<UserAdd>();
  }
  if(type == "delete") {
    return std::make_unique<DeleteUser>();
  }
  if(type == "select") {
    return std::make_unique<SelectBook>();
  }
  if(type == "import") {
    return std::make_unique<ImportBook>();
  }
  if(type == "modify") {
    return std::make_unique<ModifyBook>();
  }
  if(type == "show") {
    return std::make_unique<ShowBook>();
  }
  if(type == "buy") {
    return std::make_unique<BuyBook>();
  }
  if(type == "show finance") {
    return std::make_unique<ShowFinance>();
  }
  if(type == "log") {
    return std::make_unique<ShowLog>();
  }
  /*
  if(type == "report finance") {
    return std::make_unique<ReportFinance>();
  }
  if(type == "report employee") {
    return std::make_unique<ReportEmployee>();
  }*/
  throw Exception("not implement this command");
}

int main() {
  UserManager user_manager;
  BookManager book_manager;
  LogManager log_manager;
  Command::init(&user_manager, &book_manager, &log_manager);
  //std::cerr << "finish init" << std::endl;
  std::string command;
  while(std::getline(std::cin, command)) {
    std::vector<std::string> args;
    std::stringstream ss(command);
    std::string type;
    ss >> type;
    if (type == "") {
      continue;
    }
    if (type == "show") {
      std::string temp;
      ss >> temp;
      if (temp == "finance") {
        type += " " + temp;
      } else if(temp != "") {
        args.push_back(temp);
      }
    }
    if(type == "report") {
      std::string temp;
      ss >> temp;
      type += " " + temp;
    }
    std::string temp;
    while(ss >> temp) {
      args.push_back(temp);
    }
    //std::cerr << "finish parse" << std::endl;
    try{
      auto cmd = CreatCommand(type);
      //std::cerr << "creat command" << std::endl;
      cmd->run(args);
      //std::cerr << "finish " << type << std::endl;
    } catch(Exception &err) {
      printf("Invalid\n");
      std::cerr << err.what() << std::endl;
    }
  }
}