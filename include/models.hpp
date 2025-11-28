#ifndef MODEL
#define MODEL

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
  int quantity_ = 0;
  double price_ = 0;
  double total_cast_ = 0;
  Book();
  Book(const char *, const char *, const char *, int, double, double);
};

struct FinancialLog {
  double amount_;
};

struct SystemLog {
  char userid_[30];
  char action_[20];
  char target[30];
  int quantity = 0;
  double price_ = 0;
  char info[20];
};

#endif