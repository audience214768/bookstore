#include "journal.hpp"
#include <fstream>


void JournalManager::AddLog(const JournalHeader &header, const char *data) {
  const char *ptr = reinterpret_cast<const char *>(&header);
  buffer_.insert(buffer_.end(), ptr, ptr + sizeofHead);
  if(data != nullptr) {
    buffer_.insert(buffer_.end(), data, data + header.data_size);
  }
}

JournalManager::JournalManager(const std::string &file_name):file_name_(file_name) {
  std::fstream file_;
  file_.open(file_name, std::ios::in | std::ios::out | std::ios::binary);
  if(!file_) {
    file_.open(file_name, std::ios::out | std::ios::binary);
  } 
  file_.close();
}

int JournalManager::Init(rawWrite func) {
  file_map_.push_back(func);
  return file_map_.size() - 1;
}

void JournalManager::Ready() {
  JournalHeader header = {READY};
  AddLog(header, nullptr);
}

void JournalManager::Write(int fileID, int offset, int data_size, const char *data) {
  JournalHeader header = {WRITE, fileID, offset, data_size};
  AddLog(header, data);
}

void JournalManager::Finish() {
  JournalHeader header = {FINISH};
  AddLog(header, nullptr);
  std::fstream file(file_name_, std::ios::out | std::ios::binary | std::ios::app);
  file.write(buffer_.data(), buffer_.size());
  buffer_.clear();
  file.flush();
  file.close();
}

void JournalManager::Recover() {
  std::fstream file(file_name_, std::ios::in | std::ios::out | std::ios::binary);
  std::vector<Data> buffer;
  bool dataloss = 0;
  while(file.peek() != EOF) {
    JournalHeader header;
    file.read(reinterpret_cast<char *>(&header), sizeofHead);
    if(header.type == READY) {
      if(!buffer.empty()) {
        dataloss = 1;
        std::cerr << "something is lost" << std::endl;
      }
      buffer.clear();
    }
    if(header.type == WRITE) {
      std::vector<char> data(header.data_size);
      file.read(data.data(), header.data_size);
      buffer.push_back(Data{header.fileID, header.offset, header.data_size, data});
    }
    if(header.type == FINISH) {
      for(auto redo : buffer) {
        file_map_[redo.fileID](redo.offset, redo.data_size, redo.buffer.data());
      }
      buffer.clear();
    }
  }
  file.close();
  std::fstream claer_file(file_name_, std::ios::out | std::ios::trunc);
  if(!dataloss) {
    std::cerr << "data is fine" << std::endl;
  }
}





