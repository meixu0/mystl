// Codex 修订版：保留原有接口和 SSO 设计，并在各修改点使用 BUG/FIX 注释
// 记录原问题与修复方式。主要修复编译失败、越界读写、长度溢出、别名失效
// 及 string_view 范围错误，同时统一缩进与头文件保护宏。

#ifndef STRING_CODEX_HPP
#define STRING_CODEX_HPP

#include <algorithm>
#include <cassert>
#include <cctype>
#include <cstddef>
#include <cstdint>
#include <cstdio>
#include <cstring>
#include <functional>
#include <initializer_list>
#include <iostream>
#include <iterator>
#include <memory>
#include <optional>
#include <ostream>
#include <stdexcept>
#include <string_view>
#include <type_traits>
#include <utility>

// namespace begin
namespace my_std {

class string {
public:
  using value_type = char;
  using size_type = std::size_t;
  using pointer = char *;
  using const_pointer = const char *;
  using reference = char &;
  using const_reference = const char &;
  using const_value_type = const char;
  using difference_type = std::ptrdiff_t;
  using Traits = std::allocator_traits<std::allocator<char>>;

  static constexpr size_type npos = static_cast<size_type>(-1);
  static constexpr size_type SSO_CAPACITY = 22;
  // BUG/FIX: 原实现把 64 位常量硬编码为堆标志，在 32 位平台会截断。
  // 使用 size_type 的最高位，并验证 SSO 容量不会占用其标志位。
  static constexpr size_type HEAP_FLAG =
      size_type{1} << (sizeof(size_type) * 8 - 1);
  static_assert(SSO_CAPACITY < 0x80, "SSO size must not use the flag bit");

  string(const char *str = nullptr) {
    _init_sso();
    if (!str)
      return;
    // optimize
    size_type len = strlen(str);
    if (len <= SSO_CAPACITY) {
      _sso._size_and_flag = len;
      memcpy(_sso._buf, str, len + 1);
    } else {
      _heap._ptr = Traits::allocate(alloc, len + 1);
      _heap._size = len;
      _heap._cap = len | HEAP_FLAG;
      memcpy(_heap._ptr, str, len + 1);
    }
  }

  string(size_type len, const_value_type c = '\0') {
    _init_sso();
    if (len <= SSO_CAPACITY) {
      _sso._size_and_flag = len;
      memset(_sso._buf, c, len);
      _sso._buf[len] = '\0';
    } else {
      _heap._ptr = Traits::allocate(alloc, len + 1);
      _heap._size = len;
      _heap._cap = len | HEAP_FLAG;
      memset(_heap._ptr, c, len);
      _heap._ptr[len] = '\0';
    }
  }

  template <typename InputIt, typename = typename std::enable_if<
                                  !std::is_integral<InputIt>::value>::type>
  string(InputIt it1, InputIt it2) {
    _init_sso();
    if constexpr (std::is_base_of_v<std::forward_iterator_tag,
                                    typename std::iterator_traits<
                                        InputIt>::iterator_category>) {
      size_t len = std::distance(it1, it2);
      reserve(len);
      std::copy(it1, it2, begin());
      _set_sz(len);
      data()[len] = '\0';
    } else {
      for (auto it = it1; it != it2; ++it) {
        push_back(*it);
      }
    }
  }

  string(const std::initializer_list<char> &iniL) {
    _init_sso();
    size_type len = iniL.size();
    if (len <= SSO_CAPACITY) {
      std::copy(iniL.begin(), iniL.end(), _sso._buf);
      _sso._size_and_flag = len;
      _sso._buf[len] = '\0';
    } else {
      _heap._ptr = Traits::allocate(alloc, len + 1);
      std::copy(iniL.begin(), iniL.end(), _heap._ptr);
      _heap._size = len;
      _heap._cap = len | HEAP_FLAG;
      _heap._ptr[len] = '\0';
    }
  }

  ~string() noexcept {
    if (!is_sso()) {
      Traits::deallocate(alloc, _heap._ptr, capacity() + 1);
    }
  }

  string(const string &o) {
    _init_sso();
    size_type len = o.size();

    if (len <= SSO_CAPACITY) {
      memcpy(_sso._buf, o.data(), len + 1);
      _sso._size_and_flag = len;
      _sso._buf[len] = '\0';
    } else {
      _heap._ptr = Traits::allocate(alloc, len + 1);
      memcpy(_heap._ptr, o.data(), len + 1);
      _heap._size = len;
      _heap._cap = len | HEAP_FLAG;
    }
  }

  // change
  string(string &&o) noexcept {
    if (o.is_sso()) {
      _sso = o._sso;
    } else {
      _heap = o._heap;
    }
    o._init_sso();
  }

  void reserve(size_type sz) {
    if (sz <= capacity()) {
      return;
    }
    // BUG/FIX: sz == max_size() 时 sz + 1 会溢出；先做容量检查。
    if (sz > max_size()) {
      throw std::length_error("string capacity exceeds max_size");
    }
    pointer new_str = Traits::allocate(alloc, sz + 1);
    size_type res = size();
    pointer _pos = (is_sso()) ? _sso._buf : _heap._ptr;
    std::memcpy(new_str, _pos, res);
    new_str[res] = '\0';

    if (is_sso()) {
      _heap._size = res;
    } else {
      Traits::deallocate(alloc, _heap._ptr, capacity() + 1);
    }
    _heap._cap = sz | HEAP_FLAG;
    _heap._ptr = new_str;
    return;
  }

  void resize(size_type n, const_value_type c = '\0') {
    size_type sz = size();
    if (n > sz) {
      reserve(n);
      memset(data() + sz, c, n - sz);
    }
    data()[n] = '\0';
    _set_sz(n);
  }

  void assign(size_type n, const_value_type c) {
    reserve(n);
    std::memset(data(), c, n);
    _set_sz(n);
    data()[n] = '\0';
  }

  reference back() {
    assert(size() && "back() called on empty string");
    return data()[size() - 1];
  }

  const_reference back() const {
    assert(size() && "back() called on empty string");
    return data()[size() - 1];
  }

  reference front() {
    assert(size() && "front() called on empty string");
    return data()[0];
  }

  const_reference front() const {
    assert(size() && "front() called on empty string");
    return data()[0];
  }

  void push_back(const char c) {
    if (is_sso()) {
      size_type sz = _sso._size_and_flag;
      if (sz == SSO_CAPACITY) {
        reserve(sz * 2);
        _heap._ptr[sz] = c;
        _heap._ptr[sz + 1] = '\0';
        _heap._size = sz + 1;
      } else {
        _sso._buf[sz] = c;
        _sso._buf[sz + 1] = '\0';
        _sso._size_and_flag = sz + 1;
      }
    } else {
      size_type sz = _heap._size;
      if (sz == capacity()) {
        // BUG/FIX: 倍增可能溢出，且应确保至少能容纳一个新字符。
        if (sz == max_size()) {
          throw std::length_error("push_back exceeds max_size");
        }
        const size_type next = sz > max_size() / 2 ? max_size() : sz * 2;
        reserve(next);
      }
      _heap._ptr[sz] = c;
      _heap._ptr[sz + 1] = '\0';
      _heap._size = sz + 1;
    }

    // BUG/FIX: 原文件在此缺少右花括号，使后续所有成员函数都被解析到
    // push_back() 内部，头文件因“不允许嵌套函数定义”而无法编译。
  }

  pointer data() { return is_sso() ? _sso._buf : _heap._ptr; }

  const_pointer data() const { return is_sso() ? _sso._buf : _heap._ptr; }

  constexpr size_type size() const {
    if (is_sso())
      return _sso._size_and_flag;
    else
      return _heap._size;
  }

  constexpr size_type length() const { return size(); }

  constexpr size_type capacity() const {
    if (is_sso())
      return SSO_CAPACITY;
    else
      return _heap._cap & (~HEAP_FLAG);
  }

  constexpr size_type max_size() const {
    // BUG/FIX: 堆标志占用 size_type 最高位，容量不能使用该位；同时为结尾
    // '\0' 保留一个分配单元。
    return std::min(Traits::max_size(alloc) - 1, HEAP_FLAG - 1);
  }

  auto get_allocator() const { return alloc; }

  void shrink_to_fit() {
    if (!is_sso() && size() < capacity()) {
      string temp(*this);
      swap(temp);
    }
  }

  void swap(string & other) noexcept {
    if (is_sso() && other.is_sso()) {
      auto tmp = _sso;
      _sso = other._sso;
      other._sso = tmp;
    } else if (!is_sso() && !other.is_sso()) {
      std::swap(_heap, other._heap);
    } else if (is_sso() && !other.is_sso()) {
      auto temp_sso = _sso;
      auto temp_heap = other._heap;
      other._sso = temp_sso;
      _heap = temp_heap;
    } else if (!is_sso() && other.is_sso()) {
      auto temp_sso = other._sso;
      auto temp_heap = _heap;
      _sso = temp_sso;
      other._heap = temp_heap;
    }
  }

  reference operator[](size_type idx) { return data()[idx]; }

  const_reference operator[](size_type idx) const { return data()[idx]; }

  string &operator=(string other) {
    swap(other);
    return *this;
  }

  string &operator=(const char *s) {
    if (!s) {
      return *this;
    }
    // BUG/FIX: s 可能指向当前字符串内部；原实现扩容或 memcpy 重叠时会
    // 令源指针失效或触发未定义行为，先用临时对象保存源数据。
    if (s >= data() && s <= data() + size()) {
      string temp(s);
      swap(temp);
      return *this;
    }
    size_type len = std::strlen(s);
    if (capacity() < len) {
      string temp(s);
      swap(temp);
    } else {
      std::memmove(data(), s, len);
      _set_sz(len);
      data()[len] = '\0';
    }
    return *this;
  }

  friend string operator+(const string &s1, const string &s2) {
    string temp;
    temp.reserve(s1.size() + s2.size());
    temp += s1;
    temp += s2;
    return temp;
  }

  friend string operator+(string &&s1, const string &s2) {
    s1 += s2;
    return std::move(s1);
  }

  friend string operator+(const string &s1, string &&s2) {
    s2.insert(0, s1);
    return s2;
  }

  friend string operator+(string &&s1, string &&s2) {
    s1 += s2;
    return std::move(s1);
  }

  string &operator=(char c) {
    clear();
    push_back(c);
    return *this;
  }

  friend void swap(string & lstring, string & rstring) {
    lstring.swap(rstring);
  }

  bool operator==(const string &other) const {
    if (this == &other) {
      return 1;
    }
    if (size() != other.size()) {
      return 0;
    }
    return std::memcmp(data(), other.data(), size()) == 0;
  }

  bool operator!=(const string &other) const { return !(*this == other); }

  bool operator<(const string &o) const {
    size_type min_sz = std::min(size(), o.size());
    int val = std::memcmp(data(), o.data(), min_sz);
    if (val) {
      return val < 0;
    }
    return size() < o.size();
  }

  bool operator<=(const string &o) const {
    size_type min_sz = std::min(size(), o.size());
    int val = std::memcmp(data(), o.data(), min_sz);
    if (val) {
      return val <= 0;
    }
    return size() <= o.size();
  }

  bool operator>(const string &o) const { return !(*this <= o); }

  bool operator>=(const string &o) const { return !(*this < o); }

  friend std::ostream &operator<<(std::ostream &os, const string &s) {
    os.write(s.data(), s.size());
    return os;
  }

  friend std::istream &operator>>(std::istream &is, string &s) {
    s.clear();
    char ch;
    // skip space first, add cast
    while (is.get(ch) && std::isspace(static_cast<unsigned char>(ch))) {
      continue;
    }
    if (!is)
      return is;
    char buf[256];
    size_type cnt = 0;
    buf[cnt++] = ch;
    while (is.get(ch) && !std::isspace(static_cast<unsigned char>(ch))) {
      buf[cnt++] = ch;
      if (cnt == sizeof(buf)) {
        s.append(buf, buf + cnt);
        cnt = 0;
      }
    }
    if (is) {
      is.unget();
    }

    if (cnt) {
      s.append(buf, buf + cnt);
    }
    return is;
  }

  friend std::istream &getline(std::istream & is, string & s,
                               char delim = '\n') {
    s.clear();
    char buf[256];
    size_type cnt = 0;
    char ch;
    while (is.get(ch)) {
      if (ch == delim)
        break;
      buf[cnt++] = ch;
      if (cnt == sizeof(buf)) {
        s.append(buf, buf + cnt);
        cnt = 0;
      }
    }
    if (cnt) {
      s.append(buf, buf + cnt);
    }
    return is;
  }

  constexpr bool empty() const { return (size() == 0); }

  void clear() {
    data()[0] = '\0';
    _set_sz(0);
  }

  void pop_back() {
    assert(size() && "no elements in string when pop_back()!");
    back() = '\0';
    _set_sz(size() - 1);
  }

  const char *c_str() const { return data(); }

  reference at(size_type idx) {
    if (idx >= size()) {
      throw std::out_of_range("the curent idx exceeded the size!");
    }
    return data()[idx];
  }
  // const reload
  const_reference at(size_type idx) const {
    if (idx >= size()) {
      throw std::out_of_range("the curent idx exceeded the size!");
    }
    return data()[idx];
  }

  using iterator = char *;
  using const_iterator = const char *;
  using reverse_iterator = std::reverse_iterator<iterator>;
  using const_reverse_iterator = std::reverse_iterator<const_iterator>;

  iterator begin() { return iterator(data()); }
  iterator end() { return iterator(data() + size()); }
  const_iterator begin() const { return const_iterator(data()); }
  const_iterator end() const { return const_iterator(data() + size()); }
  const_iterator cbegin() const { return begin(); }
  const_iterator cend() const { return end(); }

  reverse_iterator rbegin() { return reverse_iterator(end()); }
  reverse_iterator rend() { return reverse_iterator(begin()); }
  const_reverse_iterator rbegin() const {
    return const_reverse_iterator(end());
  }
  const_reverse_iterator rend() const {
    return const_reverse_iterator(begin());
  }
  const_reverse_iterator crbegin() const { return rbegin(); }
  const_reverse_iterator crend() const { return rend(); }

  string &operator+=(const string &other) {
    append(other);
    return *this;
  }

  std::optional<size_type> find(const string &substr, size_type pos = 0)
      const {
    if (pos > size()) {
      return std::nullopt;
    }
    if (substr.empty()) {
      return pos;
    }
    if (substr.size() > size() - pos) {
      return std::nullopt;
    }

    size_type search_space = size() - pos;
    if (substr.size() * search_space < 10000) {
      const char fst = substr[0];
      const_pointer ptr = data() + pos, str = substr.data();
      auto rem = (search_space - substr.size() + 1);
      while (rem > 0) {
        auto it = static_cast<const_pointer>(std::memchr(ptr, fst, rem));
        if (!it) {
          break;
        }

        if (std::memcmp(it, str, substr.size()) == 0) {
          return static_cast<size_type>(it - data());
        }

        rem -= ((++it) - ptr);
        ptr = it;
      }
      return std::nullopt;
    } else {
      std::boyer_moore_horspool_searcher searcher(substr.cbegin(),
                                                  substr.cend());
      auto it = std::search(begin() + pos, end(), searcher);
      if (it == end()) {
        return std::nullopt;
      } else {
        return static_cast<size_type>(it - begin());
      }
    }
  }

  void insert(size_type pos, const string &substr, size_type subpos = 0,
              size_type sublen = npos) {
    if (pos > size() || subpos > substr.size()) {
      throw std::out_of_range("insert index out of range");
    }
    size_type actual_len = std::min(sublen, substr.size() - subpos);
    if (substr.empty() || actual_len == 0)
      return;
    if (capacity() < size() + actual_len || this == &substr) {
      string tmp;
      tmp.reserve(size() + actual_len);
      memcpy(tmp.data(), data(), pos);
      memcpy(tmp.data() + pos, substr.data() + subpos, actual_len);
      memcpy(tmp.data() + pos + actual_len, data() + pos, size() - pos);
      tmp._set_sz(size() + actual_len);
      tmp.data()[tmp.size()] = '\0';
      *this = std::move(tmp);
    } else {
      std::memmove(data() + actual_len + pos, data() + pos, size() - pos);
      std::memcpy(data() + pos, substr.data() + subpos, actual_len);
      _set_sz(size() + actual_len);
      data()[size()] = '\0';
    }
  }

  string substr(const_iterator i1, const_iterator i2) const {
    string temp;
    // BUG/FIX: 原实现未校验迭代器范围，且对无符号 len 使用 len <= 0；
    // 非法范围可能下溢为超大长度。
    if (i1 < cbegin() || i2 > cend() || i1 > i2) {
      throw std::out_of_range("invalid iterator range for substr");
    }
    size_type len = static_cast<size_type>(i2 - i1);
    if (len == 0) {
      return temp;
    }
    temp.reserve(len);
    std::memcpy(temp.data(), i1, len);
    temp._set_sz(len);
    temp.data()[len] = '\0';
    return temp;
  }

  string &replace(size_type pos, size_type n, const string &sub) {
    if (pos > size()) {
      throw std::out_of_range("pos exceeded the size!");
    }

    n = std::min(n, size() - pos);

    size_type next_sz = size() + sub.size() - n;
    // BUG/FIX: next_sz == capacity() 时已有空间（另有结尾 '\0' 的槽位），
    // 原来的 >= 会进行不必要的分配。
    if (next_sz > capacity() || this == &sub) {
      string temp;
      temp.reserve(next_sz);
      std::memcpy(temp.data(), data(), pos);
      std::memcpy(temp.data() + pos, sub.data(), sub.size());
      std::memcpy(temp.data() + pos + sub.size(), data() + pos + n,
                  size() - pos - n);
      temp._set_sz(next_sz);
      temp.data()[next_sz] = '\0';
      (*this) = std::move(temp);
    } else {
      std::memmove(data() + pos + sub.size(), data() + pos + n,
                   size() - pos - n);
      std::memcpy(data() + pos, sub.data(), sub.size());
      _set_sz(size() + sub.size() - n);
      data()[size()] = '\0';
    }

    return *this;
  }

  // BUG/FIX: 默认 len == npos 时，原实现直接构造 string(len)，会发生超大
  // 分配；显式长度超过尾部时还会越界读取。按 std::string 语义裁剪长度。
  string substr(size_type pos = 0, size_type len = npos) const {
    if (pos > size()) {
      throw std::out_of_range("pos exceeded the size!");
    }
    len = std::min(len, size() - pos);
    string temp(len);
    std::memcpy(temp.data(), data() + pos, len);
    return temp;
  }

  // BUG/FIX: 原实现既未包含 <string_view>，也未检查 pos/裁剪 npos，产生
  // 指向对象外部的 view；同时补上 const 限定。
  std::string_view subsv(size_type pos = 0, size_type len = npos) const {
    if (pos > size()) {
      throw std::out_of_range("pos exceeded the size");
    }
    return std::string_view(data() + pos, std::min(len, size() - pos));
  }

  string &append(const string &str) {
    append(str.data(), str.size());
    return *this;
  }

  string &append(const string &str, size_type subpos, size_type sublen) {
    if (subpos > str.size()) {
      throw std::out_of_range("subpos exceeded size!");
    }

    size_type len = std::min(sublen, str.size() - subpos);
    append(str.data() + subpos, len);
    return *this;
  }

  string &append(const_pointer s) {
    size_type len = strlen(s);
    append(s, len);
    return *this;
  }

  string &append(const_pointer s, size_type len) {
    if (!s && len != 0) {
      throw std::invalid_argument("null source passed to append");
    }
    if (len > max_size() - size()) {
      throw std::length_error("append exceeds max_size");
    }

    // BUG/FIX: 原来的 data()+size() >= s 会比较无关对象的指针（未定义
    // 行为），也无法可靠判断重叠；用地址整数只判断当前缓冲区内别名，
    // 并在 reserve 前记录偏移以避免源指针失效。
    const auto begin_addr = reinterpret_cast<std::uintptr_t>(data());
    const auto end_addr = begin_addr + size();
    const auto source_addr = reinterpret_cast<std::uintptr_t>(s);
    const bool aliases = source_addr >= begin_addr && source_addr <= end_addr;
    const size_type source_offset =
        aliases ? static_cast<size_type>(source_addr - begin_addr) : 0;
    const size_type old_size = size();
    reserve(old_size + len);
    if (aliases) {
      std::memmove(data() + old_size, data() + source_offset, len);
    } else if (len != 0) {
      std::memcpy(data() + old_size, s, len);
    }
    _set_sz(old_size + len);
    data()[size()] = '\0';

    return *this;
  }

  string &append(size_type n, value_type ch) {
    // if (size() + n >= capacity())
    reserve(size() + n);
    memset(data() + size(), ch, n);
    // add:
    _set_sz(size() + n);
    data()[size()] = '\0';
    return *this;
  }

  template <typename InputIt, typename = typename std::enable_if<
                                  !std::is_integral<InputIt>::value>::type>
  string &append(InputIt s, InputIt t) {
    if constexpr (std::is_base_of_v<std::forward_iterator_tag,
                                    typename std::iterator_traits<
                                        InputIt>::iterator_category>) {
      size_type len = std::distance(s, t);
      if constexpr (std::is_same_v<const_pointer, InputIt> ||
                    std::is_same_v<pointer, InputIt> ||
                    std::is_same_v<iterator, InputIt> ||
                    std::is_same_v<const_iterator, InputIt>) {
        // BUG/FIX: 空范围时 &*s 会解引用 end()；直接传入指针即可。
        return append(s, len);
      } else {
        reserve(size() + len);
        std::copy(s, t, data() + size());
        _set_sz(size() + len);
        data()[size()] = '\0';
      }
    } else {
      for (auto it = s; it != t; ++it)
        push_back(*it);
    }

    return *this;
  }

  iterator erase(const_iterator s, const_iterator t) {
    if (s < cbegin() || t > cend() || s > t) {
      throw std::out_of_range("Invalid iterator range for erase!");
    }
    if (s == t) {
      return iterator(data() + (s - cbegin()));
    }

    size_type pos_s = s - cbegin();
    size_type pos_t = t - cbegin();

    size_type tail_rem = size() - pos_t;
    size_type erase_len = pos_t - pos_s;

    std::memmove(data() + pos_s, data() + pos_t, tail_rem);
    _set_sz(size() - erase_len);
    data()[size()] = '\0';

    return iterator(data() + pos_s);
  }

  iterator erase(const_iterator it) { return erase(it, it + 1); }

  int compare(const_pointer str) const {
    size_type len = std::strlen(str), sz = size();
    size_type min_sz = std::min(sz, len);
    int val = std::memcmp(data(), str, min_sz);
    if (val != 0)
      return val;

    if (sz < len)
      return -1;
    if (sz > len)
      return 1;
    return 0;
  }

  int compare(const string &o) const {
    size_type sz1 = size(), sz2 = o.size();
    size_type min_sz = std::min(sz1, sz2);
    int val = std::memcmp(data(), o.data(), min_sz);
    if (val != 0)
      return val;

    if (sz1 < sz2)
      return -1;
    if (sz1 > sz2)
      return 1;
    return 0;
  }

  int compare(size_type pos1, size_type len1, const string &o,
              size_type pos2 = 0, size_type len2 = npos) const {
    if (pos1 > size() || pos2 > o.size()) {
      throw std::out_of_range("compare index out of range");
    }
    size_type sz1 = std::min(size() - pos1, len1);
    size_type sz2 = std::min(o.size() - pos2, len2);

    size_type min_sz = std::min(sz1, sz2);
    int val = std::memcmp(data() + pos1, o.data() + pos2, min_sz);
    if (val != 0)
      return val;

    if (sz1 < sz2)
      return -1;
    if (sz1 > sz2)
      return 1;
    return 0;
  }

  bool contains(const_pointer str) const { return find(str).has_value(); }

  bool contains(const string &s) const { return find(s).has_value(); }

  bool contains(const_value_type c) const {
    return std::memchr(data(), c, size());
  }

  bool starts_with(const_pointer str) const {
    size_type len = strlen(str);
    if (size() < len)
      return 0;
    return std::memcmp(data(), str, len) == 0;
  }

  bool starts_with(const string &o) const {
    if (size() < o.size())
      return 0;
    return std::memcmp(data(), o.data(), o.size()) == 0;
  }

  bool starts_with(const_value_type c) const {
    return (size() && front() == c);
  }

  bool ends_with(const_pointer str) const {
    size_type len = strlen(str);
    if (size() < len)
      return 0;
    return std::memcmp(data() + size() - len, str, len) == 0;
  }

  bool ends_with(const string &o) const {
    if (size() < o.size())
      return 0;
    return std::memcmp(data() + size() - o.size(), o.data(), o.size()) == 0;
  }

  bool ends_with(const_value_type c) const { return (size() && back() == c); }

private:
  // BUG/FIX: 原匿名联合体内直接声明匿名结构体是编译器扩展，在
  // -Wpedantic 下会报警；改为具名存储类型以符合标准 C++。
  struct HeapStorage {
    pointer _ptr;
    size_type _size;
    size_type _cap;
  };

  struct SsoStorage {
    char _buf[23];
    uint8_t _size_and_flag;
  };

  union {
    HeapStorage _heap;
    SsoStorage _sso;
  };

  [[no_unique_address]] std::allocator<char> alloc;

  constexpr bool is_sso() const { return !(_sso._size_and_flag & 0x80); }
  void _set_sz(size_type n) {
    if (is_sso()) {
      _sso._size_and_flag = static_cast<uint8_t>(n);
    } else {
      _heap._size = n;
    }
  }
  constexpr void _init_sso() {
    _sso._size_and_flag = 0;
    _sso._buf[0] = '\0';
  }
};
} // namespace my_std

#endif // STRING_CODEX_HPP
