#ifndef JOURNAL
#define JOURNAL

#include <fstream>
#include <vector>

enum OpType {ready, write, finish};

struct JournalHeader {
  OpType type;
  int fileID;
  int offset;
  int data_size;
};

class JournalManager {
private:
  std::fstream file_;
  std::vector<std::fstream *> file_map_;
  int count_;
  int sizeofHead = sizeof(JournalHeader);
  void AddLog(const JournalHeader &, const void *);
public:
  JournalManager(const std::string &);
  void Ready();
  void Write(int, int, int, const void *data);
  void Finish();
  void Recover();
};


#endif