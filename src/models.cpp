#include "models.hpp"
#include <cstring>

User::User(const char * id, const char *pwd, const char *name, const int privilege):privilege_(privilege) {
  strcpy(userid_, id);
  strcpy(password_, pwd);
  strcpy(username_, name);
}