#ifndef MANAGER
#define MANAGER

#include <vector>
#include <string>
#include <map>
#include "models.hpp"

struct Session {
  size_t index_user_;
  size_t index_book_ = -1;
  Session(size_t index_user):index_user_(index_user) {}
};

class UserManager {
private: 
  std::vector<Session> log_stack_;
  std::vector<User> user_list_;
  std::map<std::string, size_t> id_user_;
public:
  UserManager();
  const Session GetTopSession();
  const User GetUser(size_t);
  SystemLog Login(std::string, std::string = "");
  SystemLog Logout();
  SystemLog Register(std::string, std::string, std::string);
  SystemLog UserAdd(std::string, std::string, int, std::string);
  SystemLog Passwd(std::string, std::string, std::string = "");
  SystemLog Delete(std::string);
  void SelectBook(size_t);
};

class BookManager {
private:
  std::vector<Book> book_list_;
  std::map<std::string, size_t> isbn_book_;
  std::multimap<std::string, size_t> name_book_;
  std::multimap<std::string, size_t> author_book_;
  std::multimap<std::string, size_t> key_book_;
public:
  const Book GetBook(size_t);
  size_t UnrollIsbn(std::string);
  SystemLog Buy(std::string, int);
  SystemLog Modify(size_t, const std::string []);
  SystemLog Import(size_t, int);
  void Show(const std::string []);
};

class LogManager {
private:
  std::vector<FinancialLog> financial_log_;
  std::vector<SystemLog> system_log;
public:
  LogManager();
  void ShowFinance(int = -1);
  void ReportFinance();
  void PrintStaff();
  void PrintLog();
  void AddFinancialLog(double);
  void AddSystemLog(SystemLog &);
};

#endif
