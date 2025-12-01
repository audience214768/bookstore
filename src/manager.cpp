#include "manager.hpp"
#include "config.hpp"
#include "models.hpp"
#include "utils.hpp"
#include <cerrno>
#include <cstddef>
#include <cstdio>
#include <cstring>
#include <sstream>
#include <iostream>
#include <string>
#include <set>
#include <algorithm>

UserManager::UserManager() {
  user_list_.push_back(User("visitor", "", "virtual", VISITOR));
  user_list_.push_back(User("root", "sjtu", "audience", ADMIN));
  id_user_["root"] = 1;
  log_stack_.push_back(Session(0));
}


const Session UserManager::GetTopSession() {
  return log_stack_.back();
}

const User UserManager::GetUser(size_t index) {
  return user_list_[index];
}

SystemLog UserManager::Login(std::string id, std::string pwd) {
  if(id_user_.count(id) == 0) {
    throw Exception("the id is not exist");
  }
  size_t index = id_user_[id];
  User user = user_list_[index];
  Session session = GetTopSession();
  if(user_list_[session.index_user_].privilege_ > user.privilege_) {
    log_stack_.push_back(Session(index));
  } else {
    if (strcmp(pwd.c_str(), user.password_) == 0) {
      log_stack_.push_back(index);
    } else {
      std::stringstream ss;
      ss << "Login : pwd is wrong";
      throw Exception(ss.str());
    }
  }
  //std::cerr << "finish login" << std::endl;
  return SystemLog(id.c_str(), "login", "", 0, 0, "");
}

SystemLog UserManager::Logout() {
  if(log_stack_.size() == 1) {
    throw Exception("there is no account now");
  }
  std::string id = user_list_[log_stack_.back().index_user_].userid_;
  log_stack_.pop_back();
  return SystemLog(id.c_str(), "logout", "", 0, 0, "");
}

SystemLog UserManager::Register(std::string id, std::string pwd, std::string name) {
  if(id_user_.count(id) != 0) {
    throw Exception("Register : the id is used");
  }
  if(id.length() > 30) {
    throw Exception("Register : the length of id should be less than 30");
  }
  if(pwd.length() > 30) {
    throw Exception("Register : the length of pwd should be less than 30");
  }
  if(name.length() > 30) {
    throw Exception("Register : the length of name should be less than 30");
  }
  user_list_.push_back(User(id.c_str(), pwd.c_str(), name.c_str(), CUSTOMER));
  id_user_[id] = user_list_.size() - 1;
  return SystemLog("", "registered", id.c_str(), 0, 0, "");
}

SystemLog UserManager::Passwd(std::string id, std::string new_id, std::string old_id) {
  if(id_user_.count(id) == 0) {
    throw Exception("Passwd : the id is not exist");
  }
  if(new_id.length() > 30) {
    throw Exception("Register : the length of pwd should be less than 30");
  }
  size_t index = id_user_[id];
  User user = user_list_[index];
  Session session = GetTopSession();
  if(user_list_[session.index_user_].privilege_ == ADMIN) {
    strncpy(user.password_, new_id.c_str(), new_id.length());
    user.password_[new_id.length()] = '\0';
    user_list_[index] = user;
    return SystemLog(user_list_[1].userid_, "modify the pwd", user.userid_, 0, 0, "");
  } else {
    if (strcmp(old_id.c_str(), user.password_) == 0) {
      strncpy(user.password_, new_id.c_str(), new_id.length());
      user.password_[new_id.length()] = '\0';
      user_list_[index] = user;
      return SystemLog(id.c_str(), "modify the pwd", user.userid_, 0, 0, "");
    } else {
      std::stringstream ss;
      ss << "Passwd : pwd is wrong";
      throw Exception(ss.str());
    }
  }
}

SystemLog UserManager::UserAdd(std::string id, std::string pwd, int privilege, std::string name) {
  if(id_user_.count(id) != 0) {
    throw Exception("UserAdd : the id is used");
  }
  Session session = GetTopSession();
  User current_user = user_list_[session.index_user_];
  if(current_user.privilege_ <= privilege) {
    std::stringstream ss;
    ss << "UserAdd : current user : " << current_user.userid_ << " don't have enough privilege : " << current_user.privilege_ << " to add this privilege user";
    throw Exception(ss.str());
  }
  if(id.length() > 30) {
    throw Exception("Register : the length of id should be less than 30");
  }
  if(pwd.length() > 30) {
    throw Exception("Register : the length of pwd should be less than 30");
  }
  if(name.length() > 30) {
    throw Exception("Register : the length of name should be less than 30");
  }
  id_user_[id] = user_list_.size();
  user_list_.push_back(User(id.c_str(), pwd.c_str(), name.c_str(), privilege));
  return SystemLog(current_user.userid_, "add the user", id.c_str(), 0, 0, "");
}

SystemLog UserManager::Delete(std::string id) {
  if(id_user_.count(id) == 0) {
    throw Exception("Delete : the id is not exist");
  }
  size_t index = id_user_[id];
  for(auto session : log_stack_) {
    if(session.index_user_ == index) {
      throw Exception("Delete : the id is in log");
    }
  }
  user_list_.erase(user_list_.begin() + index);
  id_user_.erase(id);
  return SystemLog(user_list_[1].userid_, "delete the user", id.c_str(), 0, 0, "");
}

void UserManager::SelectBook(size_t index) {
  log_stack_.back().index_book_ = index;
}

size_t BookManager::UnrollIsbn(std::string isbn) {
    if(isbn_book_.count(isbn) == 0) {
      isbn_book_[isbn] = book_list_.size();
      book_list_.push_back(Book(isbn.c_str()));
      return book_list_.size() - 1;
    } else {
      return isbn_book_[isbn];
    }
}

SystemLog BookManager::Import(size_t index, int quantity) {
  Book book = book_list_[index];
  book.quantity_ += quantity;
  book_list_[index] = book;
  return SystemLog("", "import", book.isbn_, quantity, 0, "");
}

SystemLog BookManager::Modify(size_t index, const std::string modify[]) {
  Book book = book_list_[index];
  if (modify[0] != "") {
    std::string new_isbn = modify[0];
    if (book.isbn_ == new_isbn) {
      throw Exception("modify : the isbn is the same");
    }
    isbn_book_.erase(std::string(book.isbn_));
    strncpy(book.isbn_, new_isbn.c_str(), new_isbn.length());
    book.isbn_[new_isbn.length()] = '\0';
    isbn_book_[new_isbn] = index;
  }
  if (modify[1] != "") {
    std::string new_name = modify[1];
    auto range = name_book_.equal_range(std::string(book.bookname_));
    for (auto it = range.first; it != range.second; it++) {
      if (it->second == index) {
        name_book_.erase(it);
        break;
      }
    }
    //std::cerr << new_name << std::endl;
    strncpy(book.bookname_, new_name.c_str(), new_name.length());
    book.bookname_[new_name.length()] = '\0';
    name_book_.insert({new_name, index});
  }
  if (modify[2] != "") {
    std::string new_author = modify[2];
    auto range = author_book_.equal_range(std::string(book.author_));
    for (auto it = range.first; it != range.second; it++) {
      if (it->second == index) {
        author_book_.erase(it);
        break;
      }
    }
    //std::cerr << new_author << std::endl;
    strncpy(book.author_, new_author.c_str(), new_author.length());
    book.author_[new_author.length()] = '\0';
    author_book_.insert({new_author, index});
  }
  if (modify[3] != "") {
    std::string new_keyword = modify[3];
    std::stringstream ss_old(std::string(book.keyword_));
    std::string key;
    while (getline(ss_old, key, '|')) {
      auto range = key_book_.equal_range(key);
      for (auto it = range.first; it != range.second; it++) {
        if (it->second == index) {
          key_book_.erase(it);
          break;
        }
      }
    }
    std::stringstream ss_new(new_keyword);
    std::set<std::string> key_set;
    while (getline(ss_new, key, '|')) {
      if(key_set.count(key)) {
        throw Exception("modfiy : have same keyword");
      }
      key_set.insert(key);
      key_book_.insert({key, index});
    }
    strncpy(book.keyword_, new_keyword.c_str(), new_keyword.length());
    book.keyword_[new_keyword.length()] = '\0';
  }
  if (modify[4] != "") {
    double price = std::stod(modify[4]);
    book.price_ = price;
  }
  book_list_[index] = book;
  return SystemLog("", "modify", book.isbn_, 0, 0, "");
}

void BookManager::Show(const std::string show[]) {
  //std::cerr << "show" << std::endl;
  if (show[0] != "") {
    if(isbn_book_.count(show[0]) == 0) {
      printf("\n");
      return ;
    }
    Book book = book_list_[isbn_book_[show[0]]];
    printf("%s\t%s\t%s\t%s\t%.2lf\t%d\n", book.isbn_, book.bookname_, book.author_, book.keyword_, book.price_, book.quantity_);
  } else if (show[1] != "") {
    //std::cerr << show[1] << std::endl;
    auto range = name_book_.equal_range(show[1]);
    std::vector<Book> book_list;
    for (auto it = range.first; it != range.second; it++) {
      book_list.push_back(book_list_[it->second]);
    }
    sort(book_list.begin(), book_list.end(), [](Book &book1, Book &book2) {return book1.isbn_ < book2.isbn_;});
    if(book_list.empty()) {
      printf("\n");
    }
    for (auto book : book_list) {
      printf("%s\t%s\t%s\t%s\t%.2lf\t%d\n", book.isbn_, book.bookname_, book.author_, book.keyword_, book.price_, book.quantity_);
    }
  } else if (show[2] != "") {
    auto range = author_book_.equal_range(show[2]);
    std::vector<Book> book_list;
    for (auto it = range.first; it != range.second; it++) {
      book_list.push_back(book_list_[it->second]);
    }
    sort(book_list.begin(), book_list.end(), [](Book &book1, Book &book2) {return strcmp(book1.isbn_, book2.isbn_) < 0;});
    if(book_list.empty()) {
      printf("\n");
    }
    for (auto book : book_list) {
      printf("%s\t%s\t%s\t%s\t%.2lf\t%d\n", book.isbn_, book.bookname_, book.author_, book.keyword_, book.price_, book.quantity_);
    }
  } else if (show[3] != "") {
    auto range = key_book_.equal_range(show[3]);
    std::vector<Book> book_list;
    for (auto it = range.first; it != range.second; it++) {
      book_list.push_back(book_list_[it->second]);
    }
    sort(book_list.begin(), book_list.end(), [](Book &book1, Book &book2) {return strcmp(book1.isbn_, book2.isbn_) < 0;});
    if(book_list.empty()) {
      printf("\n");
    }
    for (auto book : book_list) {
      printf("%s\t%s\t%s\t%s\t%.2lf\t%d\n", book.isbn_, book.bookname_, book.author_, book.keyword_, book.price_, book.quantity_);
    }
  } else {
    std::vector<Book> book_list;
    for (auto book : book_list_) {
      book_list.push_back(book);
    }
    sort(book_list.begin(), book_list.end(), [](Book &book1, Book &book2) {return strcmp(book1.isbn_, book2.isbn_) < 0;});
    if(book_list.empty()) {
      printf("\n");
    }
    for (auto book : book_list) {
      printf("%s\t%s\t%s\t%s\t%.2lf\t%d\n", book.isbn_, book.bookname_, book.author_, book.keyword_, book.price_, book.quantity_);
    }
  }
}

SystemLog BookManager::Buy(std::string isbn, int quantity) {
  if(isbn_book_.count(isbn) == 0) {
    throw Exception("buy : don't have this book");
  }
  if(quantity <= 0) {
    throw Exception("buy : you should buy postive num book");
  }
  size_t index = isbn_book_[isbn];
  Book book = book_list_[index];
  if(quantity > book.quantity_) {
    throw Exception("buy : we don't have enough book");
  }
  book.quantity_ -= quantity;
  book_list_[index] = book;
  return SystemLog("", "buy", isbn.c_str(), quantity, quantity * book.price_, "");
}

LogManager::LogManager() {
  financial_log_.push_back(FinancialLog(0, 0));
}

void LogManager::AddFinancialLog(double amount) {
  FinancialLog last_log = financial_log_.back();
  if (amount > 0) {
    financial_log_.push_back(FinancialLog(last_log.positive_amount_ + amount, last_log.minus_amount_));
  } else {
    financial_log_.push_back(FinancialLog(last_log.positive_amount_, last_log.minus_amount_ - amount));
  }
}

void LogManager::ShowFinance(int count) {
  FinancialLog final_log = financial_log_.back();
  if(count == -1) {
    printf("+ %.2lf - %.2lf\n", final_log.positive_amount_, final_log.minus_amount_);
    return ;
  } 
  if(count == 0) {
    printf("\n");
    return ;
  }
  if(count > financial_log_.size()) {
    throw Exception("showfinance : the count should less than max_count");
  }
  FinancialLog past_log = financial_log_[financial_log_.size() - count - 1];
  printf("+ %.2lf - %.2lf\n", final_log.positive_amount_ - past_log.positive_amount_, final_log.minus_amount_ - past_log.minus_amount_);
}

void LogManager::AddSystemLog(SystemLog &log) {
  system_log.push_back(log);
}

void LogManager::PrintLog() {
   printf("%-10s %-20s %-10s %-10s %s\n", "User", "Action", "Target", "Money", "Details");
  for(auto log : system_log) {
    printf("%-10s %-20s %-10s %-10.2lf %s\n", log.userid_, log.action_, log.target_, log.total_amount_, log.info_);
  }
}