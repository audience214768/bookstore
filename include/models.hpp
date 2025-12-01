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

#endif