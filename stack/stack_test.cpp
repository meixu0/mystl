#include "../string/string.hpp"
#include "stack.hpp"
#include <iostream>
using std::endl;
using std::cout;
using my_std::stack;
using my_std::string;
int main(){
    string a = "abc";
    string b = "def";
    stack<string> stack1;
    stack1.push(a);
    cout<<stack1.top()<<endl;
    stack1.pop();
    stack1.push(a);
    stack1.push(b);
    cout<<stack1.top()<<endl;
    return 0;
}