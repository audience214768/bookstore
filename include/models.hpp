#ifndef MODEL
#define MODEL

#include <iostream>
#include <cstring>
#include <ostream>
#include "utils.hpp"


struct User {
  int privilege_ = 0;
  char userid_[31];
  char password_[31];
  char username_[31];
  User();
  User(const char *, const char *, const char *, const int);
};

struct Book {
  char isbn_[21];
  char bookname_[61];
  char author_[61];
  char keyword_[61];
  int quantity_ = 0;
  double price_ = 0;
  double total_cast_ = 0;
  Book(const char *);
  
};


struct FinancialLog {
  double positive_amount_;
  double minus_amount_;
  FinancialLog(double, double);
};

struct SystemLog {
  SystemLog(const char *, const char *, const char *, int, double, const char *);
  char userid_[31];
  char action_[30];
  char target_[31];
  int quantity_;
  double total_amount_;
  char info_[120];
};

template<int len = 1>
struct FixedString {
  char str[len];
  FixedString() {
    memset(str, 0, len);
  }
  FixedString(std::string other) {
    if(other.length() > len) {
      throw Exception("the len of string is bigger than FixedLen");
    }
    strcpy(str, other.c_str());
  }
  FixedString(char other[]) {
    if(strlen(other) > len) {
      throw Exception("the len of char array is bigger than FixedLen");
    }
    strcpy(str, other);
  }
  bool operator<(const FixedString<len> &other) const {
    return strcmp(str, other.str) < 0;
  }
  bool operator>=(const FixedString<len> &other) const {
    return strcmp(str, other.str) >= 0;
  }
  bool operator==(const FixedString<len> &other) const {
    return strcmp(str, other.str) == 0;
  }
  FixedString<len> &operator=(const FixedString<len> &other) {
    strcpy(str, other.str);
    return *this;
  }
};

template<int len = 1>
std::istream &operator>>(std::istream &is, FixedString<len> &fs) {
  is >> fs.str;
  return is;
}

template<int len = 1>
std::ostream &operator<<(std::ostream &os, const FixedString<len> &fs) {
  os << fs.str;
  return os;
}

#endif