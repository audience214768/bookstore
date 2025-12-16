#ifndef JOURNAL
#define JOURNAL

#include <functional>
#include <string>
#include <vector>

enum OpType {READY, WRITE, FINISH};

class JournalManager {
private:
  struct JournalHeader {
    OpType type;
    int fileID;
    int offset;
    int data_size;
  };
  struct Data {
    int fileID;
    int offset;
    int data_size;
    std::vector<char> buffer;
  };
  std::string file_name_;
  using rawWrite = std::function<void(int , int, const char *)>;
  using reFresh = std::function<void()>;
  std::vector<rawWrite> file_map_;
  std::vector<reFresh> fresh_list_;
  int sizeofHead = sizeof(JournalHeader);
  std::vector<char> buffer_;
  void AddLog(const JournalHeader &, const char *);
public:
  JournalManager(const std::string &);
  int Init(rawWrite, reFresh);
  void Ready();
  void Write(int, int, int, const char *data);
  void Finish();
  void Recover();
};


#endif