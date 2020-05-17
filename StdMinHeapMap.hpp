#pragma once

#include "StdMinHeap.hpp"
#include <cassert>
#include <cstddef>
#include <cstdint>
#include <algorithm>
#include <functional>
#include <limits>
#include <new>
#include <utility>
#include <vector>

template<class M, class V = uint16_t, class Compare = std::greater<V>>
class StdMinHeapMap {
 public:
  typedef V key_type;
  typedef M mapped_type;
  typedef std::pair<V, M> entry_type;

 private:
  class ValueCompare {
    Compare cmp_;
   public:
    ValueCompare(const Compare& cmp = Compare()) : cmp_(cmp) { }
    ~ValueCompare() = default;
    constexpr bool operator()(const entry_type& a, const entry_type& b) const {
      return cmp_(a.first, b.first);
    }
  };

  typedef StdMinHeap<entry_type, ValueCompare> heap_type;

 public:
  typedef typename heap_type::size_type size_type;

  StdMinHeapMap(const Compare& cmp = Compare()) : heap_(ValueCompare(cmp)) { }
  ~StdMinHeapMap() = default;
  StdMinHeapMap(const StdMinHeapMap&) = delete;
  StdMinHeapMap& operator=(const StdMinHeapMap&) = delete;

  size_type size() const { return heap_.size(); }

  entry_type entry(size_type index) const { return heap_[index]; }

  void set_entry(size_type index, entry_type a) { heap_[index] = a; }

  entry_type* extend(size_type n) { return heap_.extend(n); }

  template<class InputIterator>
  void append_entries(InputIterator begin, InputIterator end) {
    heap_.append(begin, end);
  }

  void pull_up(entry_type b, size_type q) { heap_.pull_up(b, q); }

  void push_down(entry_type a, size_type p) { heap_.push_down(a, p); }

  void heapify() { heap_.heapify(); }

  bool is_heap() const { return heap_.is_heap(); }

  void push_entry(entry_type b) { heap_.push(b); }

  void push_entry(key_type k, mapped_type m) { heap_.push(entry_type(k, m)); }

  entry_type top_entry() const { return heap_.top(); }

  entry_type pop_entry() { return heap_.pop(); }

  void sort() { heap_.sort(); }

  bool is_sorted(size_type sz) const { return heap_.is_sorted(sz); }

  void clear() { heap_.clear(); }

 private:
  heap_type heap_;
};
