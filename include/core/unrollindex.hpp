#ifndef UNROLLINDEX
#define UNROLLINDEX

#include "config.hpp"
#include "journal.hpp"
#include "memoryriver.hpp"
#include <algorithm>
#include <cstdlib>
#include <functional>
#include <vector>

template<class Key, class Value>
struct Element {
  Key key;
  Value value;
  bool operator<(const Element<Key, Value> &other) const {
    if(key != other.key) {
      return key < other.key;
    }
    return value < other.value;
  }
  bool operator<(const Key &other_key) const {
    return key < other_key;
  }
};

template<class Key, class Value>
struct Block {
  int next = -1;//for free block, the next free block;for alloc block, the next alloc block
  int count_ = 0;
  Element<Key, Value> data[BlockSize];
};

template<class Key, class Value>
class UnrollIndex {
using BlockType = Block<Key, Value>;
private:
  int first_block;
  MemoryRiver<BlockType> file_;
  void GetBlock(BlockType &block,const int index) {
    file_.read(block, index);
  }
  int AddBlock(const BlockType &block) {
    return file_.write(block);
  }
  void DeleteBlock(int index) {
    file_.Delete(index);
  }
  std::vector<Value> Find(const Key &key) {
    int now = first_block;
    BlockType block;
    for(; now != -1; now = block.next) {
      GetBlock(block, now);
      if(block.count_ > 0 && block.data[block.count_ - 1].key >= key) {
        break;
      }
    }
    //std::cerr << now << std::endl;
    std::vector<Value> value;
    while(now != -1) {
      file_.read(block, now);
      auto it = std::lower_bound(block.data, block.data + block.count_, key);
      for(; it != block.data + block.count_ && it->key == key; it++) {
        value.push_back(it->value);
      }
      if(it == block.data + block.count_) {
        now = block.next;
      } else {
        break;
      }
    }
    return value;
  }
  void Split(BlockType &block, int index) {
    //std::cerr << "begin split" << std::endl;
    BlockType newblock;
    for(int i = block.count_ / 2; i < block.count_; i++) {
      newblock.data[i - block.count_ / 2] = block.data[i];
    }
    newblock.count_ = block.count_ - block.count_ / 2;
    block.count_ /= 2;
    newblock.next = block.next;
    int new_index = AddBlock(newblock);
    block.next = new_index;
    UpdateBlock(block, index);
    /*
    //std::cerr << "finish split" << std::endl;
    for(int i = 0; i < block.count_; i++) {
      std::cerr << block.data[i].key << " " << block.data[i].value << std::endl;
    }
    std::cerr << std::endl;
    for(int i = 0; i < newblock.count_; i++) {
      std::cerr << newblock.data[i].key << " " << newblock.data[i].value << std::endl;
    }*/
  }
  void merge(BlockType &block, int index, BlockType &next_block, int next_index) {
    for(int i = 0; i < next_block.count_; i++) {
      block.data[i + block.count_] = next_block.data[i];
    }
    block.count_ += next_block.count_;
    block.next = next_block.next;
    DeleteBlock(next_index);
    UpdateBlock(block, index);
    /*
    //std::cerr << "finish merge" << std::endl;
    for(int i = 0; i < block.count_; i++) {
      std::cerr << block.data[i].key << " " << block.data[i].value << std::endl;
    }*/
  }
  void UpdateBlock(BlockType &block,const int index) {
    if(block.count_ == BlockSize) {
      Split(block, index);
      return ;
    }
    if(block.next != -1) {
      BlockType next_block;
      file_.read(next_block, block.next);
      if(block.count_ + next_block.count_ <= BlockSize * 2 / 3) {
        merge(block, index, next_block, block.next);
        return ;
      } 
    }
    file_.update(block, index);
  }
public:
  UnrollIndex(JournalManager &jm, const std::string &file_name):file_(jm, file_name) {
    file_.get_info(first_block, 1);
    //std::cerr << "head : " << head << std::endl;
    if(first_block == -1) {
      BlockType head;
      int index = file_.write(head);
      first_block = index;
      file_.write_info(first_block, 1);
      //std::cerr << head.next << std::endl;
    }
  }
  std::vector<Value> operator[](const Key &key) {
    return Find(key);
  }
  void Insert(const Key &key, const Value &value) {
    int now = first_block;
    //std::cerr << now << std::endl;
    BlockType block;
    for(; now != -1; now = block.next) {
      GetBlock(block, now);
      //std::cerr << block.next << std::endl;
      if(block.count_ > 0 && block.data[block.count_ - 1].key >= key || block.next == -1) {
        break;
      }
    }
    int i = block.count_;
    for(; i > 0 && key < block.data[i - 1].key; i--) {
      block.data[i] = block.data[i - 1];
    }
    //std::cerr << now << " " << i << std::endl;
    block.data[i].key = key;
    block.data[i].value = value;
    block.count_++;
    UpdateBlock(block, now);
  }
  std::vector<Value> traverse(std::function<void(Value &)> callback) {
    int now;
    file_.get_info(now, 1);
    std::vector<Value> list;
    while(now != -1) {
      BlockType block;
      file_.read(block, now);
      for(int i = 0; i < block.count_; i++) {
        callback(block.data[i].value);
      }
      now = block.next;
    }
    return list;
  }
  bool Delete(const Key &key, const Value &value) { // return 0 for {key, value} not exist
    int now = first_block;
    BlockType block;
    for(; now != -1; now = block.next) {
      GetBlock(block, now);
      if(block.count_ > 0 && block.data[block.count_ - 1].key >= key) {
        break;
      }
    }
    //std::cerr << now << std::endl;
    while(now != -1) {
      file_.read(block, now);
      auto it = std::lower_bound(block.data, block.data + block.count_, key);
      for(; it != block.data + block.count_ && it->key == key; it++) {
        //std::cerr << it->key << " " << it->value << std::endl;
        if(it->value == value) {
          for(auto it1 = it; it1 != block.data + block.count_ - 1; it1++) {
            *it1 = *(it1 + 1);
          }
          block.count_--;
          UpdateBlock(block, now);
          return 1;
        }
      }
      if(it == block.data + block.count_) {
        now = block.next;
      } else {
        break;
      }
    }
    return 0;
  }
};

#endif


