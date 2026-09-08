#include "../string/string.hpp"
#include "deque.hpp"
#include <iostream>
#include <ostream>
using std::cout;
using std::endl;
using my_std::string;
using my_std::deque;
int main(){
    string a = "abc";
    string b = "def";
    deque<string> deque1;
    deque1.push_back(a);
    cout<<deque1.back()<<endl;
    cout<<deque1.empty()<<endl;
    deque1.clear();
    deque1.push_front(b);
    cout<<deque1.back()<<endl;
    cout<<deque1.front()<<endl;
    deque1.push_back(a);
    cout<<deque1.back()<<endl;
    return 0;
}