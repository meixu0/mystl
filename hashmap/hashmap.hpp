#ifndef UNORDERED_MAP_HPP
#define UNORDERED_MAP_HPP
#include <cassert>
#include <cstddef>
#include <cstdlib>
#include <functional>
#include <list>
#include <utility>
#include <vector>
#include <list>
#include <iostream>
#include "hash.hpp"

namespace my_std {
template<class Key, class Ty, class Hash = my_std::hash<Key>, class Pred = std::equal_to<Key>> class Hashmap{
public:
    using value_type = std::pair<Key, Ty>;
    using bucket_type = std::list<value_type>;
    using list_it = typename bucket_type::iterator;
    Hashmap(): sz(0), bucket(8), bucket_cnt(8){}
    ~Hashmap() = default;
    Hashmap& operator=(Hashmap &o){
        if(&o == *this) return *this;
        sz = o.sz;
        bucket = o.bucket;
        return *this;
    }
   Hashmap& operator=(Hashmap &&o){
        sz = o.sz;
        bucket = o.bucket;
    }
    Ty& operator[](const Key key){
        size_t idx = Hash() (key) % bucket_cnt;
        auto cur = bucket[idx];
        for(auto it = bucket[idx].begin(); it != bucket[idx].end(); ++it){
            //std::cout<<it->second;
            if(it->first == key){
                return it->second;
            }
        }
        //return nullptr;
    }
    void insert(Key k, const Ty value){
        std::size_t idx = Hash()(k) % bucket_cnt;
        //std::cout << "idx=" << idx <<"\n";
        if(idx >= bucket.size())    bucket.resize(idx);
        //std::cout <<"bucket size"<<bucket.size()<< "\n";
        bucket[idx].push_front(std::make_pair(k, value));
        //for(auto it = bucket[idx].begin(); it != bucket[idx].end(); ++it)    std::cout << it->first <<" " << it->second << "\n";
        ++sz;
    }
    class iterator{
    public:
        iterator(Hashmap *h, size_t idx, list_it it): hmap(h), bucket_idx(idx), _it(it),bucket_size(h->bucket.size() - 1){}
        ~iterator() = default;
        value_type& operator*() const {return *_it;}
        value_type* operator->() const {return &(*_it);}
        iterator& operator++(){
            if(bucket_idx == bucket_size){
                if(_it != hmap->bucket[bucket_idx].end() && !hmap->bucket[bucket_idx].empty()){
                    ++_it;
                    return *this;
                }else{
                    return *this;
                }
            }
            if(++_it != hmap->bucket[bucket_idx].end()){
                return *this;
            }else{
                ++bucket_idx;
                while(bucket_idx < bucket_size){
                    ++bucket_idx;
                    auto cur = &hmap->bucket[bucket_idx];
                    if(!cur->empty()){
                        _it = cur->begin();
                        return *this;
                    }
                }
                return *this;
            }
        }
        bool operator==(const iterator& o){return (hmap == o.hmap && bucket_idx == o.bucket_idx && _it == o._it);}
        bool operator!=(const iterator& o) {return !(this->hmap== o.hmap && this->bucket_idx == o.bucket_idx);}
    private:
        Hashmap *hmap;
        std::size_t bucket_idx;
        list_it _it;
        size_t bucket_size;
    };
    iterator begin() {
        for(size_t i = 0; i < bucket.size(); ++i){
            if(!bucket[i].empty()){
                return iterator(this, i, bucket[i].begin());
            }
        }
        return end();
    }
    iterator end(){
        assert(!bucket.empty());
        return iterator(this, bucket.size(), bucket[bucket.size() - 1].end());
    }
    size_t size(){return sz;}
    iterator find(Key k){
        size_t idx = Hash() (k) % bucket_cnt;
        if(idx > bucket.size() || bucket[idx].empty()) return end();
        else{
            auto cur = bucket[idx];
            for(auto it = cur.begin(); it != cur.end(); ++it){
                if(it->first == k)  return iterator(this, idx, it);
            }
            return end();
        }
    }
    iterator erase(iterator pos){
        if(pos == end())    return end();
        iterator next = pos;
        ++next;
        size_t idx = Hash() (pos->first);
        auto cur = bucket[idx];
        cur.erase(pos);
        sz--;
        return next;
    }
    bool empty() const {return sz == 0;}
    void clear(){
        bucket.clear();
        sz = 0;
    }
    size_t bucket_index(const Key& k) const{
        return Hash()(k) % bucket_cnt;
    }
private:
    std::size_t sz;
    std::size_t bucket_cnt;
    std::vector<bucket_type> bucket;

};
}

#endif