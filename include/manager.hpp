#ifndef MANAGER
#define MANAGER

#include <vector>
#include <string>
#include <map>
#include "models.hpp"

struct Session {
  size_t index_user_;
  size_t index_book_ = 0;
  Session(size_t index_user):index_user_(index_user) {}
};

class UserManager {
private: 
  std::vector<Session> log_stack_;
  std::vector<User> user_list_;
  std::map<std::string, size_t> id_user_;
public:
  UserManager();
  const Session &GetTopSession();
  const User GetUser(size_t);
  void Login(std::string, std::string = "");
  void Logout();
  void Register(std::string, std::string, std::string);
  void UserAdd(std::string, std::string, int, std::string);
  void Passwd(std::string, std::string, std::string = "");
  void Delete(std::string);
  void SelectBook(std::string);
};

class BookManager {
private:
  std::vector<Book> book_list_;
  std::map<std::string, size_t> isbn_book_;
  std::map<std::string, size_t> name_book_;
  std::map<std::string, size_t> author_book_;
  std::map<std::string, size_t> key_book_;
public:
  const Book GetBook(size_t);
  void Buy(std::string, int);
  void Modify(size_t);
};

class LogManger {
private:
  std::vector<FinancialLog> financial_log_;
  std::vector<SystemLog> system_log;
public:
  LogManger();
  void PrintFinancial();
  void PrintStaff();
  void PrintLog();
};

#endif
