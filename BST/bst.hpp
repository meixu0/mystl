#ifndef BST_HPP
#define BST_HPP
#include <cstddef>
#include <cstdlib>

namespace bst{

template<typename T>struct TreeNode{
        T value;
        TreeNode *lchild, *rchild, *parent;
};

template <typename T> class BST{
public:
    BST() = default;
    ~BST(){destroy(root);}
    BST (const BST& o) {
        if(o.root == nullptr)   return;
        construct_by_copy(o.root);
    }
    BST (BST&& o) noexcept{
        if(root || !o.root){
            destroy(root);
            sz = 0;
        }
        else{
            destroy(root);
            root = o.root;
            sz = o.sz;  
        }
    }
    BST& operator=(const BST& o){
        if(o.root == nullptr){
            destroy(root);
        }
        if(root && o.root != nullptr && root != o.root){
            destroy(root);
            construct_by_copy(o.root);
        }
        if(!root){
            construct_by_copy(o.root);
        }
       return *this;
    }
    BST& operator=(const BST&& o){
        if(root && o.root != nullptr && root != o.root){
            root = o.root;
            sz = o.sz;
        }
        return *this;
    }
    void insert(const T& value){
        if(!root){
            root = new TreeNode<T>();
            root->value = value;
            root->parent = nullptr;
            root->lchild = nullptr;
            root->rchild = nullptr;
            ++sz;
            return;
        }
        TreeNode<T>* cur = root;
        while(cur){
            if(value < cur->value){
                TreeNode<T>* tmp = cur;
                cur = cur->lchild;
                if(!cur){
                    cur = new TreeNode<T>();
                    cur->value = value;
                    cur->parent = tmp;
                    tmp->lchild = cur;
                    ++sz;
                    return;
                }
            }else{
                TreeNode<T>* tmp = cur;
                cur = cur->rchild;
                if(!cur){
                    cur = new TreeNode<T>();
                    cur->value = value;
                    cur->parent = tmp;
                    tmp->rchild = cur;
                    ++sz;
                    return;
                }
            }
        }

    }
    size_t size() const noexcept {return sz;}
    bool empty() const noexcept {return sz == 0;}

    TreeNode<T>* successor(TreeNode<T> *ptr){
        if(!ptr)    return nullptr;
        TreeNode<T>* cur = ptr;
        if(cur->rchild){
            return min_node(cur->rchild);
        }
        TreeNode<T>* parent = cur->parent;
        while(parent && cur == parent->rchild){
            cur = parent;
            parent = parent->parent;
        }
        return parent;
    } 

    TreeNode<T>* minimum(){
        TreeNode<T>* cur = root;
        while(cur->lchild){
            cur = cur->lchild;
        }
        return cur;
    }
    TreeNode<T>* min_node(TreeNode<T>* ptr){
        TreeNode<T>* cur = ptr;
        while(cur->lchild){
            cur = cur->lchild;
        }
        return cur;
    }
    TreeNode<T>* max_node(TreeNode<T>* ptr){
        TreeNode<T>* cur = ptr;
        while(cur->rchild){
            cur = cur->rchild;
        }
        return cur;
    }

    class iterator{
    public:
        using value_type = TreeNode<T>;
        using pointer = TreeNode<T>*;
        using reference = TreeNode<T>&;
        iterator(TreeNode<T>* p): ptr(p) {};
        T& operator*() const{ return ptr->value; }
        bool operator==(const iterator& o) const {return ptr == o.ptr;}
        bool operator!=(const iterator& o) const {return ptr != o.ptr;}
        iterator& operator++(){
            TreeNode<T>* cur = ptr;
            if(cur->rchild){
                cur = cur->rchild;
                while(cur->lchild){
                    cur = cur->lchild;
                }
                ptr = cur;
                return *this;
            }
            TreeNode<T>* parent = ptr->parent;
            while(parent && cur == parent->rchild){
                cur = parent;
                parent = parent->parent;
            }
            ptr = parent;
            return *this;
        }
        iterator& operator++(int){
            iterator tmp = *this;
            ++(*this);
            return tmp;
        }
        iterator& operator--(){
            TreeNode<T>* cur = ptr;
            if(cur->lchild){
                cur = cur->lchild;
                while(cur->rchild){
                    cur = cur->rchild;
                }
                ptr = cur;
                return *this;
            }
            TreeNode<T> *parent = ptr->parent;
            while(parent && cur == parent->lchild){
                cur = parent;
                parent = parent->parent;
            }
            ptr = parent;
            return *this;
        }
        iterator& operator--(int){
            iterator tmp = *this;
            --(*this);
            return tmp;
        }

    private:
        TreeNode<T>* ptr;
    };

    iterator begin(){
        if(!root)   return nullptr;
        return iterator(minimum());
    }
    iterator end() {return nullptr;}
    iterator find(const T& val){
        TreeNode<T>* cur = root;
        if(!root)   return nullptr;
        while(cur){
            if(val < cur->value)    cur = cur->lchild;
            else if(val > cur->value)    cur = cur->rchild;
            else if(val == cur->value)   return iterator(cur);
        }
        return nullptr;
    }
    const iterator find(const T& val) const {
        TreeNode<T>* cur = root;
        if(!root)   return nullptr;
        while(cur){
            if(val < cur->value)    cur = cur->lchild;
            else if(val > cur->value)    cur = cur->rchild;
            else if(val == cur->value)   return iterator(cur);
        }
        return nullptr;
    }
    iterator erase(const T& val){
        TreeNode<T>* cur = root;
        if(!cur)    return nullptr;
        if(val == cur->value){
            destroy(root);
            return nullptr;
        }
        while(cur && val != cur->value){
            if(val < cur->value)    cur = cur->lchild;
            if(val > cur->value)    cur = cur->rchild;
        }
        TreeNode<T>* parent = cur->parent;
        if(!cur->lchild && !cur->rchild){
            if(cur->value < cur->parent->value){
                cur->parent->lchild = nullptr;
            }else{
                cur->parent->rchild = nullptr;
            }
            delete cur;
            --sz;
            return iterator(parent);
        }
        if(cur && cur->lchild && !cur->rchild){
            if(cur->value < cur->parent->value){
                cur->lchild->parent = cur->parent;
                cur->parent->lchild = cur->lchild;
            }else{
                cur->lchild->parent = cur->parent;
                cur->parent->rchild = cur->lchild;
            }
            delete cur;
            --sz;
            return iterator(parent);
        }
        if(cur && cur->rchild && !cur->lchild){
            if(cur->value < cur->parent->value){
                cur->rchild->parent = cur->parent;
                cur->parent->lchild = cur->rchild;
            }else{
                cur->rchild->parent = cur->parent;
                cur->parent->rchild = cur->rchild;
            }
            delete cur;
            --sz;
            return iterator(parent);
        }
        if(cur && cur->lchild && cur->rchild){
            TreeNode<T>* suc = successor(cur);
            if(suc == cur->rchild){
                if(cur->value < parent->value){
                    parent->lchild = suc;
                    suc->lchild = cur->lchild;
                }else{
                    parent->rchild = suc;
                    suc->lchild = cur->lchild;
                }
            }else{
                if(suc->rchild) suc->rchild->parent = suc->parent;
                else{return nullptr;}
                if(suc->value < suc->parent->value){
                    suc->parent->lchild = suc->rchild;
                }else{
                    suc->parent->rchild = suc->rchild;
                }
                suc->parent = cur->parent;
                suc->lchild = cur->lchild;
                suc->rchild = cur->rchild;
                cur->parent->lchild = suc;
            }
            delete cur;
            --sz;
            return iterator(parent);
        }
        
    }
    void clear(){destroy(root);}

private:
    size_t sz = 0;
    TreeNode<T> *root = nullptr;
    void construct_by_copy(TreeNode<T>* cur){
        if(cur != nullptr){
            insert(cur->value);
            construct_by_copy(cur->lchild);
            construct_by_copy(cur->rchild);
        }
    }
    void destroy(TreeNode<T>*& ptr){
        if(!ptr)    return;
        destroy(ptr->lchild);
        destroy(ptr->rchild);
        delete ptr;
        ptr = nullptr;
        --sz;
    }
};

}
#endif