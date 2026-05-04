#include "tests.h"
#include "helpers/List.h"
#include <Arduino.h>
#include <unity.h>

using namespace freertos::helpers;

void setUp() {}
void tearDown() {}

// --- SList Tests (Singly-linked list without tail pointer) ---

void test_slist_push_pop_front()
{
  SList<int> list;
  TEST_ASSERT_TRUE(list.empty());
  TEST_ASSERT_EQUAL(0, list.size());

  SNode<int> n1(10);
  SNode<int> n2(20);

  list.push_front(&n1);
  TEST_ASSERT_FALSE(list.empty());
  TEST_ASSERT_EQUAL(1, list.size());
  TEST_ASSERT_EQUAL(10, list.front());

  list.push_front(&n2);
  TEST_ASSERT_EQUAL(2, list.size());
  TEST_ASSERT_EQUAL(20, list.front());

  SNode<int>* popped = list.pop_front();
  TEST_ASSERT_EQUAL(20, popped->data);

  TEST_ASSERT_EQUAL(1, list.size());
  TEST_ASSERT_EQUAL(10, list.front());

  popped = list.pop_front();
  TEST_ASSERT_EQUAL(10, popped->data);

  TEST_ASSERT_TRUE(list.empty());
}

void test_slist_insert_erase_after()
{
  SList<int> list;
  SNode<int> n1(1);
  SNode<int> n2(2);
  SNode<int> n3(3);

  list.push_front(&n1); // [1]

  // Insert n2 after n1: [1, 2]
  list.insert_after(list.begin(), &n2);
  TEST_ASSERT_EQUAL(2, list.size());

  // Insert n3 after n1: [1, 3, 2]
  list.insert_after(list.begin(), &n3);
  TEST_ASSERT_EQUAL(3, list.size());

  // Erase after n1 (which is n3)
  SNode<int>* erased = list.erase_after(list.begin());
  TEST_ASSERT_EQUAL(3, erased->data);
  TEST_ASSERT_EQUAL(2, list.size());

  list.clear();
}

void test_slist_copy_swap_reverse_max_size()
{
  SList<int> list;
  SNode<int> n1(1);
  SNode<int> n2(2);
  SNode<int> n3(3);

  list.push_front(&n1); // [1]
  list.push_front(&n2); // [2, 1]
  list.push_front(&n3); // [3, 2, 1]

  TEST_ASSERT_TRUE(list.max_size() > 0);

  // Shallow copy shares nodes by design in this intrusive list.
  SList<int> copy = list;
  TEST_ASSERT_EQUAL(3, copy.front());

  list.reverse(); // [1, 2, 3]
  TEST_ASSERT_EQUAL(1, list.front());

  SList<int> other;
  SNode<int> n4(4);
  other.push_front(&n4);
  list.swap(other);

  TEST_ASSERT_EQUAL(4, list.front());
  TEST_ASSERT_EQUAL(1, other.front());
}

// --- DTailList Tests (Doubly-linked list with tail pointer) ---

void test_dtaillist_push_pop()
{
  DTailList<int> list;
  DNode<int> n1(1);
  DNode<int> n2(2);
  DNode<int> n3(3);

  list.push_back(&n1);  // [1]
  list.push_front(&n2); // [2, 1]
  list.push_back(&n3);  // [2, 1, 3]

  TEST_ASSERT_EQUAL(3, list.size());
  TEST_ASSERT_EQUAL(2, list.front());
  TEST_ASSERT_EQUAL(3, list.back());

  DNode<int>* popped = list.pop_front();
  TEST_ASSERT_EQUAL(2, popped->data);

  popped = list.pop_back();
  TEST_ASSERT_EQUAL(3, popped->data);

  popped = list.pop_back();
  TEST_ASSERT_EQUAL(1, popped->data);

  TEST_ASSERT_TRUE(list.empty());
}

void test_dtaillist_iterator()
{
  DTailList<int> list;
  DNode<int> n1(1);
  DNode<int> n2(2);
  DNode<int> n3(3);

  list.push_back(&n1);
  list.push_back(&n2);
  list.push_back(&n3);

  int m_val = 1;

  for (auto element : list)
    TEST_ASSERT_EQUAL(m_val++, element);

  TEST_ASSERT_EQUAL(4, m_val);

  m_val = 3;
  auto it = list.begin();
  if (it != list.end()) {
    auto next = it;
    ++next;
    while (next != list.end()) {
      ++it;
      ++next;
    }
  }

  for (;; --it) {
    TEST_ASSERT_EQUAL(m_val--, *it);
    if (it == list.begin())
      break;
  }
  TEST_ASSERT_EQUAL(0, m_val);

  list.clear();
}

void test_dtaillist_end_decrement_and_reverse_iterators()
{
  DTailList<int> list;
  DNode<int> n1(1);
  DNode<int> n2(2);
  DNode<int> n3(3);

  list.push_back(&n1);
  list.push_back(&n2);
  list.push_back(&n3);

  auto it = list.end();
  --it;
  TEST_ASSERT_EQUAL(3, *it);

  int expected = 3;
  for (auto rit = list.rbegin(); rit != list.rend(); ++rit) {
    TEST_ASSERT_EQUAL(expected--, *rit);
  }
  TEST_ASSERT_EQUAL(0, expected);

  auto cit = list.cend();
  --cit;
  TEST_ASSERT_EQUAL(3, *cit);

  TEST_ASSERT_TRUE(list.max_size() > 0);
}

void test_dtaillist_swap_and_reverse()
{
  DTailList<int> list;
  DNode<int> n1(1);
  DNode<int> n2(2);
  DNode<int> n3(3);
  list.push_back(&n1);
  list.push_back(&n2);
  list.push_back(&n3);

  DTailList<int> other;
  DNode<int> n4(4);
  other.push_back(&n4);

  list.swap(other);
  TEST_ASSERT_EQUAL(4, list.front());
  TEST_ASSERT_EQUAL(1, other.front());

  other.reverse(); // [3, 2, 1]
  TEST_ASSERT_EQUAL(3, other.front());
  TEST_ASSERT_EQUAL(1, other.back());
}

// --- STailList Tests (Singly-linked list with tail pointer) ---

void test_staillist_push_pop()
{
  STailList<int> list;
  SNode<int> n1(100);
  SNode<int> n2(200);

  list.push_back(&n1);
  list.push_back(&n2);

  TEST_ASSERT_EQUAL(2, list.size());
  TEST_ASSERT_EQUAL(100, list.front());
  TEST_ASSERT_EQUAL(200, list.back());

  SNode<int>* popped = list.pop_front();
  TEST_ASSERT_EQUAL(100, popped->data);

  popped = list.pop_back();
  TEST_ASSERT_EQUAL(200, popped->data);

  TEST_ASSERT_TRUE(list.empty());
}

void test_staillist_swap_reverse_max_size_and_insert_after_const()
{
  STailList<int> list;
  SNode<int> n1(1);
  SNode<int> n2(2);
  SNode<int> n3(3);
  list.push_back(&n1);
  list.push_back(&n3); // [1, 3]

  STailList<int>::const_iterator pos = list.begin();
  list.insert_after(pos, &n2); // [1, 2, 3]

  TEST_ASSERT_EQUAL(1, list.front());
  TEST_ASSERT_EQUAL(3, list.back());

  list.reverse(); // [3, 2, 1]
  TEST_ASSERT_EQUAL(3, list.front());
  TEST_ASSERT_EQUAL(1, list.back());

  STailList<int> other;
  SNode<int> n4(4);
  other.push_back(&n4);
  list.swap(other);

  TEST_ASSERT_EQUAL(4, list.front());
  TEST_ASSERT_EQUAL(3, other.front());
  TEST_ASSERT_TRUE(list.max_size() > 0);
}

// --- DList Tests (Doubly-linked list without tail pointer) ---

void test_dlist_push_pop()
{
  DList<int> list;
  DNode<int> n1(10);
  DNode<int> n2(20);

  list.push_back(&n1);
  list.push_front(&n2); // [20, 10]

  TEST_ASSERT_EQUAL(2, list.size());
  TEST_ASSERT_EQUAL(20, list.front());
  TEST_ASSERT_EQUAL(10, list.back());

  DNode<int>* popped = list.pop_back();
  TEST_ASSERT_EQUAL(10, popped->data);

  popped = list.pop_front();
  TEST_ASSERT_EQUAL(20, popped->data);

  TEST_ASSERT_TRUE(list.empty());
}

void test_dlist_insert_erase()
{
  DList<int> list;
  DNode<int> n1(1);
  DNode<int> n2(2);

  list.push_back(&n1);

  // Insert before n1
  list.insert(list.begin(), &n2); // [2, 1]

  TEST_ASSERT_EQUAL(2, list.front());

  DNode<int>* erased = list.erase(list.begin());
  TEST_ASSERT_EQUAL(2, erased->data);

  TEST_ASSERT_EQUAL(1, list.front());

  erased = list.erase(list.begin());
  TEST_ASSERT_EQUAL(1, erased->data);
}

void test_dlist_end_decrement_and_reverse_iterators()
{
  DList<int> list;
  DNode<int> n1(1);
  DNode<int> n2(2);
  DNode<int> n3(3);
  list.push_back(&n1);
  list.push_back(&n2);
  list.push_back(&n3);

  auto it = list.end();
  --it;
  TEST_ASSERT_EQUAL(3, *it);

  int expected = 3;
  for (auto rit = list.rbegin(); rit != list.rend(); ++rit) {
    TEST_ASSERT_EQUAL(expected--, *rit);
  }
  TEST_ASSERT_EQUAL(0, expected);

  auto cit = list.cend();
  --cit;
  TEST_ASSERT_EQUAL(3, *cit);
}

void test_dlist_swap_reverse_and_max_size()
{
  DList<int> list;
  DNode<int> n1(1);
  DNode<int> n2(2);
  DNode<int> n3(3);
  list.push_back(&n1);
  list.push_back(&n2);
  list.push_back(&n3);

  DList<int> other;
  DNode<int> n4(4);
  other.push_back(&n4);

  list.swap(other);
  TEST_ASSERT_EQUAL(4, list.front());
  TEST_ASSERT_EQUAL(1, other.front());

  other.reverse(); // [3, 2, 1]
  TEST_ASSERT_EQUAL(3, other.front());
  TEST_ASSERT_EQUAL(1, other.back());
  TEST_ASSERT_TRUE(other.max_size() > 0);
}

void runUnityTests()
{
  UNITY_BEGIN();

  RUN_TEST(test_slist_push_pop_front);
  RUN_TEST(test_slist_insert_erase_after);
  RUN_TEST(test_slist_copy_swap_reverse_max_size);

  RUN_TEST(test_dtaillist_push_pop);
  RUN_TEST(test_dtaillist_iterator);
  RUN_TEST(test_dtaillist_end_decrement_and_reverse_iterators);
  RUN_TEST(test_dtaillist_swap_and_reverse);

  RUN_TEST(test_staillist_push_pop);
  RUN_TEST(test_staillist_swap_reverse_max_size_and_insert_after_const);

  RUN_TEST(test_dlist_push_pop);
  RUN_TEST(test_dlist_insert_erase);
  RUN_TEST(test_dlist_end_decrement_and_reverse_iterators);
  RUN_TEST(test_dlist_swap_reverse_and_max_size);

  UNITY_END();
}