/*
   # first install gtest as described in h8Test.cpp
   brew install boost
   g++ -g -std=c++17 -msse4 -lgtest -lgtest_main HeapMapTest.cpp
   ./a.out
*/

#include "Heap8Aux.hpp"
#include "Heap8Embed.hpp"
#include "StdMinHeapMap.hpp"
#include "U48.hpp"
#include <vector>
#include <boost/iterator/counting_iterator.hpp>
#include <boost/iterator/transform_iterator.hpp>
#include <gtest/gtest.h>

namespace {

using boost::iterators::counting_iterator;
using boost::iterators::transform_iterator;

template <class T>
class HeapMapTest : public testing::Test {
 protected:
  T heap_;
};

typedef testing::Types<
  Heap8Aux<U48>,
  Heap8Embed<U48>,
  StdMinHeapMap<U48>
> Implementations;

TYPED_TEST_SUITE(HeapMapTest, Implementations);

TYPED_TEST(HeapMapTest, Clear) {
  EXPECT_EQ(0, this->heap_.size());
  this->heap_.push_entry(1, 100);
  EXPECT_EQ(1, this->heap_.size());
  this->heap_.clear();
  EXPECT_EQ(0, this->heap_.size());
}

TYPED_TEST(HeapMapTest, Push3) {
  typedef typename TypeParam::entry_type entry_type;
  entry_type p1(1, 100), p2(2, 200), p3(3, 300);
  EXPECT_TRUE(this->heap_.is_heap());
  this->heap_.push_entry(p2);
  EXPECT_EQ(1, this->heap_.size());
  EXPECT_EQ(p2, this->heap_.top_entry());
  EXPECT_TRUE(this->heap_.is_heap());
  this->heap_.push_entry(p1);
  EXPECT_EQ(p1, this->heap_.top_entry());
  EXPECT_EQ(2, this->heap_.size());
  EXPECT_TRUE(this->heap_.is_heap());
  this->heap_.push_entry(p3);
  EXPECT_EQ(p1, this->heap_.top_entry());
  EXPECT_EQ(3, this->heap_.size());
  EXPECT_TRUE(this->heap_.is_heap());
}

TYPED_TEST(HeapMapTest, Heapify3) {
  typedef typename TypeParam::entry_type entry_type;
  entry_type p1(1, 41), p2(2, 42), p3(3, 43);
  std::vector<entry_type> entries{p2, p1, p3};
  this->heap_.append_entries(entries.begin(), entries.end());
  EXPECT_EQ(entries.size(), this->heap_.size());
  this->heap_.heapify();
  EXPECT_TRUE(this->heap_.is_heap());
  EXPECT_EQ(p1, this->heap_.pop_entry());
  EXPECT_EQ(p2, this->heap_.pop_entry());
  EXPECT_EQ(p3, this->heap_.pop_entry());
}

TYPED_TEST(HeapMapTest, Sort3) {
  typedef typename TypeParam::entry_type entry_type;
  entry_type p1(1, 41), p2(2, 42), p3(3, 43);
  std::vector<entry_type> entries{p2, p1, p3};
  this->heap_.append_entries(entries.begin(), entries.end());
  this->heap_.heapify();
  this->heap_.sort();
  EXPECT_EQ(0, this->heap_.size());
  EXPECT_EQ(p3, this->heap_.entry(0));
  EXPECT_EQ(p2, this->heap_.entry(1));
  EXPECT_EQ(p1, this->heap_.entry(2));
  EXPECT_TRUE(this->heap_.is_sorted(entries.size()));
}

TYPED_TEST(HeapMapTest, Heapify100) {
  typedef typename TypeParam::key_type key_type;
  typedef typename TypeParam::entry_type entry_type;
  key_type const count = 100;
  counting_iterator<key_type> zero(0);
  auto revert = [=](key_type i) {
    key_type j = count - 1 - i;
    return entry_type(j, 40 + j);
  };
  auto begin = transform_iterator(zero, revert);
  this->heap_.append_entries(begin, begin + count);
  EXPECT_EQ(count, this->heap_.size());
  // 8 is the arity of Heap8Aux (dirty implementation detail, oh well)
  EXPECT_LE(count - 8, this->heap_.top_entry().first);
  this->heap_.heapify();
  EXPECT_TRUE(this->heap_.is_heap());
  EXPECT_EQ(entry_type(0, 40), this->heap_.top_entry());
  for (key_type i = 0; i < count; ++i) {
    EXPECT_EQ(entry_type(i, i + 40), this->heap_.pop_entry());
  }
}

} // namespace
