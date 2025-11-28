#include "command.hpp"
#include "config.hpp"
#include "manager.hpp"
#include "utils.hpp"
#include <iostream>
#include <sstream>

UserManager *Command::user_manager_ = nullptr;
BookManager *Command::book_manager_ = nullptr;

void Command::init(UserManager *user_manager, BookManager *book_manager) {
  user_manager_ = user_manager;
  book_manager_ = book_manager;
}

void Command::run(const std::vector<std::string> &args) {
  const Session session = user_manager_->GetTopSession();
  User user = user_manager_->GetUser(session.index_user_);
  if (user.privilege_ < NeedPrivilege()) {
    std::stringstream ss;
    ss << Name() << " : privilege required " << NeedPrivilege()
       << " current_user : " << user.userid_ << " have " << user.privilege_;
    throw Exception(ss.str());
  }
  Execute(args);
}

const char *Exit::Name() const { return "exit"; }

int Exit::NeedPrivilege() const { return VISITOR; }

void Exit::Execute(const std::vector<std::string> &args) {
  if (args.size() != 0) {
    std::stringstream ss;
    ss << "Exit : need 0 argument but given" << args.size();
    throw Exception(ss.str());
  }
  exit(0);
}

const char *Login::Name() const { return "login"; }

int Login::NeedPrivilege() const { return VISITOR; }

void Login::Execute(const std::vector<std::string> &args) {
  //std::cerr << "Login" << std::endl;
  if (args.size() == 1) {
    user_manager_->Login(args[0]);
    return ;
  }
  if (args.size() == 2) {
    user_manager_->Login(args[0], args[1]);
    return ;
  }
  throw Exception("Login : need 1/2 argument");
}

const char *Logout::Name() const { return "logout"; }

int Logout::NeedPrivilege() const { return CUSTOMER; }

void Logout::Execute(const std::vector<std::string> &args) {
  //std::cerr << "Logout" << std::endl;
  if(args.size() != 0) {
    std::stringstream ss;
    ss << "Register : need 0 argument but " << args.size() << "are given";
    throw Exception(ss.str());
  }
  user_manager_->Logout();
}

const char *Register::Name() const { return "register"; }

int Register::NeedPrivilege() const { return VISITOR; }

void Register::Execute(const std::vector<std::string> &args) {
  //std::cerr << "Logout" << std::endl;
  if(args.size() != 3) {
    std::stringstream ss;
    ss << "Register : need 3 argument but " << args.size() << "are given";
    throw Exception(ss.str());
  }
  user_manager_->Register(args[0], args[1], args[2]);
}

const char *Passwd::Name() const { return "passwd"; }

int Passwd::NeedPrivilege() const { return CUSTOMER; }

void Passwd::Execute(const std::vector<std::string> &args) {
  //std::cerr << "Logout" << std::endl;
  if(args.size() == 2) {
    user_manager_->Passwd(args[0], args[1]);
  }
  if(args.size() == 3) {
    user_manager_->Passwd(args[0], args[2], args[1]);
  }
}

const char *UserAdd::Name() const { return "useradd"; }

int UserAdd::NeedPrivilege() const { return STAFF; }

void UserAdd::Execute(const std::vector<std::string> &args) {
  //std::cerr << "Logout" << std::endl;
  if(args.size() != 4) {
    std::stringstream ss;
    ss << "UserAdd : need 4 argument but " << args.size() << "are given";
    throw Exception(ss.str());
  }
  user_manager_->UserAdd(args[0], args[1], args[2][0] - '0', args[3]);
}

const char *DeleteUser::Name() const { return "delete"; }

int DeleteUser::NeedPrivilege() const { return ADMIN; }

void DeleteUser::Execute(const std::vector<std::string> &args) {
  //std::cerr << "Logout" << std::endl;
  if(args.size() != 1) {
    std::stringstream ss;
    ss << "UserAdd : need 1 argument but " << args.size() << "are given";
    throw Exception(ss.str());
  }
  user_manager_->Delete(args[0]);
}
