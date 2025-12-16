#ifndef MANAGER
#define MANAGER

#include <vector>
#include <string>
#include "journal.hpp"
#include "models.hpp"
#include "unrollindex.hpp"
#include "memoryriver.hpp"

struct Session {
  size_t index_user_;
  size_t index_book_ = -1;
  Session(size_t index_user):index_user_(index_user) {}
};

class UserManager {
private: 
  std::vector<Session> log_stack_;
  MemoryRiver<User> user_list_;
  UnrollIndex<FixedString<31>, int> id_user_;
public:
  UserManager(JournalManager &);
  const Session GetTopSession();
  User GetUser(size_t);
  User GetUser(std::string);
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
  MemoryRiver<Book> book_list_;
  UnrollIndex<FixedString<21>, int> isbn_book_;
  UnrollIndex<FixedString<61>, int> name_book_;
  UnrollIndex<FixedString<61>, int> author_book_;
  UnrollIndex<FixedString<61>, int> key_book_;
public:
  BookManager(JournalManager &);
  const Book GetBook(size_t);
  int UnrollIsbn(std::string);
  SystemLog Buy(std::string, long long);
  SystemLog Modify(size_t, const std::string []);
  SystemLog Import(size_t, int);
  void Show(const std::string []);
};

class LogManager {
private:
  MemoryRiver<FinanceLog> finance_log_;
  MemoryRiver<SystemLog> system_log;
public:
  LogManager(JournalManager &);
  void ShowFinance(int = -1);
  void ReportFinance();
  void PrintStaff(UserManager *);
  void PrintLog();
  void AddFinancialLog(double);
  void AddSystemLog(SystemLog &);
};

#endif
