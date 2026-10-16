#include "vector.hpp"

#include <cstdlib>
#include <iostream>
#include <stdexcept>
#include <string>
#include <utility>

namespace {

class Record {
public:
  Record() : id_(0), name_("default") { ++alive_; }
  Record(int id, std::string name) : id_(id), name_(std::move(name)) {
    ++alive_;
  }
  Record(const Record &other) : id_(other.id_), name_(other.name_) {
    ++alive_;
  }
  Record(Record &&other) noexcept
      : id_(other.id_), name_(std::move(other.name_)) {
    other.id_ = -1;
    ++alive_;
  }
  Record &operator=(const Record &) = default;
  Record &operator=(Record &&) noexcept = default;
  ~Record() { --alive_; }

  int id() const { return id_; }
  const std::string &name() const { return name_; }
  static int alive() { return alive_; }

  friend bool operator==(const Record &lhs, const Record &rhs) {
    return lhs.id_ == rhs.id_ && lhs.name_ == rhs.name_;
  }

private:
  int id_;
  std::string name_;
  static int alive_;
};

int Record::alive_ = 0;

int failures = 0;

#define CHECK(condition)                                                       \
  do {                                                                         \
    if (!(condition)) {                                                        \
      std::cerr << __FILE__ << ':' << __LINE__ << ": CHECK failed: "          \
                << #condition << '\n';                                         \
      ++failures;                                                              \
    }                                                                          \
  } while (false)

Record make_record(int id) { return Record(id, "record-" + std::to_string(id)); }

void test_default_constructor_and_capacity() {
  my_std::vector<Record> values;
  CHECK(values.empty());
  CHECK(values.size() == 0);
  CHECK(values.capacity() == 0);
  CHECK(values.max_size() >= values.capacity());
  CHECK(values.begin() == values.end());
  CHECK(values.cbegin() == values.cend());

  values.reserve(5);
  CHECK(values.empty());
  CHECK(values.capacity() >= 5);
  const auto old_capacity = values.capacity();
  values.reserve(2);
  CHECK(values.capacity() == old_capacity);
}

void test_count_and_fill_constructors() {
  my_std::vector<Record> defaults(3);
  CHECK(defaults.size() == 3);
  CHECK(defaults.capacity() == 3);
  for (const auto &value : defaults)
    CHECK(value == Record());

  const Record seed = make_record(8);
  my_std::vector<Record> filled(3, seed);
  CHECK(filled.size() == 3);
  CHECK(filled.capacity() == 3);
  for (const auto &value : filled)
    CHECK(value == seed);
}

void test_push_pop_and_element_access() {
  my_std::vector<Record> values;
  values.push_back(make_record(1));
  values.push_back(make_record(2));
  values.push_back(make_record(3));

  CHECK(values.size() == 3);
  CHECK(values[0] == make_record(1));
  CHECK(values.at(1) == make_record(2));
  CHECK(values.front() == make_record(1));
  CHECK(values.back() == make_record(3));
  CHECK(values.data() == values.begin());

  values[1] = make_record(20);
  CHECK(values.at(1) == make_record(20));

  bool threw = false;
  try {
    (void)values.at(values.size());
  } catch (const std::out_of_range &) {
    threw = true;
  }
  CHECK(threw);

  values.pop_back();
  CHECK(values.size() == 2);
  CHECK(values.back() == make_record(20));
  values.pop_back();
  values.pop_back();
  bool empty_pop_threw = false;
  try {
    values.pop_back();
  } catch (const std::runtime_error &) {
    empty_pop_threw = true;
  }
  CHECK(empty_pop_threw);
  CHECK(values.empty());
}

void test_const_access_and_iterators() {
  my_std::vector<Record> values;
  values.push_back(make_record(1));
  values.push_back(make_record(2));
  values.push_back(make_record(3));
  const my_std::vector<Record> &view = values;

  CHECK(view[0] == make_record(1));
  CHECK(view.at(1) == make_record(2));
  CHECK(view.front() == make_record(1));
  CHECK(view.back() == make_record(3));
  CHECK(view.data() == view.cbegin());

  int expected = 1;
  for (auto it = view.begin(); it != view.end(); ++it)
    CHECK(it->id() == expected++);

  expected = 3;
  for (auto it = view.crbegin(); it != view.crend(); ++it)
    CHECK(it->id() == expected--);

  expected = 3;
  for (auto it = values.rbegin(); it != values.rend(); ++it)
    CHECK(it->id() == expected--);
}

void test_copy_move_assignment_and_swap() {
  my_std::vector<Record> source;
  source.push_back(make_record(1));
  source.push_back(make_record(2));

  my_std::vector<Record> copied(source);
  CHECK(copied.size() == 2);
  CHECK(copied[0] == make_record(1));
  copied[0] = make_record(10);
  CHECK(source[0] == make_record(1));

  my_std::vector<Record> assigned;
  assigned = source;
  CHECK(assigned.size() == source.size());
  CHECK(assigned[1] == source[1]);

  my_std::vector<Record> moved(std::move(copied));
  CHECK(moved.size() == 2);
  CHECK(moved[0] == make_record(10));

  assigned.swap(moved);
  CHECK(assigned[0] == make_record(10));
  CHECK(moved[0] == make_record(1));
  swap(assigned, moved);
  CHECK(assigned[0] == make_record(1));
}

void test_insert_erase_clear_and_resize() {
  my_std::vector<Record> values;
  values.reserve(8);
  values.push_back(make_record(1));
  values.push_back(make_record(3));

  auto inserted = values.insert(values.begin() + 1, make_record(2));
  CHECK(inserted == values.begin() + 1);
  CHECK(values.size() == 3);
  CHECK(values[1] == make_record(2));

  const Record repeated = make_record(7);
  values.insert(values.begin() + 1, 2, repeated);
  CHECK(values.size() == 5);
  CHECK(values[1] == repeated);
  CHECK(values[2] == repeated);

  auto after_one = values.erase(values.begin() + 1);
  CHECK(after_one == values.begin() + 1);
  CHECK(values.size() == 4);

  auto after_range = values.erase(values.begin() + 1, values.begin() + 3);
  CHECK(after_range == values.begin() + 1);
  CHECK(values.size() == 2);
  CHECK(values[0] == make_record(1));
  CHECK(values[1] == make_record(3));

  values.resize(4, make_record(9));
  CHECK(values.size() == 4);
  CHECK(values[2] == make_record(9));
  CHECK(values[3] == make_record(9));
  values.resize(1);
  CHECK(values.size() == 1);

  values.clear();
  CHECK(values.empty());
}

} // namespace

int main() {
  test_default_constructor_and_capacity();
  test_count_and_fill_constructors();
  test_push_pop_and_element_access();
  test_const_access_and_iterators();
  test_copy_move_assignment_and_swap();
  test_insert_erase_clear_and_resize();

  CHECK(Record::alive() == 0);
  if (failures != 0) {
    std::cerr << failures << " test(s) failed\n";
    return EXIT_FAILURE;
  }
  std::cout << "all vector tests passed\n";
  return EXIT_SUCCESS;
}
