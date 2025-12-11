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
  fstream file;
  string file_name;
  int count = 0;
  int sizeofT = sizeof(T);
  Cache<int, T> cache;
public:
  MemoryRiver() = default;

  MemoryRiver(const string &file_name) : file_name(file_name) {
    file.open(file_name, std::ios::in | std::ios::out | std::ios::binary);
    if(!file) {
      file.open(file_name, std::ios::out | std::ios::binary);
      int tmp = -1;
      for (int i = 0; i < info_len; ++i)
        file.write(reinterpret_cast<char *>(&tmp), sizeof(int));
      file.close();
      file.open(file_name, std::ios::in | std::ios::out | std::ios::binary);
    }
  }

  ~MemoryRiver() {
    //std::cerr << 1 << std::endl;
    auto write = [this](const int &index, const T &data) {
      //std::cerr << index << std::endl;
      int offset = sizeofT * index + info_len * sizeof(int);
      file.seekp(offset);
      file.write(reinterpret_cast<const char *>(&data), sizeofT);
    };
    cache.FlushAll(write);
    file.flush();
    if(file.is_open()) {
      file.close();
    }
  }

  int size() {
    return count;
  }

  // 读出第n个int的值赋给tmp，1_base
  void get_info(int &tmp, int n) {
    if (n > info_len)
      return;
    /* your code here */
    file.seekg((n - 1) * sizeof(int));
    file.read(reinterpret_cast<char *>(&tmp), sizeof(int));
  }

  // 将tmp写入第n个int的位置，1_base
  void write_info(int tmp, int n) {
    if (n > info_len)
      return;
    /* your code here */
    file.seekp((n - 1) * sizeof(int));
    file.write(reinterpret_cast<char *>(&tmp), sizeof(int));
  }

  // 在文件合适位置写入类对象t，并返回写入的位置索引index
  // 位置索引意味着当输入正确的位置索引index，在以下三个函数中都能顺利的找到目标对象进行操作
  // 位置索引index可以取为对象写入的起始位置
  int write(const T &t) { 
    /* your code here */
    int head;
    get_info(head, 2);
    if(head == -1) {
      file.seekp(0, std::ios::end);
      int size = file.tellp();
      file.write(reinterpret_cast<const char *>(&t), sizeofT);
      count ++;
      int index = (size - info_len * sizeof(int)) / sizeofT;
      auto ret = cache.Put(index, t, 0);
      if(ret.first && ret.second.is_dirty_) {
        size_t offset = sizeof(int) * info_len + sizeofT * ret.second.key_;
        file.seekp(offset);
        file.write(reinterpret_cast<const char *>(&ret.second.value_), sizeofT);
      }
      return index;
    }
    size_t offset = head * sizeofT + info_len * sizeof(int);
    file.seekp(offset);
    int new_head;
    file.read(reinterpret_cast<char *>(&new_head), sizeof(int));
    write_info(new_head, 2);
    int size = file.tellp();
    file.seekp(offset);
    file.write(reinterpret_cast<const char *>(&t), sizeofT);
    count++;
    auto ret = cache.Put(head, t, 0);
    if(ret.first && ret.second.is_dirty_) {
      size_t offset = sizeof(int) * info_len + sizeofT * ret.second.key_;
      file.seekp(offset);
      file.write(reinterpret_cast<const char *>(&ret.second.value_), sizeofT);
    }
    return head;
  }

  // 用t的值更新位置索引index对应的对象，保证调用的index都是由write函数产生
  void update(const T &t, int index) { 
    /* your code here */ 
    //std::cerr << index << std::endl;
    auto ret = cache.Put(index, t);
    if(ret.first && ret.second.is_dirty_) {
      size_t offset = sizeof(int) * info_len + sizeofT * ret.second.key_;
      file.seekp(offset);
      file.write(reinterpret_cast<const char *>(&ret.second.value_), sizeofT);
    }
    /*
    size_t offset = sizeof(int) * info_len + sizeofT * index;
    file.seekp(offset);
    file.write(reinterpret_cast<const char *>(&t), sizeofT);*/
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
    file.seekg(offset);
    file.read(reinterpret_cast<char *>(&t), sizeofT);
    auto ret = cache.Put(index, t, false);
    if(ret.first && ret.second.is_dirty_) {
      size_t offset = sizeof(int) * info_len + sizeofT * ret.second.key_;
      file.seekp(offset);
      file.write(reinterpret_cast<const char *>(&ret.second.value_), sizeofT);
    }
  }

  // 删除位置索引index对应的对象(不涉及空间回收时，可忽略此函数)，保证调用的index都是由write函数产生
  void Delete(int index) { 
    /* your code here */ 
    size_t offset = index * sizeofT + info_len * sizeof(int);
    int head;
    get_info(head, 2);
    file.seekp(offset);
    file.write(reinterpret_cast<char *>(&head), sizeof(int));
    write_info(index, 2);
    count--;
  }
  T operator[](int index) {
    T t;
    read(t, index);
    return t;
  }
  void tranverse(std::function<void(T &)> callback) {
    //std::cerr << count << std::endl;
    for(int i = 0; i < count; i++) {
      T t;
      //std::cerr << i << std::endl;
      read(t, i);
      callback(t);
    }
  }
};

#endif // BPT_MEMORYRIVER_HPP