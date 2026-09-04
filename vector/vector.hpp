#ifndef VECTOR_HPP
#define VECTOR_HPP

#include <algorithm>
#include <cstring>
#include <iterator>
#include <locale>
#include <memory>
#include <stdexcept>
#include <utility>

namespace my_std {
template <class Type, class Allocator = std::allocator<Type>> class vector {
public:
  using value_type = Type;
  using const_value_type = const Type;
  using pointer = Type *;
  using const_pointer = const Type *;
  using reference = Type &;
  using const_reference = const Type &;
  using size_type = std::size_t;
  using difference_type = ptrdiff_t;
  using const_void_ptr = const void *;
  using Traits = std::allocator_traits<Allocator>;
  vector() {
    _heap._ptr = Traits::allocate(alloc, 1);
    _heap._cap = 0;
    _heap._size = 0;
  }
  explicit vector(size_type count) {
    _heap._ptr = Traits::allocate(alloc, count + 1);
    _heap._size = count;
    _heap._cap = count;
  }
  vector(size_type count, const_value_type value) {
    _heap._ptr = Traits::allocate(alloc, count + 1);
    _heap._size = count;
    _heap._cap = count;
    memset(_heap._ptr, value, count * sizeof(value_type));
  }
  vector(const vector &source) {
    size_type count = source.size();
    _heap._ptr = Traits::allocate(alloc, count + 1);
    // memcpy(_heap._ptr, source.data(), (count + 1) * sizeof(value_type));
    for (size_type i = 0; i <= count + 1; i++)
      Traits::construct(alloc, _heap._ptr + i, std::move(source._heap._ptr[i]));
    _heap._size = count;
    _heap._cap = count;
  }
  vector(vector &&source) { _heap = source._heap; }

  void reserve(size_type count) {
    if (capacity() > count)
      return;
    pointer newV = Traits::allocate(alloc, count + 1);
    size_type sz = size();
    size_type cap = capacity();
    for (size_type i = 0; i < sz; i++) {
      Traits::construct(alloc, newV + i, std::move(_heap._ptr[i]));
      Traits::destroy(alloc, _heap._ptr + i);
    }
    Traits::deallocate(alloc, _heap._ptr, cap + 1);
    _heap._ptr = newV;
    _heap._cap = count;
  }

  void push_back(value_type value) {
    if (size() + 1 >= capacity())
      reserve(capacity() + 1);
    Traits::construct(alloc, _heap._ptr + size(), value);
    _heap._size++;
  }

  void pop_back() {
    if (empty())
      return;
    _heap._ptr[size() - 1].~value_type();
    _heap._size--;
  }

  reference operator[](size_type idx) { return data()[idx]; }
  const_reference operator[](size_type idx) const { return data()[idx]; }
  vector &operator=(vector other) {
    std::swap(_heap, other._heap);
    return *this;
  }

  void clear() {
    if (empty())
      return;
    for (size_type i = 0; i < size(); ++i) {
      Traits::destroy(alloc, _heap._ptr + i);
      _heap._size--;
    }
  }

  using iterator = Type *;
  using const_iterator = const Type *;
  using reverse_iterator = std::reverse_iterator<iterator>;
  using const_reverse_iterator = std::reverse_iterator<const_iterator>;

  ~vector() noexcept { Traits::deallocate(alloc, _heap._ptr, capacity() + 1); }

  pointer data() { return _heap._ptr; }
  const_pointer data() const { return _heap._ptr; }
  constexpr size_type capacity() const { return _heap._cap; }
  constexpr size_type size() const { return _heap._size; }
  constexpr bool empty() const { return (size() == 0); }

  reference at(size_type pos) {
    if (pos >= size())
      throw std::out_of_range("current pos exceeded the size!");
    return data()[pos];
  }
  const_reference at(size_type pos) const {
    if (pos >= size())
      throw std::out_of_range("current pos exceeded the size!");
    return data()[pos];
  }

  reference back() { return data()[size() - 1]; }
  const_reference back() const { return data()[size() - 1]; }

  iterator begin() { return iterator(data()); }
  iterator end() { return iterator(data() + size()); }
  const_iterator begin() const { return const_iterator(data()); }
  const_iterator end() const { return const_iterator(data() + size()); }
  const_iterator cbegin() const { return const_iterator(data()); }
  const_iterator cend() const { return const_iterator(data() + size()); }
  reverse_iterator rbegin() { return reverse_iterator(end()); }
  reverse_iterator rend() { return reverse_iterator(begin()); }
  const_reverse_iterator rbegin() const {
    return const_reverse_iterator(end());
  }
  const_reverse_iterator rend() const {
    return const_reverse_iterator(begin());
  }
  const_reverse_iterator crbegin() const {
    return const_reverse_iterator(end());
  }
  const_reverse_iterator crend() const {
    return const_reverse_iterator(begin());
  }
  reference front() { return reference(data()); }
  const_reference front() const { return const_reference(data()); }

  iterator erase(const_iterator first, const_iterator last) {
    if (first < cbegin() || last > cend() || first > last) {
      throw std::out_of_range("invalid iterator");
    }
    if (first == last) {
      size_type idx = first - cbegin();
      for (; idx < size() - 1; ++idx) {
        Traits::destroy(alloc, data() + idx);
        Traits::construct(alloc, data() + idx, data() + idx + 1);
      }
      _heap._size--;
      return iterator(data() + idx);
    }
    size_type idxl = first - cbegin();
    size_type idxr = last - cbegin();
    for (size_type idx = idxl; idx < size() - (idxr - idxl); ++idx) {
      Traits::destroy(alloc, data() + idx);
      Traits::construct(alloc, data() + idx, data() + idx + idxr - idxl);
    }
    _heap._size - (idxr - idxl);
    return iterator(data() + idxl);
  }

  iterator erase(const_iterator pos) { return (erase(pos, pos)); }

  iterator insert(const_iterator pos, const_reference value) {
    if (pos >= cend())
      throw std::out_of_range("invalid iterator");
    size_type idx = pos - cbegin();
    pointer tmp = Traits::allocate(alloc, capacity() + 1);
    for (size_type i = 0; i < idx; ++i) {
      Traits::construct(alloc, tmp + i, data() + i);
    }
    Traits::construct(alloc, tmp + idx, value);
    for (size_type i = idx + 1; i < size(); ++i) {
      Traits::construct(alloc, tmp + i + 1, data() + i);
    }
    for (size_type i = 0; i < size(); ++i) {
      Traits::destroy(alloc, data() + i);
    }
    if (size() + 1 >= capacity())
      ++_heap._cap;
    ++_heap._size;
    _heap._ptr = tmp;
    return iterator(data() + idx);
  }
  iterator insert(const_reference pos, Type &&value) {
    return iterator(insert(pos, std::forward<value_type>(value)));
  }
  void insert(const_iterator pos, size_type cnt, const_reference value) {
    for (size_type i = 0; i < cnt; ++i)
      insert(pos, value);
  }

  void resize(size_type sz) {
    if (sz == size())
      return;
    if (sz < size()) {
      for (size_type i = sz; i < size(); ++i) {
        Traits::destroy(alloc, data() + i);
        _heap._size--;
      }
      return;
    }
    if (sz >= capacity())
      reserve(sz);
    _heap._size = sz;
  }
  void resize(size_type sz, value_type value) {
    if (sz <= size())
      resize(sz);
    if (sz >= capacity())
      reserve(sz);
    for (size_type i = size(); i <= sz; ++i) {
      _heap._ptr[i] = value;
    }
    _heap._size = sz;
  }

  void swap(vector<value_type, Allocator> &other) {
    std::swap(_heap, other._heap);
  }
  friend void swap(vector<value_type, Allocator> &lvec,
                   vector<value_type, Allocator> &rvec) {
    lvec.swap(rvec);
  }

  size_type max_size() const { return capacity(); }

private:
  struct HeapStorage {
    pointer _ptr;
    size_type _size;
    size_type _cap;
  } _heap;
  [[no_unique_address]] Allocator alloc;
};
} // namespace my_std
#endif
