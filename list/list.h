#pragma once

#include <iterator>


template <class T>
class LinkedList;

namespace linked_list {

template <class T>
struct _ListNode {
  T val;
  _ListNode* next{nullptr};
  _ListNode* prev{nullptr};

  _ListNode()
      : val() {
  }

  _ListNode(_ListNode* prev_, _ListNode* next_, T val_)
      : val(val_), next(next_), prev(prev_) {
  }

};

template <class VAL, class PTR, class REF>
struct _list_iterator : public std::iterator<std::bidirectional_iterator_tag, VAL, std::ptrdiff_t, PTR, REF> {
 private:
  typedef _ListNode<VAL> node;

  node* p;
  friend LinkedList<VAL>;
 public:
  _list_iterator()
      : p(nullptr) {
  }

  _list_iterator(node* n)
      : p(n) {
  }

  _list_iterator(const _list_iterator& a)
      : p(a.p) {
  }

  _list_iterator& operator=(const _list_iterator& a) {
    p = a.p;
    return *this;
  }

  _list_iterator& operator++() {
    p = p->next;
    return *this;
  }

  _list_iterator& operator--() {
    p = p->prev;
    return *this;
  }

  const _list_iterator operator++(int) {
    _list_iterator t(*this);
    ++*this;
    return t;
  }

  const _list_iterator operator--(int) {
    _list_iterator t(*this);
    --*this;
    return t;
  }

  REF operator*() {
    return p->val;
  }

  PTR operator->() {
    return &(p->val);
  }

  bool operator==(_list_iterator a) const {
    return p == a.p;
  }

  bool operator!=(_list_iterator a) const {
    return p != a.p;
  }

  operator _list_iterator<VAL, const VAL*, const VAL&>() {
    return p;
  };
};
}

template <class T>
class LinkedList {
  typedef linked_list::_ListNode<T> node;

  size_t sz{0};
  node* head{nullptr};
  node* tail{nullptr};

  void copy_from(const LinkedList& old) {
    node* oldv = old.head;
    while (oldv) {
      push_back(oldv->val);
      oldv = oldv->next;
    }
  }

 public:

  typedef linked_list::_list_iterator<T, T*, T&> iterator;
  typedef linked_list::_list_iterator<T, const T*, const T&> const_iterator;

  LinkedList() {
  }

  ~LinkedList() {
    clear();
  }

  LinkedList(const LinkedList& old) {
    copy_from(old);
  }

  LinkedList& operator=(const LinkedList& old) {
    if (this == &old) {
      return *this;
    }
    ~LinkedList();
    copy_from(old);
    return *this;
  }

  void swap(LinkedList& a) {
    std::swap(sz, a.sz);
    std::swap(head, a.head);
    std::swap(tail, a.tail);
  }

  size_t size() const {
    return sz;
  }

  bool is_valid() const {
    if (head->prev || tail->next) {
      return false;
    }
    size_t real_size = 0;
    node* v = head;
    node* last = nullptr;
    while (v && real_size <= sz) {
      ++real_size;
      last = v;
      if (v->next && v->next->prev != v) {
        return false;
      }
      v = v->next;
    }
    return real_size == sz && last == tail;
  }

  const std::string dump() const {
    if (!is_valid()) {
      return "<ERROR>";
    }
    std::string res = "[";
    node* v = head;
    while (v) {
      if (v != head) {
        res += ", ";
      }
      res += std::to_string(v->val);
      v = v->next;
    }
    res += "]";
    return res;
  }

  void push_front(const T& a) {
    node* v = new node(nullptr, head, a);
    if (head) {
      head->prev = v;
    } else {
      tail = v;
    }
    head = v;
    ++sz;
  }

  void push_back(const T& a) {
    node* v = new node(tail, nullptr, a);
    if (tail) {
      tail->next = v;
    } else {
      head = v;
    }
    tail = v;
    ++sz;
  }

  void pop_front() {
    node* v = head->next;
    delete head;
    head = v;
    if (v) {
      v->prev = nullptr;
    } else {
      tail = v;
    }
    --sz;
  }

  void pop_back() {
    node* v = tail->prev;
    tail->prev = nullptr;
    delete tail;
    tail = v;
    if (v) {
      v->next = nullptr;
    } else {
      head = v;
    }
    --sz;
  }

  iterator begin() {
    return iterator(head);
  }

  const_iterator cbegin() const {
    return const_iterator(head);
  }

  const_iterator begin() const {
    return cbegin();
  }

  iterator end() {
    return iterator(nullptr);
  }

  const_iterator cend() const {
    return const_iterator(nullptr);
  }

  const_iterator end() const {
    return cend();
  }

  T& front() {
    return head->val;
  }

  const T& front() const {
    return head->val;
  }

  T& back() {
    return tail->val;
  }

  const T& back() const {
    return tail->val;
  }

  iterator insert_after(const_iterator position, const T& value) {
    if (!position.p) {
      push_front(value);
      return head;
    }
    node* v = new node(position.p, position.p->next, value);
    if (position.p->next) {
      position.p->next->prev = v;
    } else {
      tail = v;
    }
    position.p->next = v;
    ++sz;
    return iterator(v);
  }

  iterator insert_before(const_iterator position, const T& value) {
    if (!position.p) {
      push_back(value);
      return tail;
    }
    node* v = new node(position.p->prev, position.p, value);
    if (position.p->prev) {
      position.p->prev = v;
    } else {
      head = v;
    }
    position.p->prev = v;
    ++sz;
    return iterator(v);
  }

  void erase(iterator elem) {
    if (!elem.p->prev) {
      pop_front();
    } else if (!elem.p->next) {
      pop_back();
    } else {
      elem.p->prev->next = elem.p->next;
      elem.p->next->prev = elem.p->prev;
      delete elem.p;
      --sz;
    }
  }

  void clear() {
    while (head) {
      pop_front();
    }
  }

  const T& operator[](size_t index) const {
    node* v = head;
    for (; index; --index) {
      v = v->next;
    }
    return v->val;
  }

  T& operator[](size_t index) {
    return const_cast<T&>(static_cast<const LinkedList&>(*this)[index]);
  }

  const_iterator find(const T& value) const {
    node* v = head;
    for (; v; v = v->next) {
      if (v->val == value) {
        return v;
      }
    }
    return cend();
  }

  iterator find(const T& value) {
    return iterator(static_cast<const LinkedList&>(*this).find(value).p);
  }


};

