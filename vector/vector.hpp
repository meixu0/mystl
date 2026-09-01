#ifndef VECTOR_HPP
#define VECTOR_HPP

#include <algorithm>
#include <cstring>
#include <iostream>
#include <iterator>
#include <memory>
#include <system_error>
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

  ~vector() noexcept { Traits::deallocate(alloc, _heap._ptr, capacity() + 1); }
  pointer data() { return _heap._ptr; }
  const_pointer data() const { return _heap._ptr; }
  constexpr size_type capacity() const { return _heap._cap; }
  constexpr size_type size() const { return _heap._size; }
  constexpr bool empty() const { return (size() == 0); }

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
