#ifndef COMMAND
#define COMMAND

#include "manager.hpp"
#include <vector>
#include <string>

class Command {
protected:
  static UserManager *user_manager_;
  static BookManager *book_manager_;
public:
  static void init(UserManager *, BookManager *);
  virtual const char *Name() const = 0;
  virtual int NeedPrivilege() const = 0;
  virtual void Execute(const std::vector<std::string> &) = 0;
  void run(const std::vector<std::string> &);
};

class Exit : public Command { // both for exit and quit
public:
  const char *Name() const override;
  int NeedPrivilege() const override;
  void Execute(const std::vector<std::string> &) override;
};

class Login : public Command {
public:
  const char *Name() const override;
  int NeedPrivilege() const override;
  void Execute(const std::vector<std::string> &) override;
};



class Logout : public Command {
public:
  const char *Name() const override;
  int NeedPrivilege() const override;
  void Execute(const std::vector<std::string> &) override;
};

class Register : public Command { 
public:
  const char *Name() const override;
  int NeedPrivilege() const override;
  void Execute(const std::vector<std::string> &) override;
};

class UserAdd : public Command { 
public:
  const char *Name() const override;
  int NeedPrivilege() const override;
  void Execute(const std::vector<std::string> &) override;
};

class Passwd : public Command {
public:
  const char *Name() const override;
  int NeedPrivilege() const override;
  void Execute(const std::vector<std::string> &) override;
};

class DeleteUser : public Command {
public:
  const char *Name() const override;
  int NeedPrivilege() const override;
  void Execute(const std::vector<std::string> &) override;
};

class ShowBook : public Command {
public:
  const char *Name() const override;
  int NeedPrivilege() const override;
  void Execute(const std::vector<std::string> &) override;
};

class BuyBook : public Command {
public:
  const char *Name() const override;
  int NeedPrivilege() const override;
  void Execute(const std::vector<std::string> &) override;
};

class SelectBook : public Command {
public:
  const char *Name() const override;
  int NeedPrivilege() const override;
  void Execute(const std::vector<std::string> &) override;
};

class ModifyBook : public Command {
public:
  const char *Name() const override;
  int NeedPrivilege() const override;
  void Execute(const std::vector<std::string> &) override;
};

class ImportBook : public Command {
public:
  const char *Name() const override;
  int NeedPrivilege() const override;
  void Execute(const std::vector<std::string> &) override;
};

class ShowFinance : public Command {
public:
  const char *Name() const override;
  int NeedPrivilege() const override;
  void Execute(const std::vector<std::string> &) override;
};

class ShowLog : public Command {
public:
  const char *Name() const override;
  int NeedPrivilege() const override;
  void Execute(const std::vector<std::string> &) override;
};

class ReportFinance : public Command {
public:
  const char *Name() const override;
  int NeedPrivilege() const override;
  void Execute(const std::vector<std::string> &) override;
};

class ReportEmployee : public Command {
public:
  const char *Name() const override;
  int NeedPrivilege() const override;
  void Execute(const std::vector<std::string> &) override;
};

#endif