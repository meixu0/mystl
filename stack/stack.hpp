#ifndef STACK_HPP
#define STACK_HPP
#include "../deque/deque.hpp"
namespace my_std {
template<class Type, class Container = my_std::deque<Type>>class stack{
public:
    using container_type = Container;
    using size_type = size_t;
    using value_type = Type;
    using reference = Type&;
    stack() = default;
    explicit stack(const container_type &right){
        container = right;
    }
    ~stack() = default;
    bool empty() const{ return container.empty(); }
    void pop(){ container.pop_back();}
    void push(const Type& value){ container.push_back(value);}
    value_type size() const{return container.size();}
    Type& top(){ return container.back();}
    const Type& top() const {return container.back();}
private:
    container_type container;
};
}

#endif