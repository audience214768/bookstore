#ifndef BPT_MEMORYRIVER_HPP
#define BPT_MEMORYRIVER_HPP

#include "cache.hpp"
#include "journal.hpp"
#include <fstream>
#include <functional>
#include <iostream>

using std::fstream;
using std::ifstream;
using std::ofstream;
using std::string;



template <class T, int info_len = 2> //info 1 : the first alloc block info 2 : the first free block
class MemoryRiver {
private:
  /* your code here */
  fstream file_;
  string file_name_;
  int sizeofT = sizeof(T);
  Cache<int, T> cache;
  JournalManager &jm_;
  int free_head = -1;
  int last_index = 0;
  int file_id_;
public:
  MemoryRiver() = default;

  MemoryRiver(JournalManager &jm, const string &file_name) : file_name_(file_name), jm_(jm) {
    file_.open(file_name, std::ios::in | std::ios::out | std::ios::binary);
    if(!file_) {
      file_.open(file_name, std::ios::out | std::ios::binary);
      int tmp = -1;
      for (int i = 0; i < info_len; ++i)
        file_.write(reinterpret_cast<char *>(&tmp), sizeof(int));
      file_.close();
      file_.open(file_name, std::ios::in | std::ios::out | std::ios::binary);
    } else {
      get_info(free_head, 2);
    }
    auto write = [this](int offset, int size, const char *data) {
      file_.seekp(offset);
      file_.write(data, size);
    };
    auto refresh = [this]() {
      file_.seekg(0, std::ios::end);
      int offset = file_.tellg();
      last_index = (offset - sizeof(int) * info_len) / sizeofT;
      cache.Clear();
    };
    file_id_ = jm.Init(write, refresh);
    //std::cerr << file_name << " " << file_id_ << std::endl;
    file_.seekg(0, std::ios::end);
    int offset = file_.tellg();
    last_index = (offset - sizeof(int) * info_len) / sizeofT;
    //std::cerr << last_index << std::endl;
  }

  ~MemoryRiver() {
    //std::cerr << 1 << std::endl;
    auto write = [this](const int &index, const T &data) {
      //std::cerr << index << std::endl;
      int offset = sizeofT * index + info_len * sizeof(int);
      file_.seekp(offset);
      file_.write(reinterpret_cast<const char *>(&data), sizeofT);
    };
    cache.FlushAll(write);
    write_info(free_head, 2);
    file_.flush();
    if(file_.is_open()) {
      file_.close();
    }
  }

  int size() {
    return last_index;
  }

  // 读出第n个int的值赋给tmp，1_base
  void get_info(int &tmp, int n) {
    if (n > info_len)
      return;
    /* your code here */
    file_.seekg((n - 1) * sizeof(int));
    file_.read(reinterpret_cast<char *>(&tmp), sizeof(int));
  }

  // 将tmp写入第n个int的位置，1_base
  void write_info(int tmp, int n) {
    if (n > info_len)
      return;
    /* your code here */
    file_.seekp((n - 1) * sizeof(int));
    file_.write(reinterpret_cast<char *>(&tmp), sizeof(int));
  }

  // 在文件合适位置写入类对象t，并返回写入的位置索引index
  // 位置索引意味着当输入正确的位置索引index，在以下三个函数中都能顺利的找到目标对象进行操作
  // 位置索引index可以取为对象写入的起始位置
  int write(const T &t) { 
    /* your code here */
    if(free_head == -1) {
      jm_.Write(file_id_, last_index * sizeofT + info_len * sizeof(int), sizeofT, reinterpret_cast<const char *>(&t));
      auto ret = cache.Put(last_index, t, 1);
      if(ret.first && ret.second.is_dirty_) {
        size_t offset = sizeof(int) * info_len + sizeofT * ret.second.key_;
        file_.seekp(offset);
        file_.write(reinterpret_cast<const char *>(&ret.second.value_), sizeofT);
      }
      //std::cerr << last_index << std::endl;
      return last_index++;
    }
    size_t offset = free_head * sizeofT + info_len * sizeof(int);
    T temp;
    read(temp, free_head);
    int new_head;
    memcpy(&new_head, &temp, sizeof(int));
    //std::cerr << sizeofT << std::endl;
    jm_.Write(file_id_, offset, sizeofT, reinterpret_cast<const char *>(&t));
    auto ret = cache.Put(free_head, t, 1);
    int index = free_head;
    jm_.Write(file_id_, sizeof(int), sizeof(int), reinterpret_cast<const char *>(&new_head));
    free_head = new_head;
    if(ret.first && ret.second.is_dirty_) {
      size_t offset = sizeof(int) * info_len + sizeofT * ret.second.key_;
      file_.seekp(offset);
      file_.write(reinterpret_cast<const char *>(&ret.second.value_), sizeofT);
    }
    return index;
  }

  // 用t的值更新位置索引index对应的对象，保证调用的index都是由write函数产生
  void update(const T &t, int index) { 
    /* your code here */ 
    //std::cerr << index << std::endl;
    jm_.Write(file_id_, sizeof(int) * info_len + sizeofT * index, sizeofT, reinterpret_cast<const char *>(&t));
    auto ret = cache.Put(index, t);
    if(ret.first && ret.second.is_dirty_) {
      size_t offset = sizeof(int) * info_len + sizeofT * ret.second.key_;
      file_.seekp(offset);
      file_.write(reinterpret_cast<const char *>(&ret.second.value_), sizeofT);
    }
  }

  // 读出位置索引index对应的T对象的值并赋值给t，保证调用的index都是由write函数产生
  void read(T &t, int index) { 
    /* your code here */ 
    auto node = cache.Get(index);
    if(node) {
      //std::cerr << "cache hit" << std::endl;
      t = *node;
      return ;
    }
    //std::cerr << "cache miss" << std::endl;
    size_t offset = sizeof(int) * info_len + sizeofT * index;
    file_.seekg(offset);
    file_.read(reinterpret_cast<char *>(&t), sizeofT);
    auto ret = cache.Put(index, t, false);
    if(ret.first && ret.second.is_dirty_) {
      size_t offset = sizeof(int) * info_len + sizeofT * ret.second.key_;
      file_.seekp(offset);
      file_.write(reinterpret_cast<const char *>(&ret.second.value_), sizeofT);
    }
  }

  // 删除位置索引index对应的对象(不涉及空间回收时，可忽略此函数)，保证调用的index都是由write函数产生
  void Delete(int index) { 
    /* your code here */ 
    T temp;
    memcpy(&temp, &free_head, sizeof(int));
    update(temp, index);
    jm_.Write(file_id_, sizeof(int), sizeof(int), reinterpret_cast<const char *>(&index));
    free_head = index;
  }
  T operator[](int index) {
    T t;
    read(t, index);
    return t;
  }
  void tranverse(std::function<void(T &)> callback) {
    //std::cerr << count << std::endl;
    int count = size();
    for(int i = 0; i < count; i++) {
      T t;
      //std::cerr << i << std::endl;
      read(t, i);
      callback(t);
    }
  }
};

#endif // BPT_MEMORYRIVER_HPP