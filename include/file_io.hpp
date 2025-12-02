#ifndef BPT_MEMORYRIVER_HPP
#define BPT_MEMORYRIVER_HPP

#include <fstream>

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
  int sizeofT = sizeof(T);

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
    if(file.is_open()) {
      file.close();
    }
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
      return (size - info_len * sizeof(int)) / sizeofT;
    }
    size_t offset = head * sizeofT + info_len * sizeof(int);
    file.seekp(offset);
    int new_head;
    file.read(reinterpret_cast<char *>(&new_head), sizeof(int));
    write_info(new_head, 2);
    int size = file.tellp();
    file.seekp(offset);
    file.write(reinterpret_cast<const char *>(&t), sizeofT);
    return head;
  }

  // 用t的值更新位置索引index对应的对象，保证调用的index都是由write函数产生
  void update(const T &t, const int index) { 
    /* your code here */ 
    size_t offset = sizeof(int) * info_len + sizeofT * index;
    file.seekp(offset);
    file.write(reinterpret_cast<const char *>(&t), sizeofT);
  }

  // 读出位置索引index对应的T对象的值并赋值给t，保证调用的index都是由write函数产生
  void read(T &t, const int index) { 
    /* your code here */ 
    size_t offset = sizeof(int) * info_len + sizeofT * index;
    file.seekg(offset);
    file.read(reinterpret_cast<char *>(&t), sizeofT);
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
  }
};

#endif // BPT_MEMORYRIVER_HPP