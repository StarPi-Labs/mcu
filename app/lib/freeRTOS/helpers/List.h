#pragma once

#include <cassert>
#include <cstddef>
#include <iterator>
#include <utility>

namespace freertos::helpers
{

/**
 * @brief Node for singly-linked list
 */
template <typename T> struct SNode {
  T data;
  SNode* next = nullptr;

  explicit SNode(const T& value)
      : data(value)
  {
  }
  explicit SNode(T&& value)
      : data(std::move(value))
  {
  }
};

/**
 * @brief Node for doubly-linked list
 */
template <typename T> struct DNode {
  T data;
  DNode* next = nullptr;
  DNode* prev = nullptr;

  explicit DNode(const T& value)
      : data(value)
  {
  }
  explicit DNode(T&& value)
      : data(std::move(value))
  {
  }
};

/**
 * @brief Singly-linked list with STL-like interface
 *
 * Provides O(1) insertion/deletion at front, O(n) elsewhere.
 * Caller controls node allocation - list only manages pointers.
 * This allows custom allocators, stack allocation, or pre-allocated pools.
 */
template <typename T> class SList
{
public:
  using value_type = T;
  using pointer = T*;
  using reference = T&;
  using const_reference = const T&;
  using node_type = SNode<T>;

  // Forward declaration for iterator
  class iterator;
  class const_iterator;

  // Default constructor and destructor
  SList() : m_head(nullptr) {}
  /// @brief Destructor does NOT delete nodes - caller is responsible for node
  /// memory management
  ~SList() = default;

  // Prevent copying (for embedded systems with limited memory)
  SList(const SList&) = delete;
  SList& operator=(const SList&) = delete;

  // Allow moving
  SList(SList&& other) noexcept
      : m_head(other.m_head)
  {
    other.m_head = nullptr;
  }

  SList& operator=(SList&& other) noexcept
  {
    if (this != &other) {
      m_head = other.m_head;
      other.m_head = nullptr;
    }

    return *this;
  }

  /**
   * @brief Insert node at front in O(1)
   * Caller owns the node and is responsible for deallocation
   */
  void push_front(node_type* node)
  {
    assert(node && "Node is null");
    node->next = m_head;
    m_head = node;
  }

  /**
   * @brief Remove node at front in O(1)
   * Returns the removed node (caller must delete it)
   */
  node_type* pop_front()
  {
    assert(m_head && "List is empty");
    auto* temp = m_head;
    m_head = m_head->next;
    temp->next = nullptr;
    return temp;
  }

  /**
   * @brief Access front element
   */
  reference front()
  {
    assert(m_head && "List is empty");
    return m_head->data;
  }
  const_reference front() const
  {
    assert(m_head && "List is empty");
    return m_head->data;
  }

  /**
   * @brief Check if list is empty
   */
  bool empty() const { return m_head == nullptr; }

  /**
   * @brief Get number of elements
   */
  size_t size() const
  {
    size_t count = 0;

    for (auto* curr = m_head; curr != nullptr; curr = curr->next)
      ++count;

    return count;
  }

  /**
   * @brief Unlink all nodes from list (does NOT delete them)
   * Caller is responsible for deleting nodes
   */
  void clear() { m_head = nullptr; }

  /**
   * @brief Insert node after iterator position in O(1)
   * Caller owns the node and is responsible for deallocation
   */
  iterator insert_after(iterator pos, node_type* node)
  {
    assert(node && "Node is null");

    if (pos.m_current) {
      node->next = pos.m_current->next;
      pos.m_current->next = node;
    } else {
      node->next = m_head;
      m_head = node;
    }
    return iterator(node);
  }

  /**
   * @brief Remove node after iterator position in O(1)
   * Returns the removed node (caller must delete it)
   */
  node_type* erase_after(iterator pos)
  {
    assert(pos.m_current && "Iterator is invalid");
    assert(pos.m_current->next && "Iterator is at end");

    auto* temp = pos.m_current->next;
    pos.m_current->next = temp->next;
    temp->next = nullptr;
    return temp;
  }

  // Iterators
  iterator begin() { return iterator(m_head); }
  const_iterator begin() const { return const_iterator(m_head); }
  const_iterator cbegin() const { return const_iterator(m_head); }

  iterator end() { return iterator(nullptr); }
  const_iterator end() const { return const_iterator(nullptr); }
  const_iterator cend() const { return const_iterator(nullptr); }

  class iterator
  {
  public:
    using difference_type = std::ptrdiff_t;
    using value_type = T;
    using pointer = T*;
    using reference = T&;
    using iterator_category = std::forward_iterator_tag;

    iterator()
        : m_current(nullptr)
    {
    }
    explicit iterator(node_type* node)
        : m_current(node)
    {
    }

    reference operator*() { return m_current->data; }
    pointer operator->() { return &m_current->data; }

    iterator& operator++()
    {
      if (m_current)
        m_current = m_current->next;
      return *this;
    }

    iterator operator++(int)
    {
      iterator tmp = *this;
      ++(*this);
      return tmp;
    }

    bool operator==(const iterator& other) const
    {
      return m_current == other.m_current;
    }

    bool operator!=(const iterator& other) const
    {
      return m_current != other.m_current;
    }

  private:
    friend class SList;
    node_type* m_current;
  };

  class const_iterator
  {
  public:
    using difference_type = std::ptrdiff_t;
    using value_type = const T;
    using pointer = const T*;
    using reference = const T&;
    using iterator_category = std::forward_iterator_tag;

    const_iterator()
        : m_current(nullptr)
    {
    }
    explicit const_iterator(const node_type* node)
        : m_current(node)
    {
    }
    const_iterator(const iterator& other)
        : m_current(other.m_current)
    {
    }

    reference operator*() const { return m_current->data; }
    pointer operator->() const { return &m_current->data; }

    const_iterator& operator++()
    {
      if (m_current)
        m_current = m_current->next;
      return *this;
    }

    const_iterator operator++(int)
    {
      const_iterator tmp = *this;
      ++(*this);
      return tmp;
    }

    bool operator==(const const_iterator& other) const
    {
      return m_current == other.m_current;
    }

    bool operator!=(const const_iterator& other) const
    {
      return m_current != other.m_current;
    }

  private:
    const node_type* m_current;
  };

private:
  node_type* m_head;
};

/**
 * @brief Doubly-linked list with STL-like interface
 *
 * Provides O(1) insertion/deletion at both ends and O(n) elsewhere.
 * Supports bidirectional iteration.
 * Caller controls node allocation - list only manages pointers.
 * This allows custom allocators, stack allocation, or pre-allocated pools.
 */
template <typename T> class DTailList
{
public:
  using value_type = T;
  using pointer = T*;
  using reference = T&;
  using const_reference = const T&;
  using node_type = DNode<T>;

  class iterator;
  class const_iterator;

  DTailList()
      : m_head(nullptr), m_tail(nullptr)
  {
  }

  ~DTailList() = default;

  // Prevent copying
  DTailList(const DTailList&) = delete;
  DTailList& operator=(const DTailList&) = delete;

  // Allow moving
  DTailList(DTailList&& other) noexcept
      : m_head(other.m_head), m_tail(other.m_tail)
  {
    other.m_head = nullptr;
    other.m_tail = nullptr;
  }

  DTailList& operator=(DTailList&& other) noexcept
  {
    if (this != &other) {
      m_head = other.m_head;
      m_tail = other.m_tail;
      other.m_head = nullptr;
      other.m_tail = nullptr;
    }
    return *this;
  }

  /**
   * @brief Insert node at front in O(1)
   * Caller owns the node and is responsible for deallocation
   */
  void push_front(node_type* node)
  {
    assert(node && "Node is null");

    node->next = m_head;
    node->prev = nullptr;

    if (m_head) {
      m_head->prev = node;
    } else {
      m_tail = node;
    }
    m_head = node;
  }

  /**
   * @brief Insert node at back in O(1)
   * Caller owns the node and is responsible for deallocation
   */
  void push_back(node_type* node)
  {
    assert(node && "Node is null");
    node->next = nullptr;
    node->prev = m_tail;

    if (m_tail)
      m_tail->next = node;
    else
      m_head = node;

    m_tail = node;
  }

  /**
   * @brief Remove node at front in O(1)
   * Returns the removed node (caller must delete it)
   */
  node_type* pop_front()
  {
    assert(m_head && "List is empty");
    auto* temp = m_head;
    m_head = m_head->next;

    if (m_head)
      m_head->prev = nullptr;
    else
      m_tail = nullptr;

    temp->next = nullptr;
    temp->prev = nullptr;
    return temp;
  }

  /**
   * @brief Remove node at back in O(1)
   * Returns the removed node (caller must delete it)
   */
  node_type* pop_back()
  {
    assert(m_tail && "List is empty");
    auto* temp = m_tail;
    m_tail = m_tail->prev;

    if (m_tail)
      m_tail->next = nullptr;
    else
      m_head = nullptr;

    temp->next = nullptr;
    temp->prev = nullptr;
    return temp;
  }

  /**
   * @brief Access front element
   */
  reference front()
  {
    assert(m_head && "List is empty");
    return m_head->data;
  }
  const_reference front() const
  {
    assert(m_head && "List is empty");
    return m_head->data;
  }

  /**
   * @brief Access back element
   */
  reference back()
  {
    assert(m_tail && "List is empty");
    return m_tail->data;
  }
  const_reference back() const
  {
    assert(m_tail && "List is empty");
    return m_tail->data;
  }

  /**
   * @brief Check if list is empty
   */
  bool empty() const { return m_head == nullptr; }

  /**
   * @brief Get number of elements
   */
  size_t size() const
  {
    size_t count = 0;
    for (auto* curr = m_head; curr != nullptr; curr = curr->next) {
      ++count;
    }
    return count;
  }

  /**
   * @brief Unlink all nodes from list (does NOT delete them)
   * Caller is responsible for deleting nodes
   */
  void clear()
  {
    m_head = nullptr;
    m_tail = nullptr;
  }

  /**
   * @brief Insert node before iterator position in O(1)
   * Caller owns the node and is responsible for deallocation
   */
  iterator insert(iterator pos, node_type* node)
  {
    assert(node && "Node is null");

    if (pos.m_current == nullptr) {
      push_back(node);
      return iterator(m_tail);
    }

    node->next = pos.m_current;
    node->prev = pos.m_current->prev;

    if (pos.m_current->prev) {
      pos.m_current->prev->next = node;
    } else {
      m_head = node;
    }
    pos.m_current->prev = node;
    return iterator(node);
  }

  /**
   * @brief Remove node at iterator position in O(1)
   * Returns the removed node (caller must delete it)
   */
  node_type* erase(iterator pos)
  {
    assert(pos.m_current && "Iterator is invalid");

    node_type* temp = pos.m_current;
    node_type* next = pos.m_current->next;

    if (pos.m_current->prev) {
      pos.m_current->prev->next = pos.m_current->next;
    } else {
      m_head = pos.m_current->next;
    }

    if (pos.m_current->next) {
      pos.m_current->next->prev = pos.m_current->prev;
    } else {
      m_tail = pos.m_current->prev;
    }

    temp->next = nullptr;
    temp->prev = nullptr;
    return temp;
  }

  // Iterators
  iterator begin() { return iterator(m_head); }
  const_iterator begin() const { return const_iterator(m_head); }
  const_iterator cbegin() const { return const_iterator(m_head); }

  iterator end() { return iterator(nullptr); }
  const_iterator end() const { return const_iterator(nullptr); }
  const_iterator cend() const { return const_iterator(nullptr); }

  class iterator
  {
  public:
    using difference_type = std::ptrdiff_t;
    using value_type = T;
    using pointer = T*;
    using reference = T&;
    using iterator_category = std::bidirectional_iterator_tag;

    iterator()
        : m_current(nullptr)
    {
    }
    explicit iterator(node_type* node)
        : m_current(node)
    {
    }

    reference operator*() { return m_current->data; }
    pointer operator->() { return &m_current->data; }

    iterator& operator++()
    {
      if (m_current)
        m_current = m_current->next;
      return *this;
    }

    iterator operator++(int)
    {
      iterator tmp = *this;
      ++(*this);
      return tmp;
    }

    iterator& operator--()
    {
      if (m_current)
        m_current = m_current->prev;
      return *this;
    }

    iterator operator--(int)
    {
      iterator tmp = *this;
      --(*this);
      return tmp;
    }

    bool operator==(const iterator& other) const
    {
      return m_current == other.m_current;
    }

    bool operator!=(const iterator& other) const
    {
      return m_current != other.m_current;
    }

  private:
    friend class DTailList;
    node_type* m_current;
  };

  class const_iterator
  {
  public:
    using difference_type = std::ptrdiff_t;
    using value_type = const T;
    using pointer = const T*;
    using reference = const T&;
    using iterator_category = std::bidirectional_iterator_tag;

    const_iterator()
        : m_current(nullptr)
    {
    }
    explicit const_iterator(const node_type* node)
        : m_current(node)
    {
    }
    const_iterator(const iterator& other)
        : m_current(other.m_current)
    {
    }

    reference operator*() const { return m_current->data; }
    pointer operator->() const { return &m_current->data; }

    const_iterator& operator++()
    {
      if (m_current)
        m_current = m_current->next;
      return *this;
    }

    const_iterator operator++(int)
    {
      const_iterator tmp = *this;
      ++(*this);
      return tmp;
    }

    const_iterator& operator--()
    {
      if (m_current)
        m_current = m_current->prev;
      return *this;
    }

    const_iterator operator--(int)
    {
      const_iterator tmp = *this;
      --(*this);
      return tmp;
    }

    bool operator==(const const_iterator& other) const
    {
      return m_current == other.m_current;
    }

    bool operator!=(const const_iterator& other) const
    {
      return m_current != other.m_current;
    }

  private:
    const node_type* m_current;
  };

private:
  node_type* m_head;
  node_type* m_tail;
};

// Singly-linked list variant that keeps a tail pointer for O(1) push_back.
// pop_back remains O(n) because nodes are singly-linked.
template <typename T> class STailList
{
public:
  using value_type = T;
  using pointer = T*;
  using reference = T&;
  using const_reference = const T&;
  using node_type = SNode<T>;

  class iterator;
  class const_iterator;

  STailList()
      : m_head(nullptr), m_tail(nullptr)
  {
  }

  ~STailList() = default;

  STailList(const STailList&) = delete;
  STailList& operator=(const STailList&) = delete;

  STailList(STailList&& other) noexcept
      : m_head(other.m_head), m_tail(other.m_tail)
  {
    other.m_head = nullptr;
    other.m_tail = nullptr;
  }

  STailList& operator=(STailList&& other) noexcept
  {
    if (this != &other) {
      m_head = other.m_head;
      m_tail = other.m_tail;
      other.m_head = nullptr;
      other.m_tail = nullptr;
    }
    return *this;
  }

  void push_front(node_type* node)
  {
    assert(node && "Node is null");
    node->next = m_head;
    m_head = node;
    if (!m_tail)
      m_tail = node;
  }

  void push_back(node_type* node)
  {
    assert(node && "Node is null");
    node->next = nullptr;
    if (m_tail) {
      m_tail->next = node;
      m_tail = node;
    } else {
      m_head = m_tail = node;
    }
  }

  node_type* pop_front()
  {
    assert(m_head && "List is empty");

    auto* temp = m_head;
    m_head = m_head->next;

    if (!m_head)
      m_tail = nullptr;

    temp->next = nullptr;
    return temp;
  }

  // O(n) for singly list (no prev pointer)
  node_type* pop_back()
  {
    assert(m_head && "List is empty");
    if (m_head == m_tail) {
      auto* temp = m_head;
      m_head = m_tail = nullptr;
      return temp;
    }
    node_type* prev = m_head;
    while (prev->next != m_tail)
      prev = prev->next;
    auto* temp = m_tail;
    prev->next = nullptr;
    m_tail = prev;
    return temp;
  }

  reference front()
  {
    assert(m_head && "List is empty");
    return m_head->data;
  }
  const_reference front() const
  {
    assert(m_head && "List is empty");
    return m_head->data;
  }

  reference back()
  {
    assert(m_tail && "List is empty");
    return m_tail->data;
  }
  const_reference back() const
  {
    assert(m_tail && "List is empty");
    return m_tail->data;
  }

  bool empty() const { return m_head == nullptr; }

  size_t size() const
  {
    size_t count = 0;
    for (auto* curr = m_head; curr != nullptr; curr = curr->next) {
      ++count;
    }
    return count;
  }

  void clear() { m_head = m_tail = nullptr; }

  iterator insert_after(iterator pos, node_type* node)
  {
    assert(node && "Node is null");

    if (pos.m_current) {
      node->next = pos.m_current->next;
      pos.m_current->next = node;
      if (!node->next)
        m_tail = node;
    } else {
      node->next = m_head;
      m_head = node;
      if (!m_tail)
        m_tail = node;
    }
    return iterator(node);
  }

  node_type* erase_after(iterator pos)
  {
    assert(pos.m_current && "Iterator is invalid");
    assert(pos.m_current->next && "Iterator is at end");
    auto* temp = pos.m_current->next;
    pos.m_current->next = temp->next;
    if (!pos.m_current->next)
      m_tail = pos.m_current;
    temp->next = nullptr;
    return temp;
  }

  iterator begin() { return iterator(m_head); }
  const_iterator begin() const { return const_iterator(m_head); }
  const_iterator cbegin() const { return const_iterator(m_head); }

  iterator end() { return iterator(nullptr); }
  const_iterator end() const { return const_iterator(nullptr); }
  const_iterator cend() const { return const_iterator(nullptr); }

  class iterator
  {
  public:
    using difference_type = std::ptrdiff_t;
    using value_type = T;
    using pointer = T*;
    using reference = T&;
    using iterator_category = std::forward_iterator_tag;

    iterator()
        : m_current(nullptr)
    {
    }
    explicit iterator(node_type* node)
        : m_current(node)
    {
    }

    reference operator*() { return m_current->data; }
    pointer operator->() { return &m_current->data; }

    iterator& operator++()
    {
      if (m_current)
        m_current = m_current->next;
      return *this;
    }

    iterator operator++(int)
    {
      iterator tmp = *this;
      ++(*this);
      return tmp;
    }

    bool operator==(const iterator& other) const
    {
      return m_current == other.m_current;
    }
    bool operator!=(const iterator& other) const
    {
      return m_current != other.m_current;
    }

  private:
    friend class STailList;
    node_type* m_current;
  };

  class const_iterator
  {
  public:
    using difference_type = std::ptrdiff_t;
    using value_type = const T;
    using pointer = const T*;
    using reference = const T&;
    using iterator_category = std::forward_iterator_tag;

    const_iterator()
        : m_current(nullptr)
    {
    }
    explicit const_iterator(const node_type* node)
        : m_current(node)
    {
    }
    const_iterator(const iterator& other)
        : m_current(other.m_current)
    {
    }

    reference operator*() const { return m_current->data; }
    pointer operator->() const { return &m_current->data; }

    const_iterator& operator++()
    {
      if (m_current)
        m_current = m_current->next;
      return *this;
    }

    const_iterator operator++(int)
    {
      const_iterator tmp = *this;
      ++(*this);
      return tmp;
    }

    bool operator==(const const_iterator& other) const
    {
      return m_current == other.m_current;
    }
    bool operator!=(const const_iterator& other) const
    {
      return m_current != other.m_current;
    }

  private:
    const node_type* m_current;
  };

private:
  node_type* m_head;
  node_type* m_tail;
};

// Doubly-linked list variant WITHOUT a tail pointer. Operations that would
// normally use the tail become O(n) (e.g., push_back, pop_back, back()).
template <typename T> class DList
{
public:
  using value_type = T;
  using pointer = T*;
  using reference = T&;
  using const_reference = const T&;
  using node_type = DNode<T>;

  class iterator;
  class const_iterator;

  DList()
      : m_head(nullptr)
  {
  }

  ~DList() = default;

  DList(const DList&) = delete;
  DList& operator=(const DList&) = delete;

  DList(DList&& other) noexcept
      : m_head(other.m_head)
  {
    other.m_head = nullptr;
  }

  DList& operator=(DList&& other) noexcept
  {
    if (this != &other) {
      m_head = other.m_head;
      other.m_head = nullptr;
    }
    return *this;
  }

  void push_front(node_type* node)
  {
    assert(node && "Node is null");

    node->next = m_head;
    node->prev = nullptr;

    if (m_head)
      m_head->prev = node;

    m_head = node;
  }

  // O(n) because there's no stored tail
  void push_back(node_type* node)
  {
    assert(node && "Node is null");

    node->next = nullptr;

    if (!m_head) {
      node->prev = nullptr;
      m_head = node;
      return;
    }

    node_type* curr = m_head;
    while (curr->next)
      curr = curr->next;
    curr->next = node;
    node->prev = curr;
  }

  node_type* pop_front()
  {
    assert(m_head && "List is empty");
    auto* temp = m_head;
    m_head = m_head->next;
    if (m_head)
      m_head->prev = nullptr;
    temp->next = nullptr;
    temp->prev = nullptr;
    return temp;
  }

  // O(n) because there's no stored tail
  node_type* pop_back()
  {
    assert(m_head && "List is empty");
    if (!m_head->next) {
      auto* temp = m_head;
      m_head = nullptr;
      return temp;
    }
    node_type* curr = m_head;
    while (curr->next)
      curr = curr->next;
    node_type* prev = curr->prev;
    prev->next = nullptr;
    curr->prev = nullptr;
    return curr;
  }

  reference front()
  {
    assert(m_head && "List is empty");
    return m_head->data;
  }
  const_reference front() const
  {
    assert(m_head && "List is empty");
    return m_head->data;
  }

  // O(n)
  reference back()
  {
    assert(m_head && "List is empty");
    node_type* curr = m_head;
    while (curr->next)
      curr = curr->next;
    return curr->data;
  }
  const_reference back() const
  {
    assert(m_head && "List is empty");
    node_type* curr = m_head;
    while (curr->next)
      curr = curr->next;
    return curr->data;
  }

  bool empty() const { return m_head == nullptr; }

  size_t size() const
  {
    size_t count = 0;

    for (auto* curr = m_head; curr != nullptr; curr = curr->next)
      ++count;

    return count;
  }

  void clear() { m_head = nullptr; }

  iterator insert(iterator pos, node_type* node)
  {
    assert(node && "Node is null");
    if (pos.m_current == nullptr) {
      push_back(node);
      // find the new tail to return iterator; O(n)
      node_type* curr = m_head;
      while (curr && curr->next)
        curr = curr->next;
      return iterator(curr);
    }

    node->next = pos.m_current;
    node->prev = pos.m_current->prev;

    if (pos.m_current->prev) {
      pos.m_current->prev->next = node;
    } else {
      m_head = node;
    }
    pos.m_current->prev = node;
    return iterator(node);
  }

  node_type* erase(iterator pos)
  {
    assert(pos.m_current && "Iterator is invalid");

    node_type* temp = pos.m_current;

    if (pos.m_current->prev) {
      pos.m_current->prev->next = pos.m_current->next;
    } else {
      m_head = pos.m_current->next;
    }

    if (pos.m_current->next) {
      pos.m_current->next->prev = pos.m_current->prev;
    }

    temp->next = nullptr;
    temp->prev = nullptr;
    return temp;
  }

  iterator begin() { return iterator(m_head); }
  const_iterator begin() const { return const_iterator(m_head); }
  const_iterator cbegin() const { return const_iterator(m_head); }

  iterator end() { return iterator(nullptr); }
  const_iterator end() const { return const_iterator(nullptr); }
  const_iterator cend() const { return const_iterator(nullptr); }

  class iterator
  {
  public:
    using difference_type = std::ptrdiff_t;
    using value_type = T;
    using pointer = T*;
    using reference = T&;
    using iterator_category = std::bidirectional_iterator_tag;

    iterator()
        : m_current(nullptr)
    {
    }
    explicit iterator(node_type* node)
        : m_current(node)
    {
    }

    reference operator*() { return m_current->data; }
    pointer operator->() { return &m_current->data; }

    iterator& operator++()
    {
      if (m_current)
        m_current = m_current->next;
      return *this;
    }

    iterator operator++(int)
    {
      iterator tmp = *this;
      ++(*this);
      return tmp;
    }

    iterator& operator--()
    {
      if (m_current)
        m_current = m_current->prev;
      return *this;
    }

    iterator operator--(int)
    {
      iterator tmp = *this;
      --(*this);
      return tmp;
    }

    bool operator==(const iterator& other) const
    {
      return m_current == other.m_current;
    }
    bool operator!=(const iterator& other) const
    {
      return m_current != other.m_current;
    }

  private:
    friend class DList;
    node_type* m_current;
  };

  class const_iterator
  {
  public:
    using difference_type = std::ptrdiff_t;
    using value_type = const T;
    using pointer = const T*;
    using reference = const T&;
    using iterator_category = std::bidirectional_iterator_tag;

    const_iterator()
        : m_current(nullptr)
    {
    }
    explicit const_iterator(const node_type* node)
        : m_current(node)
    {
    }
    const_iterator(const iterator& other)
        : m_current(other.m_current)
    {
    }

    reference operator*() const { return m_current->data; }
    pointer operator->() const { return &m_current->data; }

    const_iterator& operator++()
    {
      if (m_current)
        m_current = m_current->next;
      return *this;
    }

    const_iterator operator++(int)
    {
      const_iterator tmp = *this;
      ++(*this);
      return tmp;
    }

    const_iterator& operator--()
    {
      if (m_current)
        m_current = m_current->prev;
      return *this;
    }

    const_iterator operator--(int)
    {
      const_iterator tmp = *this;
      --(*this);
      return tmp;
    }

    bool operator==(const const_iterator& other) const
    {
      return m_current == other.m_current;
    }
    bool operator!=(const const_iterator& other) const
    {
      return m_current != other.m_current;
    }

  private:
    const node_type* m_current;
  };

private:
  node_type* m_head;
};
} // namespace freertos::helpers