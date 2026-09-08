#ifndef DEQUE_HPP
#define DEQUE_HPP
#include <list>
namespace my_std {
template <class Type> class deque{
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
    deque() = default;
    ~deque() = default;
    void push_front(const Type& value) {container.push_front(value);}
    void push_back(const Type& value) {container.push_back(value);}
    void push_front(Type&& value) {container.push_front(value);}
    void push_back(Type&& value) {container.push_back(value);}
    void pop_front() {container.pop_front();}
    void pop_back() {container.pop_back();}
    size_type size() {return container.size();}
    size_type max_size() {return container.max_size();}
    void clear() {container.clear();}
    bool empty() {return container.empty();}
    reference front() {return container.front();}
    const_reference front() const {return container.front();}
    reference back(){return container.back();}
    const_reference back()const {return container.back();}
    reference operator[](size_type idx){return container[idx];}
    const_reference operator[](size_type idx) const {return container[idx];}
private:
    std::list<Type> container;
};
}
#endif