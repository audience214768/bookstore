#include "models.hpp"
#include <cstring>

User::User(const char * id, const char *pwd, const char *name, const int privilege):privilege_(privilege) {
  strcpy(userid_, id);
  strcpy(password_, pwd);
  strcpy(username_, name);
}

Book::Book(const char *isbn):bookname_(), author_(), keyword_() {
  strcpy(isbn_, isbn);
}


FinanceLog::FinanceLog(double postive_amount, double minus_amount):positive_amount_(postive_amount), minus_amount_(minus_amount) {}

SystemLog::SystemLog(const char *id, const char *action, const char *tar, int quantity, double amount, const char *info):quantity_(quantity), total_amount_(amount) {
  strcpy(userid_, id);
  strcpy(action_, action);
  strcpy(target_, tar);
  strcpy(info_, info);
}