#include "journal.hpp"


void JournalManager::AddLog(const JournalHeader &header, const void *data) {
  file_.seekp(0, std::ios::end);
  file_.write(reinterpret_cast<const char *>(&header), sizeofHead);
  if(data) {
    file_.write(reinterpret_cast<const char *>(data), header.data_size);
  }
}

JournalManager::JournalManager(const std::string &file_name) {
  file_.open(file_name, std::ios::in | std::ios::out | std::ios::binary);
  if(!file_) {
    file_.open(file_name, std::ios::out | std::ios::binary);
    file_.close();
    file_.open(file_name, std::ios::in | std::ios::out | std::ios::binary);
  }
}

void JournalManager::Ready() {
  JournalHeader header = {ready};
  AddLog(header, nullptr);
}

void JournalManager::Write(int fileID, int offset, int data_size, const void *data) {
  JournalHeader header = {write, fileID, offset, data_size};
  AddLog(header, data);
}

void JournalManager::Finish() {
  JournalHeader header = {finish};
  AddLog(header, nullptr);
}





