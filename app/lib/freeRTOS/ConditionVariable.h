#pragma once

#include <FreeRTOS.h>
#include <array>
#include <chrono>
#include <concepts>
#include <condition_variable>
#include <mutex>
#include <stop_token>
#include <utility>

#include "Mutex.h"
#include "Semaphore.h"
#include "helpers/List.h"

namespace freertos
{
namespace implementation
{
/// @brief Structure holding a binary semaphore and a signaled flag for a
/// waiting thread.
struct CVWaiter {
  BinarySemaphore semaphore;
  bool signaled = false;
};

/// @brief Concept defining the requirements for a wait queue used by
/// ConditionVariableAny.
///
/// @tparam T The container type that implements the queue.
template <typename T>
concept WaitersQueue =
    requires(T queue, typename T::node_type node, typename T::iterator it) {
      // Value
      typename T::value_type;
      requires std::same_as<typename T::value_type, CVWaiter>;

      // Node (must be intrusive queue)
      typename T::node_type;
      { node.data } -> std::same_as<typename T::value_type&>;

      // Supports iteration
      typename T::iterator;
      requires std::input_iterator<typename T::iterator>;
      requires std::same_as<typename T::iterator::value_type,
                            typename T::value_type>;

      // Reference type
      typename T::reference;
      requires std::same_as<typename T::reference, typename T::value_type&>;

      // Queue
      { queue.empty() } -> std::same_as<bool>;
      { queue.enqueue(&node) } -> std::same_as<typename T::iterator>;
      { queue.peek() } -> std::same_as<typename T::reference>;
      { queue.remove(it) } -> std::same_as<void>;
      { queue.begin() } -> std::same_as<typename T::iterator>;
      { queue.end() } -> std::same_as<typename T::iterator>;
    };

using namespace freertos::helpers;

class FIFOWaitersQueue
{
public:
  using value_type = CVWaiter;
  using node_type = DTailList<CVWaiter>::node_type;
  using iterator = DTailList<CVWaiter>::iterator;
  using reference = CVWaiter&;

  bool empty() const;
  iterator enqueue(node_type* node);
  reference peek();
  void remove(iterator it);
  iterator begin();
  iterator end();

private:
  DTailList<CVWaiter> m_list;
};

class PriorityWaitersQueue
{
public:
  using value_type = CVWaiter;
  using node_type = DTailList<CVWaiter>::node_type;
  using reference = CVWaiter&;

  class iterator
  {
  public:
    using difference_type = std::ptrdiff_t;
    using value_type = CVWaiter;
    using pointer = CVWaiter*;
    using reference = CVWaiter&;
    using iterator_concept = std::input_iterator_tag;
    using iterator_category = std::input_iterator_tag;

    iterator();

    reference operator*() const;
    pointer operator->() const;

    iterator& operator++();
    iterator operator++(int);

    bool operator==(const iterator& other) const;
    bool operator!=(const iterator& other) const;

  private:
    friend class PriorityWaitersQueue;

    iterator(PriorityWaitersQueue* owner, std::size_t priority,
             DTailList<CVWaiter>::iterator inner);

    void advanceToNextValid();

    PriorityWaitersQueue* m_owner;
    std::size_t m_priority;
    DTailList<CVWaiter>::iterator m_inner;
  };

  bool empty() const;
  iterator enqueue(node_type* node);
  reference peek();
  void remove(iterator it);
  iterator begin();
  iterator end();

private:
  std::array<DTailList<CVWaiter>, configMAX_PRIORITIES> m_lists;
};

} // namespace implementation

/// @brief A synchronization primitive that blocks one or more threads until
/// notified.
///
/// This class models the behavior of `std::condition_variable_any` using
/// FreeRTOS primitives, allowing waiting on any object that satisfies the
/// BasicLockable concept.
///
/// @tparam WaitersContainer The container type used to hold the thread waiters.
/// Defaults to a FIFO queue.
///
/// @see PriorityConditionVariable for a condition variable that wakes threads
/// based on priority. (avoids priority inversion, but has higher overhead).
template <typename WaitersContainer>
  requires implementation::WaitersQueue<WaitersContainer>
class ConditionVariableAny
{
public:
  /// @brief Default constructor.
  ConditionVariableAny() = default;

  /// @brief Default destructor.
  ~ConditionVariableAny() = default;

  ConditionVariableAny(const ConditionVariableAny&) = delete;
  ConditionVariableAny& operator=(const ConditionVariableAny&) = delete;

  /// @brief Unblocks one of the threads currently waiting for this condition.
  /// If no threads are waiting, the function does nothing.
  void notify_one() noexcept
  {
    std::lock_guard internalLock(m_waitersMutex);

    for (implementation::CVWaiter& waiter : m_waiters) {
      if (!waiter.signaled) {
        waiter.signaled = true;
        waiter.semaphore.release();
        return;
      }
    }
  }

  /// @brief Unblocks all threads currently waiting for this condition.
  /// If no threads are waiting, the function does nothing.
  void notify_all() noexcept
  {
    std::lock_guard internalLock(m_waitersMutex);

    for (implementation::CVWaiter& waiter : m_waiters) {
      if (!waiter.signaled) {
        waiter.signaled = true;
        waiter.semaphore.release();
      }
    }
  }

  /// @brief Blocks the current thread until woken up by notify_one() or
  /// notify_all().
  ///
  /// @tparam Lock The lock type.
  /// @param lock An object of type Lock that must be locked by the current
  /// thread
  ///        before calling this function.
  template <typename Lock>
    requires BasicLockable<Lock>
  void wait(Lock& lock)
  {
    typename WaitersContainer::node_type node;
    typename WaitersContainer::iterator it;
    implementation::CVWaiter& waiter = node.data;

    {
      std::lock_guard internalLock(m_waitersMutex);
      it = m_waiters.enqueue(&node);
      lock.unlock();
    }

    waiter.semaphore.acquire();

    {
      std::lock_guard internalLock(m_waitersMutex);
      m_waiters.remove(it);
    }

    lock.lock();
  }

  /// @brief Blocks the current thread until woken up and the predicate returns
  /// true.
  ///
  /// @tparam Lock The lock type.
  /// @tparam Predicate The predicate callable type.
  /// @param lock An object of type Lock that must be locked by the current
  /// thread.
  /// @param pred A callable that returns true if the wait should be terminated,
  ///        and false if it should continue waiting.
  template <typename Lock, typename Predicate>
    requires BasicLockable<Lock> && std::predicate<Predicate>
  void wait(Lock& lock, Predicate pred)
  {
    while (!pred())
      wait(lock);
  }

  /// @brief Blocks the current thread until woken up, the predicate returns
  /// true,
  ///        or the stop token is requested.
  ///
  /// @tparam Lock The lock type.
  /// @tparam Predicate The predicate callable type.
  /// @param lock An object of type Lock that must be locked by the current
  /// thread.
  /// @param stopToken A std::stop_token that can interrupt the wait.
  /// @param pred A callable that returns true if the wait should be terminated.
  /// @return true if the predicate returns true, false if the stop token was
  /// triggered.
  template <typename Lock, typename Predicate>
    requires BasicLockable<Lock> && std::predicate<Predicate>
  bool wait(Lock& lock, std::stop_token stopToken, Predicate pred)
  {
    if (stopToken.stop_requested())
      return pred();

    std::stop_callback callback(stopToken, [this] { notify_all(); });

    while (!stopToken.stop_requested()) {
      if (pred())
        return true;

      wait(lock);
    }

    return pred();
  }

  /// @brief Blocks the current thread until woken up or the specified absolute
  /// time is reached.
  ///
  /// @tparam Lock The lock type.
  /// @tparam Clock The clock type of the time point.
  /// @tparam Duration The duration type of the time point.
  /// @param lock An object of type Lock that must be locked by the current
  /// thread.
  /// @param absTime The absolute time point to wait until.
  /// @return std::cv_status::no_timeout if woken up before the timeout,
  /// std::cv_status::timeout otherwise.
  template <typename Lock, typename Clock, typename Duration>
    requires BasicLockable<Lock>
  std::cv_status
  wait_until(Lock& lock,
             const std::chrono::time_point<Clock, Duration>& absTime)
  {
    typename WaitersContainer::node_type node;
    typename WaitersContainer::iterator it;
    implementation::CVWaiter& waiter = node.data;

    {
      std::lock_guard internalLock(m_waitersMutex);
      it = m_waiters.enqueue(&node);
      lock.unlock();
    }

    bool acquired = waiter.semaphore.try_acquire_until(absTime);

    {
      std::lock_guard internalLock(m_waitersMutex);
      m_waiters.remove(it);
    }

    lock.lock();

    if (acquired)
      return std::cv_status::no_timeout;
    else {
      if (waiter.signaled)
        notify_one();

      return std::cv_status::timeout;
    }
  }

  /// @brief Blocks the current thread until woken up, the absolute time is
  /// reached,
  ///        or the predicate returns true.
  ///
  /// @tparam Lock The lock type.
  /// @tparam Clock The clock type of the time point.
  /// @tparam Duration The duration type of the time point.
  /// @tparam Predicate The predicate callable type.
  /// @param lock An object of type Lock that must be locked by the current
  /// thread.
  /// @param absTime The absolute time point to wait until.
  /// @param pred A callable that returns true if the wait should be terminated.
  /// @return true if the predicate returns true, false if the timeout expired.
  template <typename Lock, typename Clock, typename Duration,
            typename Predicate>
    requires BasicLockable<Lock> && std::predicate<Predicate>
  bool wait_until(Lock& lock,
                  const std::chrono::time_point<Clock, Duration>& absTime,
                  Predicate pred)
  {
    while (!pred()) {
      if (wait_until(lock, absTime) == std::cv_status::timeout) {
        return pred();
      }
    }
    return true;
  }

  /// @brief Blocks the current thread until woken up, the absolute time is
  /// reached,
  ///        the stop token is requested, or the predicate returns true.
  ///
  /// @tparam Lock The lock type.
  /// @tparam Clock The clock type of the time point.
  /// @tparam Duration The duration type of the time point.
  /// @tparam Predicate The predicate callable type.
  /// @param lock An object of type Lock that must be locked by the current
  /// thread.
  /// @param stopToken A std::stop_token that can interrupt the wait.
  /// @param absTime The absolute time point to wait until.
  /// @param pred A callable that returns true if the wait should be terminated.
  /// @return true if the predicate returns true, false if the timeout expired
  /// or stop was requested.
  template <typename Lock, typename Clock, typename Duration,
            typename Predicate>
    requires BasicLockable<Lock> && std::predicate<Predicate>
  bool wait_until(Lock& lock, std::stop_token stopToken,
                  const std::chrono::time_point<Clock, Duration>& absTime,
                  Predicate pred)
  {
    if (stopToken.stop_requested())
      return pred();

    std::stop_callback callback(stopToken, [this] { notify_all(); });

    while (!stopToken.stop_requested()) {
      if (pred())
        return true;

      if (wait_until(lock, absTime) == std::cv_status::timeout)
        return pred();
    }
    return pred();
  }

  /// @brief Blocks the current thread until woken up or the specified duration
  /// elapses.
  ///
  /// @tparam Lock The lock type.
  /// @tparam Rep The representation type of the duration.
  /// @tparam Period The period type of the duration.
  /// @param lock An object of type Lock that must be locked by the current
  /// thread.
  /// @param relativeTime The maximum duration to block for.
  /// @return std::cv_status::no_timeout if woken up, std::cv_status::timeout if
  /// duration elapsed.
  template <typename Lock, typename Rep, typename Period>
    requires BasicLockable<Lock>
  std::cv_status
  wait_for(Lock& lock, const std::chrono::duration<Rep, Period>& relativeTime)
  {
    return wait_until(lock, std::chrono::steady_clock::now() + relativeTime);
  }

  /// @brief Blocks the current thread until woken up, the duration elapses,
  ///        or the predicate returns true.
  ///
  /// @tparam Lock The lock type.
  /// @tparam Rep The representation type of the duration.
  /// @tparam Period The period type of the duration.
  /// @tparam Predicate The predicate callable type.
  /// @param lock An object of type Lock that must be locked by the current
  /// thread.
  /// @param relativeTime The maximum duration to block for.
  /// @param pred A callable that returns true if the wait should be terminated.
  /// @return true if the predicate returns true, false if the timeout expired.
  template <typename Lock, typename Rep, typename Period, typename Predicate>
    requires BasicLockable<Lock> && std::predicate<Predicate>
  bool wait_for(Lock& lock,
                const std::chrono::duration<Rep, Period>& relativeTime,
                Predicate pred)
  {
    return wait_until(lock, std::chrono::steady_clock::now() + relativeTime,
                      std::move(pred));
  }

  /// @brief Blocks the current thread until woken up, the duration elapses,
  ///        the stop token is requested, or the predicate returns true.
  ///
  /// @tparam Lock The lock type.
  /// @tparam Rep The representation type of the duration.
  /// @tparam Period The period type of the duration.
  /// @tparam Predicate The predicate callable type.
  /// @param lock An object of type Lock that must be locked by the current
  /// thread.
  /// @param stopToken A std::stop_token that can interrupt the wait.
  /// @param relativeTime The maximum duration to block for.
  /// @param pred A callable that returns true if the wait should be terminated.
  /// @return true if the predicate returns true, false if the timeout expired
  /// or stop was requested.
  template <typename Lock, typename Rep, typename Period, typename Predicate>
    requires BasicLockable<Lock> && std::predicate<Predicate>
  bool wait_for(Lock& lock, std::stop_token stopToken,
                const std::chrono::duration<Rep, Period>& relativeTime,
                Predicate pred)
  {
    return wait_until(lock, std::move(stopToken),
                      std::chrono::steady_clock::now() + relativeTime,
                      std::move(pred));
  }

private:
  Mutex m_waitersMutex;
  WaitersContainer m_waiters;
};

/// @brief A condition variable that wakes threads in FIFO order.
///
/// @see PriorityConditionVariable for a condition variable that wakes threads
/// based on priority. (avoids priority inversion, but has higher overhead).
using ConditionVariable =
    ConditionVariableAny<implementation::FIFOWaitersQueue>;

/// @brief Condition variable that wakes threads first in task priority order
/// then in FIFO order within the same priority.
///
/// It can avoid priority inversion scenarios where a high priority thread is
/// waiting for a condition but there are lower priority threads that are also
/// waiting.
///
/// @note This condition variable has higher overhead than the default
/// ConditionVariable.
using PriorityConditionVariable =
    ConditionVariableAny<implementation::PriorityWaitersQueue>;
} // namespace freertos