#pragma once

#include <cassert>
#include <concepts>
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

  template <typename... Args>
    requires std::constructible_from<T, Args...>
  explicit SNode(Args&&... args)
      : data(std::forward<Args>(args)...)
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

  template <typename... Args>
    requires std::constructible_from<T, Args...>
  explicit DNode(Args&&... args)
      : data(std::forward<Args>(args)...)
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
  SList()
      : m_head(nullptr)
  {
  }

  /// @brief Destructor does NOT delete nodes - caller is responsible for node
  /// memory management
  ~SList() = default;

  // Allow copying (shallow copy of pointers, caller must manage node lifetimes)
  SList(const SList&) = default;
  SList& operator=(const SList&) = default;

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
    node_type* removed = m_head;
    m_head = m_head->next;

    removed->next = nullptr;
    return removed;
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

    for (node_type* curr = m_head; curr != nullptr; curr = curr->next)
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
  iterator insert_after(const_iterator pos, node_type* node)
  {
    assert(node && "Node is null");

    if (pos.m_current) {
      node_type* current = const_cast<node_type*>(pos.m_current);
      node->next = current->next;
      current->next = node;
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
  node_type* erase_after(const_iterator pos)
  {
    assert(pos.m_current && "Iterator is invalid");
    assert(pos.m_current->next && "Iterator is at end");

    node_type* current = const_cast<node_type*>(pos.m_current);
    node_type* removed = current->next;
    current->next = removed->next;
    removed->next = nullptr;
    return removed;
  }

  /**
   * @brief Swap contents with another SList
   */
  void swap(SList& other) noexcept
  {
    using std::swap;
    swap(m_head, other.m_head);
  }

  /**
   * @brief Get maximum size
   */
  size_t max_size() const { return static_cast<size_t>(-1); }

  /**
   * @brief Reverse the order of elements
   */
  void reverse() noexcept
  {
    node_type* prev = nullptr;
    node_type* current = m_head;
    while (current) {
      node_type* next = current->next;
      current->next = prev;
      prev = current;
      current = next;
    }
    m_head = prev;
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
    using iterator_concept = std::forward_iterator_tag;
    using iterator_category = std::forward_iterator_tag;

    iterator()
        : m_current(nullptr)
    {
    }
    explicit iterator(node_type* node)
        : m_current(node)
    {
    }

    reference operator*() const { return m_current->data; }
    pointer operator->() const { return &m_current->data; }

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
    using value_type = T;
    using pointer = const T*;
    using reference = const T&;
    using iterator_concept = std::forward_iterator_tag;
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
    friend class SList;
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
  using size_type = std::size_t;
  using node_type = DNode<T>;

  class iterator;
  class const_iterator;

  using reverse_iterator = std::reverse_iterator<iterator>;
  using const_reverse_iterator = std::reverse_iterator<const_iterator>;

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

    if (m_head)
      m_head->prev = node;
    else
      m_tail = node;
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
    node_type* removed = m_head;
    m_head = m_head->next;

    if (m_head)
      m_head->prev = nullptr;
    else
      m_tail = nullptr;

    removed->next = nullptr;
    removed->prev = nullptr;
    return removed;
  }

  /**
   * @brief Remove node at back in O(1)
   * Returns the removed node (caller must delete it)
   */
  node_type* pop_back()
  {
    assert(m_tail && "List is empty");
    node_type* removed = m_tail;
    m_tail = m_tail->prev;

    if (m_tail)
      m_tail->next = nullptr;
    else
      m_head = nullptr;

    removed->prev = removed->next = nullptr;
    return removed;
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
   * @brief Get maximum size
   */
  size_type max_size() const { return static_cast<size_type>(-1); }

  /**
   * @brief Get number of elements
   */
  size_t size() const
  {
    size_t count = 0;
    for (node_type* curr = m_head; curr != nullptr; curr = curr->next)
      ++count;
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
  iterator insert(const_iterator pos, node_type* node)
  {
    assert(node && "Node is null");

    if (pos.m_current == nullptr) {
      push_back(node);
      return iterator(m_tail);
    }

    node->next = pos.m_current;
    node->prev = pos.m_current->prev;

    if (pos.m_current->prev)
      pos.m_current->prev->next = node;
    else
      m_head = node;

    pos.m_current->prev = node;
    return iterator(node);
  }

  /**
   * @brief Remove node at iterator position in O(1)
   * Returns the removed node (caller must delete it)
   */
  node_type* erase(const_iterator pos)
  {
    assert(pos.m_current && "Iterator is invalid");

    node_type* removed = const_cast<node_type*>(pos.m_current);

    if (removed->prev) {
      removed->prev->next = removed->next;
    } else {
      m_head = removed->next;
    }

    if (removed->next) {
      removed->next->prev = removed->prev;
    } else {
      m_tail = removed->prev;
    }

    removed->next = nullptr;
    removed->prev = nullptr;
    return removed;
  }

  /**
   * @brief Swap contents with another DTailList
   */
  void swap(DTailList& other) noexcept
  {
    using std::swap;
    swap(m_head, other.m_head);
    swap(m_tail, other.m_tail);
  }

  /**
   * @brief Reverse the order of elements
   */
  void reverse() noexcept
  {
    m_tail = m_head;

    node_type* current = m_head;
    node_type* previous = nullptr;
    while (current) {
      node_type* next = current->next;
      current->next = previous;
      current->prev = next;
      previous = current;
      current = next;
    }
    m_head = previous;
  }

  // Iterators
  iterator begin() { return iterator(m_head); }
  const_iterator begin() const { return const_iterator(m_head, this); }
  const_iterator cbegin() const { return const_iterator(m_head, this); }

  reverse_iterator rbegin() { return reverse_iterator(end()); }
  const_reverse_iterator rbegin() const
  {
    return const_reverse_iterator(end());
  }
  const_reverse_iterator crbegin() const
  {
    return const_reverse_iterator(cend());
  }

  iterator end() { return iterator(nullptr, this); }
  const_iterator end() const { return const_iterator(nullptr, this); }
  const_iterator cend() const { return const_iterator(nullptr, this); }

  reverse_iterator rend() { return reverse_iterator(begin()); }
  const_reverse_iterator rend() const
  {
    return const_reverse_iterator(begin());
  }
  const_reverse_iterator crend() const
  {
    return const_reverse_iterator(cbegin());
  }

  class iterator
  {
  public:
    using difference_type = std::ptrdiff_t;
    using value_type = T;
    using pointer = T*;
    using reference = T&;
    using iterator_concept = std::bidirectional_iterator_tag;
    using iterator_category = std::bidirectional_iterator_tag;

    iterator()
        : m_current(nullptr), m_owner(nullptr)
    {
    }
    explicit iterator(node_type* node, const DTailList* owner = nullptr)
        : m_current(node), m_owner(owner)
    {
    }

    reference operator*() const { return m_current->data; }
    pointer operator->() const { return &m_current->data; }

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
      if (m_current) {
        m_current = m_current->prev;
      } else if (m_owner) {
        m_current = m_owner->m_tail;
      }
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
    friend class const_iterator;
    node_type* m_current;
    const DTailList* m_owner;
  };

  class const_iterator
  {
  public:
    using difference_type = std::ptrdiff_t;
    using value_type = T;
    using pointer = const T*;
    using reference = const T&;
    using iterator_concept = std::bidirectional_iterator_tag;
    using iterator_category = std::bidirectional_iterator_tag;

    const_iterator()
        : m_current(nullptr), m_owner(nullptr)
    {
    }
    explicit const_iterator(const node_type* node,
                            const DTailList* owner = nullptr)
        : m_current(node), m_owner(owner)
    {
    }
    const_iterator(const iterator& other)
        : m_current(other.m_current), m_owner(other.m_owner)
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
      if (m_current) {
        m_current = m_current->prev;
      } else if (m_owner) {
        m_current = m_owner->m_tail;
      }
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
    friend class DTailList;
    const node_type* m_current;
    const DTailList* m_owner;
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

    node_type* removed = m_head;
    m_head = m_head->next;

    if (!m_head)
      m_tail = nullptr;

    removed->next = nullptr;
    return removed;
  }

  // O(n) for singly list (no prev pointer)
  node_type* pop_back()
  {
    assert(m_head && "List is empty");
    if (m_head == m_tail) {
      node_type* removed = m_head;
      m_head = m_tail = nullptr;
      return removed;
    }
    node_type* prev = m_head;
    while (prev->next != m_tail)
      prev = prev->next;
    node_type* removed = m_tail;
    prev->next = nullptr;
    m_tail = prev;
    return removed;
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
    for (node_type* curr = m_head; curr != nullptr; curr = curr->next)
      ++count;
    return count;
  }

  void clear() { m_head = m_tail = nullptr; }

  iterator insert_after(const_iterator pos, node_type* node)
  {
    assert(node && "Node is null");

    if (pos.m_current) {
      node_type* current = const_cast<node_type*>(pos.m_current);
      node->next = current->next;
      current->next = node;
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

  node_type* erase_after(const_iterator pos)
  {
    assert(pos.m_current && "Iterator is invalid");
    assert(pos.m_current->next && "Iterator is at end");
    node_type* current = const_cast<node_type*>(pos.m_current);
    node_type* removed = current->next;
    current->next = removed->next;
    if (!current->next)
      m_tail = current;
    removed->next = nullptr;
    return removed;
  }

  /**
   * @brief Swap contents with another STailList
   */
  void swap(STailList& other) noexcept
  {
    using std::swap;
    swap(m_head, other.m_head);
    swap(m_tail, other.m_tail);
  }

  /**
   * @brief Get maximum size
   */
  size_t max_size() const { return static_cast<size_t>(-1); }

  /**
   * @brief Reverse the order of elements
   */
  void reverse() noexcept
  {
    m_tail = m_head;
    node_type* prev = nullptr;
    node_type* current = m_head;
    while (current) {
      node_type* next = current->next;
      current->next = prev;
      prev = current;
      current = next;
    }
    m_head = prev;
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
    using iterator_concept = std::forward_iterator_tag;
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
    friend class const_iterator;
    node_type* m_current;
  };

  class const_iterator
  {
  public:
    using difference_type = std::ptrdiff_t;
    using value_type = T;
    using pointer = const T*;
    using reference = const T&;
    using iterator_concept = std::forward_iterator_tag;
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
    friend class STailList;
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
  using size_type = std::size_t;
  using node_type = DNode<T>;

  class iterator;
  class const_iterator;

  using reverse_iterator = std::reverse_iterator<iterator>;
  using const_reverse_iterator = std::reverse_iterator<const_iterator>;

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
    node_type* removed = m_head;
    m_head = m_head->next;
    if (m_head)
      m_head->prev = nullptr;
    removed->next = nullptr;
    removed->prev = nullptr;
    return removed;
  }

  // O(n) because there's no stored tail
  node_type* pop_back()
  {
    assert(m_head && "List is empty");
    if (!m_head->next) {
      node_type* removed = m_head;
      m_head = nullptr;
      removed->prev = removed->next = nullptr;
      return removed;
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

  /**
   * @brief Get maximum size
   */
  size_type max_size() const { return static_cast<size_type>(-1); }

  size_t size() const
  {
    size_t count = 0;

    for (node_type* curr = m_head; curr != nullptr; curr = curr->next)
      ++count;

    return count;
  }

  void clear() { m_head = nullptr; }

  iterator insert(const_iterator pos, node_type* node)
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

    node_type* current = const_cast<node_type*>(pos.m_current);

    node->next = current;
    node->prev = current->prev;

    if (current->prev) {
      current->prev->next = node;
    } else {
      m_head = node;
    }
    current->prev = node;
    return iterator(node);
  }

  node_type* erase(const_iterator pos)
  {
    assert(pos.m_current && "Iterator is invalid");

    node_type* removed = const_cast<node_type*>(pos.m_current);

    if (removed->prev) {
      removed->prev->next = removed->next;
    } else {
      m_head = removed->next;
    }

    if (removed->next) {
      removed->next->prev = removed->prev;
    }

    removed->next = nullptr;
    removed->prev = nullptr;
    return removed;
  }

  /**
   * @brief Swap contents with another DList
   */
  void swap(DList& other) noexcept
  {
    using std::swap;
    swap(m_head, other.m_head);
  }

  /**
   * @brief Reverse the order of elements
   */
  void reverse() noexcept
  {
    node_type* current = m_head;
    node_type* new_head = nullptr;

    while (current) {
      node_type* next = current->next;
      std::swap(current->next, current->prev);
      new_head = current;
      current = next;
    }

    m_head = new_head;
  }

  iterator begin() { return iterator(m_head, this); }
  const_iterator begin() const { return const_iterator(m_head, this); }
  const_iterator cbegin() const { return const_iterator(m_head, this); }

  reverse_iterator rbegin() { return reverse_iterator(end()); }
  const_reverse_iterator rbegin() const
  {
    return const_reverse_iterator(end());
  }
  const_reverse_iterator crbegin() const
  {
    return const_reverse_iterator(cend());
  }

  iterator end() { return iterator(nullptr, this); }
  const_iterator end() const { return const_iterator(nullptr, this); }
  const_iterator cend() const { return const_iterator(nullptr, this); }

  reverse_iterator rend() { return reverse_iterator(begin()); }
  const_reverse_iterator rend() const
  {
    return const_reverse_iterator(begin());
  }
  const_reverse_iterator crend() const
  {
    return const_reverse_iterator(cbegin());
  }

  class iterator
  {
  public:
    using difference_type = std::ptrdiff_t;
    using value_type = T;
    using pointer = T*;
    using reference = T&;
    using iterator_concept = std::bidirectional_iterator_tag;
    using iterator_category = std::bidirectional_iterator_tag;

    iterator()
        : m_current(nullptr), m_owner(nullptr)
    {
    }
    explicit iterator(node_type* node, const DList* owner = nullptr)
        : m_current(node), m_owner(owner)
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
      if (m_current) {
        m_current = m_current->prev;
      } else if (m_owner) {
        m_current = m_owner->tail_node();
      }
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
    friend class const_iterator;
    node_type* m_current;
    const DList* m_owner;
  };

  class const_iterator
  {
  public:
    using difference_type = std::ptrdiff_t;
    using value_type = T;
    using pointer = const T*;
    using reference = const T&;
    using iterator_concept = std::bidirectional_iterator_tag;
    using iterator_category = std::bidirectional_iterator_tag;

    const_iterator()
        : m_current(nullptr), m_owner(nullptr)
    {
    }
    explicit const_iterator(const node_type* node, const DList* owner = nullptr)
        : m_current(node), m_owner(owner)
    {
    }
    const_iterator(const iterator& other)
        : m_current(other.m_current), m_owner(other.m_owner)
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
      if (m_current) {
        m_current = m_current->prev;
      } else if (m_owner) {
        m_current = m_owner->tail_node();
      }
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
    friend class DList;
    const node_type* m_current;
    const DList* m_owner;
  };

private:
  node_type* m_head;

  node_type* tail_node() const
  {
    node_type* curr = m_head;
    while (curr && curr->next)
      curr = curr->next;
    return curr;
  }
};
} // namespace freertos::helpers