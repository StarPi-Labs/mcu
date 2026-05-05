#include "ConditionVariable.h"

namespace freertos::implementation
{

PriorityWaitersQueue::iterator::iterator()
    : m_owner(nullptr), m_priority(configMAX_PRIORITIES), m_inner()
{
}

PriorityWaitersQueue::iterator::iterator(PriorityWaitersQueue* owner,
                                         std::size_t priority,
                                         DTailList<CVWaiter>::iterator inner)
    : m_owner(owner), m_priority(priority), m_inner(inner)
{
}

PriorityWaitersQueue::iterator::reference
PriorityWaitersQueue::iterator::operator*() const
{
  return *m_inner;
}

PriorityWaitersQueue::iterator::pointer
PriorityWaitersQueue::iterator::operator->() const
{
  return &(*m_inner);
}

PriorityWaitersQueue::iterator& PriorityWaitersQueue::iterator::operator++()
{
  // If the iterator is already at end, it stays there
  if (m_owner == nullptr || m_priority >= configMAX_PRIORITIES)
    return *this;

  ++m_inner;

  auto bucketEnd = m_owner->m_lists[m_priority].end();

  if (m_inner == bucketEnd)
    advanceToNextValid();

  return *this;
}

PriorityWaitersQueue::iterator PriorityWaitersQueue::iterator::operator++(int)
{
  iterator tmp = *this;
  ++(*this);
  return tmp;
}

bool PriorityWaitersQueue::iterator::operator==(const iterator& other) const
{
  // Iterators from different containers are never equal
  if (m_owner != other.m_owner)
    return false;

  // Both iterators are end iterators
  if (m_priority >= configMAX_PRIORITIES &&
      other.m_priority >= configMAX_PRIORITIES)
    return true;

  // Otherwise, both priority and position must be the same
  return m_priority == other.m_priority && m_inner == other.m_inner;
}

bool PriorityWaitersQueue::iterator::operator!=(const iterator& other) const
{
  return !(*this == other);
}

void PriorityWaitersQueue::iterator::advanceToNextValid()
{
  configASSERT(m_owner != nullptr && "Called on end iterator");

  for (std::size_t i = 1; i <= m_priority; ++i) {
    // This way it doesn't underflow
    std::size_t nextPriority = m_priority - i;

    if (!m_owner->m_lists[nextPriority].empty()) {
      m_priority = nextPriority;
      m_inner = m_owner->m_lists[nextPriority].begin();
      return;
    }
  }

  // No more valid priorities
  m_priority = configMAX_PRIORITIES;
  m_inner = DTailList<CVWaiter>::iterator();
}

bool PriorityWaitersQueue::empty() const
{
  for (const auto& list : m_lists) {
    if (!list.empty())
      return false;
  }
  return true;
}

PriorityWaitersQueue::iterator
PriorityWaitersQueue::enqueue(PriorityWaitersQueue::node_type* node)
{
  configASSERT(node != nullptr && "Node is null");

  auto priority = uxTaskPriorityGet(NULL);

  auto& list = m_lists[priority];
  list.push_back(node);

  auto inner = list.end();
  --inner;

  return iterator(this, priority, inner);
}

PriorityWaitersQueue::reference PriorityWaitersQueue::peek()
{
  for (std::size_t i = 1; i <= configMAX_PRIORITIES; ++i) {
    std::size_t priority = configMAX_PRIORITIES - i;

    if (!m_lists[priority].empty())
      return m_lists[priority].front();
  }

  configASSERT(false && "Queue is empty");
  // Unreachable
  return m_lists[0].front();
}

void PriorityWaitersQueue::remove(iterator it)
{
  configASSERT(it.m_owner == this);
  configASSERT(it.m_priority < configMAX_PRIORITIES);
  m_lists[it.m_priority].erase(it.m_inner);
}

PriorityWaitersQueue::iterator PriorityWaitersQueue::begin()
{
  // Start from the highest (non valid) priority
  auto result =
      iterator(this, configMAX_PRIORITIES, DTailList<CVWaiter>::iterator());

  // Advance to the first valid priority (if any)
  result.advanceToNextValid();

  return result;
}

PriorityWaitersQueue::iterator PriorityWaitersQueue::end()
{
  return iterator(this, configMAX_PRIORITIES, DTailList<CVWaiter>::iterator());
}

// -- FIFOWaitersQueue implementation --

bool FIFOWaitersQueue::empty() const
{
  return m_list.empty();
}

FIFOWaitersQueue::iterator
FIFOWaitersQueue::enqueue(FIFOWaitersQueue::node_type* node)
{
  m_list.push_back(node);
  return --m_list.end();
}

FIFOWaitersQueue::reference FIFOWaitersQueue::peek()
{
  return m_list.front();
}

void FIFOWaitersQueue::remove(iterator it)
{
  m_list.erase(it);
}

FIFOWaitersQueue::iterator FIFOWaitersQueue::begin()
{
  return m_list.begin();
}

FIFOWaitersQueue::iterator FIFOWaitersQueue::end()
{
  return m_list.end();
}
} // namespace freertos::implementation