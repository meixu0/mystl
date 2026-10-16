#include <cstddef>
#include <iostream>
#include <string>
#include "hashmap.hpp"
using std::string;
using std::cout;
int main(){
    my_std::Hashmap<int, int> map;
    map.insert(123, 1);
    map.insert(456, 2);
    /*my_std::Hashmap<string, int> m;
    m.insert("string1", 111);
    m.insert("string2", 222);*/
    
    //for(auto it = map.begin(); it != map.end(); ++it)  cout <<it->first << " " <<it->second <<"\n";
    //map.test();
    //cout<<map[123]<<"\n";
    
    /*auto it = map.begin();
    cout<<it->first<<" "<<it->second<<" "<<map.bucket_index(it->first)<<"\n";
    ++it;
    cout<<it->first<<" "<<it->second<<" "<<map.bucket_index(it->first)<<"\n";
    ++it;
    cout<<it->first<<" "<<it->second<<" "<<map.bucket_index(it->first)<<"\n";
    ++it;
    cout<<it->first<<" "<<it->second<<" "<<map.bucket_index(it->first)<<"\n";
    */
    for(auto it : map)  cout<<it.first<<" "<<it.second<<" "<<map.bucket_index(it.first)<<"\n";
    return 0;
}