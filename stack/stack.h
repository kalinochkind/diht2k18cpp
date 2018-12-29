#pragma once

#include <iostream>

#undef constexpr
#undef size_t
#undef CANARY1
#undef CANARY2
#undef HASH_MULTIPLIER
#undef template
#undef class
#undef T
#undef bool
#undef is_ok
#undef const
#undef Stack
#undef stack
#undef return
#undef canary1
#undef canary2
#undef sz
#undef allocated
#undef buf
#undef void
#undef dump
#undef std
#undef cout
#undef typeid
#undef name
#undef if
#undef public
#undef this
#undef private
#undef size
#undef push
#undef pop
#undef extend
#undef true
#undef false
#undef try
#undef catch
#undef exception
#undef val
#undef out
#undef move_if_noexcept
#undef friend
#undef new_allocated
#undef new_buf
#undef i
#undef bad_alloc
#undef new
#undef delete
#undef checksum
#undef check
#undef operator
#undef is_bad_pointer

constexpr size_t CANARY1 = 0xDEADBEEFABACABADul;
constexpr size_t HASH_MULTIPLIER = 257;


template <class T>
class Stack;

template <class T>
bool is_bad_pointer(T* p) {
  return (size_t) p < 0x10000 || ~(size_t) p < 0x10000;
}

template <class T>
bool is_ok(const Stack<T>* stack) {
  return (!is_bad_pointer(stack) && stack->canary1 == CANARY1 && stack->canary2 == stack &&
          stack->check == checksum(stack) && stack->sz <= stack->allocated &&
          (!is_bad_pointer(stack->buf) || !stack->allocated));
}

template <class T>
size_t checksum(const Stack<T>* stack) {
  if (is_bad_pointer(stack)) {
    return 0;
  }
  return (size_t) stack->buf + stack->allocated * HASH_MULTIPLIER + stack->sz * HASH_MULTIPLIER * HASH_MULTIPLIER;
}


template <class T>
void dump(const Stack<T>* stack) {
  std::cout << "Stack<" << typeid(T).name() << "> at " << stack << ":\n";
  if (is_bad_pointer(stack)) {
    std::cout << "Does not exist\n";
    return;
  }
  std::cout << "canary1: " << (void*) stack->canary1 << "\n";
  std::cout << "size: " << stack->sz << "\n";
  std::cout << "allocated: " << stack->allocated << "\n";
  std::cout << "buffer: " << stack->buf << "\n";
  std::cout << "[\n";
  for (size_t i = 0; i < stack->sz; ++i) {
    std::cout << "  " << i << " = " << stack->buf[i] << '\n';
  }
  std::cout << "]\n";
  std::cout << "canary2: " << (void*) stack->canary2 << "\n";
  std::cout << "checksum: " << stack->check << "\n";
  std::cout << "actual checksum: " << checksum(stack) << "\n";
  std::cout << "OK: " << is_ok(stack) << '\n';
}

#ifndef RELEASE_BUILD
#define ASSERT_OK if(!is_ok(this)) { dump(this); return false; }
#else
#define ASSERT_OK
#endif

template <class T>
class Stack {
 public:
  Stack() {
    canary2 = (void*) this;
  }

  Stack(const Stack<T>&) = delete;

  Stack& operator=(const Stack<T>&) = delete;

  ~Stack() {
    if (is_ok(this) && buf) {
      delete[] buf;
    }
  }

  size_t size() const {
    ASSERT_OK;
    return sz;
  }

  bool push(const T& val) {
    ASSERT_OK;
    if (sz >= allocated) {
      if (!extend()) {
        std::cout << "push() failed\n";
        return false;
      }
      check = checksum(this);
      ASSERT_OK;
    }
    try {
      buf[sz] = val;
    } catch (...) {
      ASSERT_OK;
      throw;
    }
    ++sz;
    check = checksum(this);
    ASSERT_OK;
    return true;
  }

  bool pop(T* out) {
    ASSERT_OK;
    if (!out) {
      std::cout << "pop() called with invalid pointer\n";
      return false;
    }
    if (!sz) {
      std::cout << "pop() called for empty stack\n";
      return false;
    }
    try {
      *out = std::move_if_noexcept(buf[sz - 1]);
    } catch (const std::exception&) {
      ASSERT_OK;
      return false;
    }
    --sz;
    check = checksum(this);
    ASSERT_OK;
    return true;
  }

 private:
  friend void dump<T>(const Stack<T>*);

  friend bool is_ok<T>(const Stack<T>*);

  friend size_t checksum<T>(const Stack<T>*);

  bool extend() {
    size_t new_allocated = 4;
    if (allocated) {
      new_allocated = allocated * 2;
    }
    T* new_buf = nullptr;
    try {
      new_buf = new T[new_allocated];
    } catch (const std::bad_alloc&) {
      std::cout << "Buffer allocation failed\n";
      return false;
    }
    for (size_t i = 0; i < sz; ++i) {
      try {
        new_buf[i] = std::move_if_noexcept(buf[i]);
      } catch (const std::exception&) {
        delete[] new_buf;
        return false;
      }
    }
    delete[] buf;
    buf = new_buf;
    allocated = new_allocated;
    return true;
  }


  size_t canary1 = CANARY1;
  size_t sz = 0;
  size_t allocated = 0;
  T* buf = nullptr;
  size_t check = 0;
  void* canary2 = nullptr;

};