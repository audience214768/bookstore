#include <iostream>
#include <memory>
#include <sstream>
#include <string>
#include <filesystem>
#include "command.hpp"
#include "manager.hpp"
#include "utils.hpp"
#include "journal.hpp"

std::unique_ptr<Command> CreatCommand(const std::vector<std::string> &args) {
  std::string type = args[0];
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
    if(args[1] == "finance") {
      return std::make_unique<ShowFinance>();
    } else {
      return std::make_unique<ShowBook>();
    }
  }
  if(type == "buy") {
    return std::make_unique<BuyBook>();
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
  //JournalManager journal_manager("data/journal.log");
  Command::init(&user_manager, &book_manager, &log_manager);
  //std::cerr << "finish init" << std::endl;
  std::string command;
  while(std::getline(std::cin, command)) {
    std::vector<std::string> args;
    std::stringstream ss(command);
    std::string arg_line = ss.str();
    std::string current_arg;
    bool is_inquote = 0;
    for (auto c : arg_line) {
      if (c == '"') {
        current_arg += c;
        is_inquote = !is_inquote;
      } else if (c == ' ') {
        if (is_inquote) {
          current_arg += c;
        } else {
          if(current_arg != "") {
            args.push_back(current_arg);
          }
          //std::cerr << current_arg << std::endl;
          current_arg = "";
        }
      } else {
        current_arg += c;
      }
    }
    if(current_arg != "") {
      args.push_back(current_arg);
    }
    //std::cerr << "finish parse" << std::endl;
    try{
      auto cmd = CreatCommand(args);
      //std::cerr << "creat command" << std::endl;
      cmd->run(args);
      //std::cerr << "finish " << type << std::endl;
    } catch(Exception &err) {
      printf("Invalid\n");
      std::cerr << err.what() << std::endl;
    } catch(ProgramExitException &err) {
      break;
    } catch(...) {
      std::cerr << "something unecpected" << std::endl;
    }
  }
}