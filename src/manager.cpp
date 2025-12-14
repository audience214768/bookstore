#include "manager.hpp"
#include "config.hpp"
#include "journal.hpp"
#include "models.hpp"
#include "utils.hpp"
#include "unrollindex.hpp"
#include <cerrno>
#include <cstddef>
#include <cstdio>
#include <cstring>
#include <sstream>
#include <iostream>
#include <string>
#include <set>
#include <algorithm>

UserManager::UserManager(JournalManager &jm):user_list_(jm, "user.dat"), id_user_(jm, "user_id.dat") {
  int index;
  auto user_index = id_user_[std::string("virtual_visitor")];
  if(user_index.empty()) {
    //std::cerr << 1 << std::endl;
    index = user_list_.write(User("virtual_visitor", "audience", "virtual", VISITOR));
    id_user_.Insert(std::string("virtual_visitor"), index);
  } else {
    index = user_index[0];
  }
  log_stack_.push_back(Session(index));
  user_index = id_user_[std::string("root")];
  if(user_index.empty()) {
    index = user_list_.write(User("root", "sjtu", "audience", ADMIN));
    id_user_.Insert(std::string("root"), index);
  }
  //user_index = id_user_[std::string("worker")];
  //std::cerr << user_index.empty() << std::endl;
}


const Session UserManager::GetTopSession() {
  return log_stack_.back();
}

User UserManager::GetUser(size_t index) {
  return user_list_[index];
}

SystemLog UserManager::Login(std::string id, std::string pwd) {
  //std::cerr << id << std::endl;
  try {
    expect(id).toBeLength(1, 30);
  } catch(...) {
    throw Exception("login : invalid id");
  }
  auto user_index = id_user_[id];
  if(user_index.empty()) {
    std::stringstream ss;
    ss << "login : thd id : " << id << " is not exist";
    throw Exception(ss.str());
  }
  int index = user_index[0];
  User user = user_list_[index];
  //std::cerr << user.userid_ << std::endl;
  Session session = GetTopSession();
  User current_user = user_list_[session.index_user_];
  std::cerr << id << " " << user.password_ << " " << pwd << std::endl;
  if(pwd != "") {
    if (strcmp(pwd.c_str(), user.password_) == 0) {
      log_stack_.push_back(index);
    } else {
      throw Exception("Login : pwd is wrong");
    }
  } else {
    if(current_user.privilege_ > user.privilege_) {
      log_stack_.push_back(Session(index));
    } else {
      throw Exception("Login : need pwd");
    }
  }

  
  //std::cerr << "finish login" << std::endl;
  return SystemLog(id.c_str(), "login", "", 0, 0, "");
}

SystemLog UserManager::Logout() {
  if(log_stack_.size() == 1) {
    throw Exception("there is no account now");
  }
  Session session = log_stack_.back();
  User current_user = user_list_[session.index_user_];
  std::string id = current_user.userid_;
  log_stack_.pop_back();
  return SystemLog(id.c_str(), "logout", "", 0, 0, "");
}

SystemLog UserManager::Register(std::string id, std::string pwd, std::string name) {
  try {
    expect(id).toBeLength(1, 30).toMatch("^[a-zA-Z0-9_]+$");
  } catch(...) {
    throw Exception("register : invalid id");
  }
  try {
    expect(pwd).toBeLength(1, 30).toMatch("^[a-zA-Z0-9_]+$");
  } catch(...) {
    throw Exception("register : invalid pwd");
  }
  try {
    expect(name).toBeLength(1, 30).toMatch("^[\\x21-\\x7E]+$");
  } catch(...) {
    throw Exception("register : invalid name");
  }
  auto user_index = id_user_[id];
  if(!user_index.empty()) {
    throw Exception("Register : the id is used");
  }
  int index = user_list_.write(User(id.c_str(), pwd.c_str(), name.c_str(), CUSTOMER));
  id_user_.Insert(id, index);
  return SystemLog("", "registered", id.c_str(), 0, 0, "");
}

SystemLog UserManager::Passwd(std::string id, std::string new_pwd, std::string old_pwd) {
  try {
    expect(id).toBeLength(1, 30);
  } catch(...) {
    throw Exception("passwd : invalid id");
  }
  auto user_index = id_user_[id];
  if(user_index.empty()) {
    throw Exception("Passwd : the id is not exist");
  }
  try {
    expect(new_pwd).toBeLength(1, 30).toMatch("^[a-zA-Z0-9_]+$");
  } catch(...) {
    throw Exception("passwd : invalid new_pwd");
  }
  size_t index = user_index[0];
  User user = user_list_[index];
  Session session = GetTopSession();
  User current_user = user_list_[session.index_user_];

  if (old_pwd == "") {
    if (current_user.privilege_ == ADMIN) {
      strncpy(user.password_, new_pwd.c_str(), new_pwd.length());
      user.password_[new_pwd.length()] = '\0';
      user_list_.update(user, index);
      return SystemLog(current_user.userid_, "modify the pwd", user.userid_, 0, 0, "");
    } else {
      throw Exception("passwd : nned old pwd");
    }
  } else {
    if (strcmp(old_pwd.c_str(), user.password_) == 0) {
      strncpy(user.password_, new_pwd.c_str(), new_pwd.length());
      user.password_[new_pwd.length()] = '\0';
      user_list_.update(user, index);
      return SystemLog(id.c_str(), "modify the pwd", user.userid_, 0, 0, "");
    } else {
      throw Exception("passwd : pwd is wrong");
    }
  }
}

SystemLog UserManager::UserAdd(std::string id, std::string pwd, int privilege, std::string name) {
  try {
    expect(id).toBeLength(1, 30).toMatch("^[a-zA-Z0-9_]+$");
  } catch(...) {
    throw Exception("useradd : invalid id");
  }
  try {
    expect(pwd).toBeLength(1, 30).toMatch("^[a-zA-Z0-9_]+$");
  } catch(...) {
    throw Exception("useradd : invalid pwd");
  }
  try {
    expect(name).toBeLength(1, 30).toMatch("^[\\x21-\\x7E]+$");
  } catch(...) {
    throw Exception("useradd : invalid name");
  }
  try {
    expect(privilege).toBeOneOf(VISITOR, CUSTOMER, STAFF);
  } catch(...) {
    throw Exception("useradd : invalid priviledge");
  }
  auto user_index = id_user_[id];
  if(!user_index.empty()) {
    throw Exception("userAdd : the id is used");
  }
  Session session = GetTopSession();
  User current_user = user_list_[session.index_user_];
  if(current_user.privilege_ <= privilege) {
    std::stringstream ss;
    ss << "UserAdd : current user : " << current_user.userid_ << " don't have enough privilege : " << current_user.privilege_ << " to add this privilege user";
    throw Exception(ss.str());
  }
  int index = user_list_.write(User(id.c_str(), pwd.c_str(), name.c_str(), privilege));
  id_user_.Insert(id, index);
  return SystemLog(current_user.userid_, "add the user", id.c_str(), 0, 0, "");
}

SystemLog UserManager::Delete(std::string id) {
  try {
    expect(id).toBeLength(1, 30).toMatch("^[a-zA-Z0-9_]+$");
  } catch(...){
    throw Exception("delete : invalid id");
  }
  auto user_index = id_user_[id];
  if(user_index.empty()) {
    throw Exception("Delete : the id is not exist");
  }
  size_t index = user_index[0];
  for(auto session : log_stack_) {
    if(session.index_user_ == index) {
      throw Exception("Delete : the id is in log");
    }
  }
  user_list_.Delete(index);
  id_user_.Delete(id, index);
  Session session = GetTopSession();
  return SystemLog(user_list_[session.index_user_].userid_, "delete the user", id.c_str(), 0, 0, "");
}

void UserManager::SelectBook(size_t index) {
  log_stack_.back().index_book_ = index;
}

BookManager::BookManager(JournalManager &jm):book_list_(jm, "book.dat"), isbn_book_(jm, "book_isbn.dat"), name_book_(jm, "book_name.dat"), author_book_(jm, "book_author.dat"), key_book_(jm, "book_key.dat") {}

int BookManager::UnrollIsbn(std::string isbn) {
  try {
    expect(isbn).toBeLength(1, 20).toMatch("^[\\x21-\\x7E]+$");
  } catch(...) {
    throw Exception("modify : invalid isbn");
  }
  auto book_index = isbn_book_[isbn];
  if (book_index.empty()) {
    int index = book_list_.write(Book(isbn.c_str()));
    isbn_book_.Insert(isbn, index);
    return index;
  }
  else {
    return book_index[0];
  }
}

SystemLog BookManager::Import(size_t index, int quantity) {
  Book book = book_list_[index];
  book.quantity_ += quantity;
  book_list_.update(book, index);
  return SystemLog("", "import", book.isbn_, quantity, 0, "");
  
}

SystemLog BookManager::Modify(size_t index, const std::string modify[]) {
  Book book = book_list_[index];
  if (modify[0] != "") {
    std::string new_isbn = modify[0];
    try {
      expect(new_isbn).toBeLength(1, 20).toMatch("^[\\x21-\\x7E]+$");
    } catch(...) {
      throw Exception("modify : invalid isbn");
    }
    if (book.isbn_ == new_isbn) {
      throw Exception("modify : the isbn is the same");
    }
    auto book_index = isbn_book_[new_isbn];
    if(!book_index.empty()) {
      throw Exception("modify : the isbn is used");
    }
  }
  if (modify[1] != "") {
    std::string new_name = modify[1];
    try {
      expect(new_name).toBeLength(1, 60).toMatch("^[\\x21\\x23-\\x7E]+$");
    } catch(...) {
      throw Exception("modify : invalid bookname");
    }
  }
  if (modify[2] != "") {
    std::string new_author = modify[2];
    try {
      expect(new_author).toBeLength(1, 60).toMatch("^[\\x21\\x23-\\x7E]+$");
    } catch(...) {
      throw Exception("modify : invalid authorname");
    }
  }
  if (modify[3] != "") {
    std::string new_keyword = modify[3];
    try {
      expect(new_keyword).toBeLength(1, 60).toMatch("^[\\x21\\x23-\\x7E]+$");
    } catch(...) {
      throw Exception("modify : invalid keyword");
    }
    std::string key;
    std::stringstream ss_new(new_keyword);
    std::set<std::string> key_set;
    while (getline(ss_new, key, '|')) {
      if(key.empty()) {
        throw Exception("modify : blank key");
      }
      if(key_set.count(key)) {
        throw Exception("modfiy : have same keyword");
      }
      key_set.insert(key);
    }
  }
  if (modify[4] != "") {
    double price;
    size_t pos;
    try {
      price = std::stod(modify[4], &pos);
    } catch(...) {
      throw Exception("modify : price need to be num");
    }
    if(pos != modify[4].length()) {
      throw Exception("modify : price need to be num");
    }
    if(price < -1e-7) {
      throw Exception("modify : price need to be postive");
    }
  }
  if(modify[0] != "") {
    std::string new_isbn = modify[0];
    isbn_book_.Delete(book.isbn_, index);
    strncpy(book.isbn_, new_isbn.c_str(), new_isbn.length());
    book.isbn_[new_isbn.length()] = '\0';
    isbn_book_.Insert(new_isbn, index);
  }
  if(modify[1] != "") {
    std::string new_name = modify[1];
    name_book_.Delete(book.bookname_, index);
    //std::cerr << new_name << std::endl;
    strncpy(book.bookname_, new_name.c_str(), new_name.length());
    book.bookname_[new_name.length()] = '\0';
    name_book_.Insert(new_name, index);
  }
  if(modify[2] != "") {
    std::string new_author = modify[2];
    author_book_.Delete(book.author_, index);
    //std::cerr << new_author << std::endl;
    strncpy(book.author_, new_author.c_str(), new_author.length());
    book.author_[new_author.length()] = '\0';
    author_book_.Insert(new_author, index);
  }
  if(modify[3] != "") {
    std::string new_keyword = modify[3];
    std::stringstream ss_old(std::string(book.keyword_));
    std::string key;
    while (getline(ss_old, key, '|')) {
      key_book_.Delete(key, index);
    }
    std::stringstream ss_new(new_keyword);
    std::set<std::string> key_set;
    while (getline(ss_new, key, '|')) {
      key_set.insert(key);
      key_book_.Insert(key, index);
    }
    strncpy(book.keyword_, new_keyword.c_str(), new_keyword.length());
    book.keyword_[new_keyword.length()] = '\0';
  }
  if(modify[4] != "") {
    double price = std::stod(modify[4]);
    book.price_ = price;
  }
  book_list_.update(book, index);
  return SystemLog("", "modify", book.isbn_, 0, 0, "");
}

void BookManager::Show(const std::string show[]) {
  //std::cerr << "show" << std::endl;
  if (show[0] != "") {
    try {
      expect(show[0]).toBeLength(1, 20).toMatch("^[\\x21-\\x7E]+$");
    } catch(...) {
      throw Exception("modify : invalid isbn");
    }
    auto book_index = isbn_book_[show[0]];
    if(book_index.empty()) {
      printf("\n");
      return ;
    }
    Book book = book_list_[book_index[0]];
    printf("%s\t%s\t%s\t%s\t%.2lf\t%lld\n", book.isbn_, book.bookname_, book.author_, book.keyword_, book.price_, book.quantity_);
  } else if (show[1] != "") {
    //std::cerr << show[1] << std::endl;
    try {
      expect(show[1]).toBeLength(1, 60).toMatch("^[\\x21\\x23-\\x7E]+$");
    } catch(...) {
      throw Exception("modify : invalid bookname");
    }
    auto book_index = name_book_[show[1]];
    std::vector<Book> book_list;
    for (auto it : book_index) {
      book_list.push_back(book_list_[it]);
    }
    sort(book_list.begin(), book_list.end(), [](Book &book1, Book &book2) {return strcmp(book1.isbn_, book2.isbn_) < 0;});
    if(book_list.empty()) {
      printf("\n");
    }
    for (auto book : book_list) {
      printf("%s\t%s\t%s\t%s\t%.2lf\t%lld\n", book.isbn_, book.bookname_, book.author_, book.keyword_, book.price_, book.quantity_);
    }
  } else if (show[2] != "") {
    try {
      expect(show[2]).toBeLength(1, 60).toMatch("^[\\x21\\x23-\\x7E]+$");
    } catch(...) {
      throw Exception("modify : invalid authorname");
    }
    auto book_index = author_book_[show[2]];
    std::vector<Book> book_list;
    for (auto it : book_index) {
      book_list.push_back(book_list_[it]);
    }
    sort(book_list.begin(), book_list.end(), [](Book &book1, Book &book2) {return strcmp(book1.isbn_, book2.isbn_) < 0;});
    if(book_list.empty()) {
      printf("\n");
    }
    for (auto book : book_list) {
      printf("%s\t%s\t%s\t%s\t%.2lf\t%lld\n", book.isbn_, book.bookname_, book.author_, book.keyword_, book.price_, book.quantity_);
    }
  } else if (show[3] != "") {
    try {
      expect(show[3]).toBeLength(1, 60).toMatch("^[\\x21\\x23-\\x7E]+$");
    } catch(...) {
      throw Exception("modify : invalid keyword");
    }
    auto book_index = key_book_[show[3]];
    std::vector<Book> book_list;
    for (auto it : book_index) {
      book_list.push_back(book_list_[it]);
    }
    sort(book_list.begin(), book_list.end(), [](Book &book1, Book &book2) {return strcmp(book1.isbn_, book2.isbn_) < 0;});
    if(book_list.empty()) {
      printf("\n");
    }
    for (auto book : book_list) {
      printf("%s\t%s\t%s\t%s\t%.2lf\t%lld\n", book.isbn_, book.bookname_, book.author_, book.keyword_, book.price_, book.quantity_);
    }
  } else {
    bool have_book = 0;
    auto PrintBook = [&, this](int index) {
      have_book = 1;
      Book book = book_list_[index];
      printf("%s\t%s\t%s\t%s\t%.2lf\t%lld\n", book.isbn_, book.bookname_, book.author_, book.keyword_, book.price_, book.quantity_);
    };
    isbn_book_.traverse(PrintBook);
    if(!have_book) {
      printf("\n");
    }
  }
}

SystemLog BookManager::Buy(std::string isbn, long long quantity) {
  try {
    expect(isbn).toBeLength(1, 20).toMatch("^[\\x21-\\x7E]+$");
  } catch(...) {
    throw Exception("buy : invalid isbn");
  }
  auto book_index = isbn_book_[isbn];
  if(book_index.empty()) {
    throw Exception("buy : don't have this book");
  }
  int index = book_index[0];
  Book book = book_list_[index];
  if(quantity > book.quantity_) {
    throw Exception("buy : we don't have enough book");
  }
  book.quantity_ -= quantity;
  printf("%0.2lf\n", quantity * book.price_);
  book_list_.update(book, index);
  return SystemLog("", "buy", isbn.c_str(), quantity, quantity * book.price_, "");
}

LogManager::LogManager(JournalManager &jm):finance_log_(jm, "finance.log"), system_log(jm, "system.log") {
  if(finance_log_.size() == 0) {
    finance_log_.write(FinanceLog(0, 0));
  }
  
}

void LogManager::AddFinancialLog(double amount) {
  FinanceLog last_log = finance_log_[finance_log_.size() - 1];
  if (amount > 0) {
    finance_log_.write(FinanceLog(last_log.positive_amount_ + amount, last_log.minus_amount_));
  } else {
    finance_log_.write(FinanceLog(last_log.positive_amount_, last_log.minus_amount_ - amount));
  }
}

void LogManager::ShowFinance(int count) {
  FinanceLog final_log = finance_log_[finance_log_.size() - 1];
  if(count == -1) {
    printf("+ %.2lf - %.2lf\n", final_log.positive_amount_, final_log.minus_amount_);
    return ;
  } 
  if(count == 0) {
    printf("\n");
    return ;
  }
  if(count >= finance_log_.size()) {
    throw Exception("showfinance : the count should less than max_count");
  }
  FinanceLog past_log = finance_log_[finance_log_.size() - count - 1];
  printf("+ %.2lf - %.2lf\n", final_log.positive_amount_ - past_log.positive_amount_, final_log.minus_amount_ - past_log.minus_amount_);
}

void LogManager::AddSystemLog(SystemLog &log) {
  system_log.write(log);
}

void LogManager::PrintLog() {
  printf("%-10s %-20s %-10s %-10s %s\n", "User", "Action", "Target", "Money", "Details");
  auto PrintLog = [this](SystemLog &log) {
    printf("%-10s %-20s %-10s %-10.2lf %s\n", log.userid_, log.action_, log.target_, log.total_amount_, log.info_);
  };
  system_log.tranverse(PrintLog);
}