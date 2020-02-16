#pragma once

#include "align.h"
#include <cassert>
#include <cstddef>
#include <cstdint>
#include <algorithm>
#include <array>
#include <limits>
#include <new>
#include <vector>

template<std::size_t Arity, class ValueType, class LineType = std::array<ValueType, Arity>>
class HeapN {
  static_assert(Arity > 1, "Arity must be greater than 1");
  static_assert((Arity & (Arity - 1)) == 0, "Arity must be power of 2");

 public:
  typedef ValueType value_type;
  typedef std::size_t size_type;

 public:
  HeapN() : size_(0) { }
  ~HeapN() = default;
  HeapN(const HeapN&) = delete;
  HeapN& operator=(const HeapN&) = delete;

  size_type size() const { return size_; }

  value_type& operator[](size_type index) {
    return data()[index];
  }

  value_type* extend(size_type n) {
    if (n > kSizeMax - size_) throw_bad_alloc();
    size_type new_size = size_ + n;
    if (new_size > kArity * vector_.size()) {
      static_assert(std::numeric_limits<typename vector_type::size_type>::max() >=
                    std::numeric_limits<size_type>::max() / kArity);
      // Smallest new_vector_size s.t. size <= kArity * new_vector_size.
      size_type new_vector_size = align_up(new_size, kArity) / kArity;
      vector_.resize(new_vector_size, kLineMax);
    }
    size_ = new_size;
    value_type* array = data();
    return array + (size_ - n);
  }

  template<class InputIterator>
  void append(InputIterator begin, InputIterator end) {
    value_type* array = data();
    while (begin != end) {
      if (size_ == kArity * vector_.size()) {
        vector_.push_back(kLineMax);
        array = data();
      }
      array[size_++] = *begin++;
    }
  }

  void pull_up(value_type b, size_type q) {
    assert(q < size_);
    value_type* array = data();
    while (q >= kArity) {
      size_t p = parent(q);
      value_type a = array[p];
      if (a <= b) break;
      array[q] = a;
      q = p;
    }
    array[q] = b;
  }

  void push_down(value_type a, size_type p) {
    assert(p < size_);
    value_type* array = data();
    while (true) {
      size_t q = children(p);
      if (q >= size_) break;
      minpos_type x = minpos(vector_[q / kArity]);
      value_type b = x.min;
      if (a <= b) break;
      array[p] = b;
      p = q + x.pos;
    }
    array[p] = a;
  }

  void heapify() {
    if (size_ <= kArity) return;
    value_type* array = data();
    size_t q = align_down(size_ - 1, kArity);

    // The first while loop is an optimization for the bottom level of the heap,
    // inlining the call to heap_push_down which is trivial at the bottom level.
    // Here "bottom level" means a line without children.
    size_t r = parent(q);
    while (q > r) {
      minpos_type x = minpos(vector_[q / kArity]);
      value_type b = x.min;
      size_t p = parent(q);
      value_type a = array[p];
      if (b < a) {
        array[p] = b;
        // The next line inlines push_down(a, q + minpos_pos(x))
        // with the knowledge that children(q) >= size_.
        array[q + x.pos] = a;
      }
      q -= kArity;
    }

    while (q > 0) {
      minpos_type x = minpos(vector_[q / kArity]);
      value_type b = x.min;
      size_t p = parent(q);
      value_type a = array[p];
      if (b < a) {
        array[p] = b;
        push_down(a, q + x.pos);
      }
      q -= kArity;
    }
  }

  bool is_heap() const {
    if (size_ <= kArity) return true;
    value_type const* array = data();
    size_t q = align_down(size_ - 1, kArity);
    while (q > 0) {
      minpos_type x = minpos(vector_[q / kArity]);
      value_type b = x.min;
      size_t p = parent(q);
      value_type a = array[p];
      if (b < a) return false;
      q -= kArity;
    }
    return true;
  }

  void push(value_type b) {
    if (size_ == kArity * vector_.size()) vector_.push_back(kLineMax);
    size_++;
    pull_up(b, size_ - 1);
  }

  value_type const top() {
    assert(size_ > 0);
    minpos_type x = minpos(vector_[0]);
    return x.min;
  }

  value_type pop() {
    assert(size_ > 0);
    minpos_type x = minpos(vector_[0]);
    value_type b = x.min;
    value_type* array = data();
    value_type a = array[size_ - 1];
    array[size_ - 1] = kMax;
    size_--;
    size_type p = x.pos;
    if (p != size_) {
      push_down(a, p);
    }
    return b;
  }

  void sort() {
    line_type v = kLineMax;
    size_type x = size_;
    size_type i = x % kArity;
    x -= i;
    while (i > 0) {
      --i;
      v[i] = pop();
    }
    vector_[x / kArity] = v;
    while (x > 0) {
      x -= kArity;
      for (size_type j = kArity; j > 0; --j) {
        v[j - 1] = pop();
      }
      vector_[x / kArity] = v;
    }
  }

  bool is_sorted(size_type sz) const {
    return std::is_sorted(data(), data() + sz, std::greater<value_type>());
  }

  void clear() {
    vector_.clear();
    vector_.shrink_to_fit(); // to match heap_clear(heap*)
    size_ = 0;
  }

 private:
  static constexpr value_type kMax = std::numeric_limits<value_type>::max();
  static constexpr size_type kArity = Arity;
  static constexpr size_type kSizeMax = align_down(std::numeric_limits<size_type>::max(), kArity);

  static size_type parent(size_type q) { return (q / kArity) - 1; }
  static size_type children(size_type p) { return (p + 1) * kArity; }

  typedef LineType line_type;

  struct minpos_type {
    size_type pos;
    value_type min;
  };
  static minpos_type minpos(const line_type& line) {
    minpos_type mp;
    // TODO(soren): implement this correctly for kArity > 2
    value_type a = std::get<0>(line);
    value_type b = std::get<1>(line);
    if (a <= b) {
      mp.pos = 0;
      mp.min = a;
    } else {
      mp.pos = 1;
      mp.min = b;
    }
    return mp;
  }

  [[noreturn]] static void throw_bad_alloc() {
    std::bad_alloc exception;
    throw exception;
  }
  value_type* data() { return reinterpret_cast<value_type*>(vector_.data()); }
  value_type const* data() const { return reinterpret_cast<value_type const*>(vector_.data()); }
  static constexpr line_type kLineMax = { kMax, kMax, };  // TODO(soren): fill with kMax kArity times
  typedef std::vector<line_type> vector_type;
  vector_type vector_;
  size_type size_;
};
