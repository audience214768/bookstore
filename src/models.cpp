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


FinancialLog::FinancialLog(double postive_amount, double minus_amount):postive_amount_(postive_amount), minus_amount_(minus_amount) {}