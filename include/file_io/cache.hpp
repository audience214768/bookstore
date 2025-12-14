#ifndef CACHE
#define CACHE

#include "config.hpp"
#include <iostream>
#include <functional>
#include <list>
#include <unordered_map>

template<class Key, class Value>
class Cache {
private:
  struct Node {
    Key key_;
    Value value_;
    bool is_dirty_;
    Node() = default;
    Node(Key key, Value value, bool is_dirty):key_(key), value_(value), is_dirty_(is_dirty) {}
  };
  std::list<Node> cache; // the front for hot, the back for cold
  std::unordered_map<Key, typename std::list<Node>::iterator> map;
public:
  Value *Get(const Key &key) {
    auto it = map.find(key);
    if(it != map.end()) {
      cache.splice(cache.begin(), cache, it->second);
      return &(it->second->value_);
    }
    return nullptr;
  }
  std::pair<bool, Node> Put(const Key &key, const Value &value, bool is_dirty = true) {
    auto it = map.find(key);
    if(it != map.end()) {
      it->second->value_ = value;
      it->second->is_dirty_ = is_dirty;
      cache.splice(cache.begin(), cache, it->second);
      return std::make_pair(false, Node());
    } 
    bool need_back = false;
    Node old_node;
    if(cache.size() == CacheSize) {
      need_back = true;
      old_node = cache.back();
      map.erase(old_node.key_);
      cache.pop_back();
    }
    Node new_node(key, value, is_dirty);
    cache.push_front(new_node);
    map[key] = cache.begin();
    return std::make_pair(need_back, old_node);
  }
  void FlushAll(std::function<void(const Key &, const Value &)> func) {
    for(auto node : cache) {
      //std::cerr << 1 << std::endl;
      if(node.is_dirty_) {
        func(node.key_, node.value_);
        node.is_dirty_ = 0;
      }
    }
  }
  void Delete(const Key &key) {
    for(auto it = cache.begin(); it != cache.end(); it++) {
      if(it->key_ == key) {
        cache.erase(it);
        map.erase(key);
        break;
      }
    }
  }
};

#endif