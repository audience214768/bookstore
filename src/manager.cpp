#include "manager.hpp"
#include "config.hpp"
#include "utils.hpp"
#include <cstring>
#include <sstream>
#include <iostream>

UserManager::UserManager() {
  user_list_.push_back(User("visitor", "", "virtual", VISITOR));
  user_list_.push_back(User("root", "sjtu", "audience", ADMIN));
  id_user_["root"] = 1;
  log_stack_.push_back(Session(0));
}


const Session &UserManager::GetTopSession() {
  return log_stack_.back();
}

const User UserManager::GetUser(size_t index) {
  return user_list_[index];
}

void UserManager::Login(std::string id, std::string pwd) {
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
}

void UserManager::Logout() {
  if(log_stack_.size() == 1) {
    throw Exception("there is no account now");
  }
  log_stack_.pop_back();
}

void UserManager::Register(std::string id, std::string pwd, std::string name) {
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
}

void UserManager::Passwd(std::string id, std::string new_id, std::string old_id) {
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
  } else {
    if (strcmp(old_id.c_str(), user.password_) == 0) {
      strncpy(user.password_, new_id.c_str(), new_id.length());
      user.password_[new_id.length()] = '\0';
      user_list_[index] = user;
    } else {
      std::stringstream ss;
      ss << "Passwd : pwd is wrong";
      throw Exception(ss.str());
    }
  }
}

void UserManager::UserAdd(std::string id, std::string pwd, int privilege, std::string name) {
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
}

void UserManager::Delete(std::string id) {
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
}