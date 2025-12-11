#include "command.hpp"
#include "config.hpp"
#include "manager.hpp"
#include "models.hpp"
#include "utils.hpp"
#include <cstring>
#include <iostream>
#include <sstream>
#include <string>

UserManager *Command::user_manager_ = nullptr;
BookManager *Command::book_manager_ = nullptr;
LogManager *Command::log_manager_ = nullptr;

void Command::init(UserManager *user_manager, BookManager *book_manager, LogManager *log_manager) {
  user_manager_ = user_manager;
  book_manager_ = book_manager;
  log_manager_ = log_manager;
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
  throw ProgramExitException();
}

const char *Login::Name() const { return "login"; }

int Login::NeedPrivilege() const { return VISITOR; }

void Login::Execute(const std::vector<std::string> &args) {
  //std::cerr << "Login" << std::endl;
  if (args.size() == 1) {
    SystemLog log = user_manager_->Login(args[0]);
    log_manager_->AddSystemLog(log);
    return ;
  }
  if (args.size() == 2) {
    SystemLog log = user_manager_->Login(args[0], args[1]);
    //std::cerr << "finish login" << std::endl;
    log_manager_->AddSystemLog(log);
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
  SystemLog log = user_manager_->Logout();
  log_manager_->AddSystemLog(log);
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
  SystemLog log = user_manager_->Register(args[0], args[1], args[2]);
  log_manager_->AddSystemLog(log);
}

const char *Passwd::Name() const { return "passwd"; }

int Passwd::NeedPrivilege() const { return CUSTOMER; }

void Passwd::Execute(const std::vector<std::string> &args) {
  //std::cerr << "Logout" << std::endl;
  if(args.size() == 2) {
    SystemLog log = user_manager_->Passwd(args[0], args[1]);
    log_manager_->AddSystemLog(log);
  }
  if(args.size() == 3) {
    SystemLog log = user_manager_->Passwd(args[0], args[2], args[1]);
    log_manager_->AddSystemLog(log);
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
  SystemLog log = user_manager_->UserAdd(args[0], args[1], args[2][0] - '0', args[3]);
  strcpy(log.info_, ("priv = " + args[2] + " name = " + args[3]).c_str());
  log_manager_->AddSystemLog(log);
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
  SystemLog log = user_manager_->Delete(args[0]);
  log_manager_->AddSystemLog(log);
}

const char *SelectBook::Name() const { return "select"; }

int SelectBook::NeedPrivilege() const { return STAFF; }

void SelectBook::Execute(const std::vector<std::string> &args) {
  //std::cerr << "Logout" << std::endl;
  if(args.size() != 1) {
    std::stringstream ss;
    ss << "select : need 1 argument but " << args.size() << "are given";
    throw Exception(ss.str());
  }
  user_manager_->SelectBook(book_manager_->UnrollIsbn(args[0]));
}

const char *ImportBook::Name() const { return "import"; }

int ImportBook::NeedPrivilege() const { return STAFF; }

void ImportBook::Execute(const std::vector<std::string> &args) {
  //std::cerr << "Logout" << std::endl;
  if(args.size() != 2) {
    std::stringstream ss;
    ss << "Import : need 2 argument but " << args.size() << "are given";
    throw Exception(ss.str());
  }
  Session session = user_manager_->GetTopSession();
  if(session.index_book_ == -1) {
    throw Exception("Import : havn't selected a book");
  }
  int quantity;
  
  double total_cast;
  try {
    quantity = std::stod(args[0]);
    total_cast = std::stod(args[1]);
  } catch(...) {
    throw Exception("import : arg need to bu num");
  }
  if(quantity <= 0) {
    throw Exception("Import : the quantity is not postive");
  }
  if(total_cast <= 1e-7) {
    throw Exception("Import : the total_cast is not postive");
  } 
  SystemLog log = book_manager_->Import(session.index_book_, quantity);
  strcpy(log.userid_, user_manager_->GetUser(session.index_user_).userid_);
  log.total_amount_ = total_cast;
  log_manager_->AddSystemLog(log);
  log_manager_->AddFinancialLog(-total_cast);
}

const char *ModifyBook::Name() const { return "modify"; }

int ModifyBook::NeedPrivilege() const { return STAFF; }

void ModifyBook::Execute(const std::vector<std::string> &args) {
  //std::cerr << "Logout" << std::endl;
  Session session = user_manager_->GetTopSession();
  if(session.index_book_ == -1) {
    throw Exception("modify : havn't selected a book");
  }
  std::string modify[5] = {""};
  std::string detail;
  for(auto arg : args) {
    detail = detail + arg + " ";
    auto it = arg.find("=");
    std::string type = arg.substr(1, it - 1);
    std::string info = arg.substr(it + 1);
    if (info == "") {
      throw Exception("modify : info is empty");
    }
    if (type == "ISBN") {
      if (modify[0] != "") {
        throw Exception("modify : multiple isbn");
      }
      modify[0] = info;
    } else if (type == "name") {
      if (modify[1] != "") {
        throw Exception("modify : multiple name");
      }
      modify[1] = info.substr(1, info.length() - 2);
    } else if (type == "author") {
      if (modify[2] != "") {
        throw Exception("modify : multiple author");
      }
      modify[2] = info.substr(1, info.length() - 2);
    } else if (type == "keyword") {
      if (modify[3] != "") {
        throw Exception("modify : multiple keyword");
      }
      modify[3] = info.substr(1, info.length() - 2);
    } else if (type == "price") {
      if (modify[4] != "") {
        throw Exception("modify : multiple name");
      }
      modify[4] = info;
    } else {
      throw Exception("modify : invalid arg");
    }
  }
  User current_user = user_manager_->GetUser(session.index_user_);
  SystemLog log = book_manager_->Modify(session.index_book_, modify);
  strcpy(log.userid_, current_user.userid_);
  strcpy(log.info_, detail.c_str());
  log_manager_->AddSystemLog(log);
}

const char *ShowBook::Name() const { return "show"; }

int ShowBook::NeedPrivilege() const { return CUSTOMER; }

void ShowBook::Execute(const std::vector<std::string> &args) {
  //std::cerr << "Logout" << std::endl;
  //std::cerr << "exe : show " << args.size() << std::endl;
  std::string show[4] = {""};
  if(args.size() == 0) {
    book_manager_->Show(show);
    return ;
  }
  if (args.size() == 1) {
    auto it = args[0].find("=");
    std::string type = args[0].substr(1, it - 1);
    std::string info = args[0].substr(it + 1);
    if(info == "") {
      throw Exception("show : the info is empty");
    }
    if (type == "ISBN") {
      show[0] = info;
    } else if (type == "name") {
      show[1] = info.substr(1, info.length() - 2);
    } else if (type == "author") {
      show[2] = info.substr(1, info.length() - 2);
    } else if (type == "keyword") {
      if(info.find("=") != std::string::npos) {
        throw Exception("show : can't find mutiple keyword");
      }
      show[3] = info.substr(1, info.length() - 2);
    } else {
      throw Exception("show : invalid arg");
    }
    book_manager_->Show(show);
    return ;
  }
  throw Exception("show : need 0 / 1 argument");
}

const char *BuyBook::Name() const { return "buy"; }

int BuyBook::NeedPrivilege() const { return CUSTOMER; }

void BuyBook::Execute(const std::vector<std::string> &args) {
  //std::cerr << "Logout" << std::endl;
  if(args.size() != 2) {
    std::stringstream ss;
    ss << "buy : need 2 argument but " << args.size() << "are given";
    throw Exception(ss.str());
  }
  User current_user = user_manager_->GetUser(user_manager_->GetTopSession().index_user_);
  int quantity;
  try {
    quantity = std::stod(args[1]);
  } catch(...) {
    throw Exception("buy : quantity need to be num");
  }
  SystemLog log = book_manager_->Buy(args[0], quantity);
  strcpy(log.userid_, current_user.userid_);
  log_manager_->AddSystemLog(log);
  log_manager_->AddFinancialLog(log.total_amount_);
}

const char *ShowFinance::Name() const { return "showfinance"; }

int ShowFinance::NeedPrivilege() const { return ADMIN; }

void ShowFinance::Execute(const std::vector<std::string> &args) {
  //std::cerr << "Logout" << std::endl;
  if(args.size() == 0) {
    log_manager_->ShowFinance();
    return ;
  }
  if(args.size() == 1) {
    int count;
    try {
      count = std::stod(args[0]);
    } catch(...) {
      throw Exception("show finance : count need to be num");
    }
    log_manager_->ShowFinance(count);
    return ;
  }
  throw Exception("show finance : need 0 / 1 argument");
}

const char *ShowLog::Name() const {return "log";}

int ShowLog::NeedPrivilege() const {return ADMIN;}

void ShowLog::Execute(const std::vector<std::string> &args) {
  log_manager_->PrintLog();
}