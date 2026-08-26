#include "list_c.h"
#include <stdint.h>  
#include <stdio.h>
int main(){
    /*
    List list;
    init(&list);
    int a = 1;
    char ch = 'a';
    push_back(&list, (void*)&a);
    for(Node* node = list.head->next; node != list.head; node = node->next)  printf("%d\n", *(int*)node->data);
    push_front(&list, (void*)(&ch));
    Node* node = list.head->next;
    printf("%c\n", *(char*)node->data);
    Node* n = node->next;
    printf("%d\n", *(int*)n->data);
    int c = 3;
    push_back(&list, (void*)&c);
    Node* node1 = list.head->prev;
    printf("%d\n", *(int*)node1->data);
    pop_back(&list);
    node1 = list.head->prev;
    printf("%c\n", *(char*)node1->data);
    printf("\n\n");
    int d = 1, e = 2, f = 3;
    List lst;
    init(&lst);
    push_back(&lst, (void*)(intptr_t) d);
    push_back(&lst, (void*)(intptr_t) e);
    push_back(&lst, (void*)(intptr_t) f);
    for(Node* n = lst.head->next; n != lst.head; n = n->next)  printf("%d\n", (int)(intptr_t)n->data);
    pop_back(&lst);
    pop_front(&lst);
    for(Node* n = lst.head->next; n != lst.head; n = n->next)  printf("%d\n", (int)(intptr_t)n->data);
    printf("\n\n");
    */
    //init pushback pushfront popback popfront size empty ok

    List l;
    init(&l);
    int aa =1, bb = 2, cc = 3, dd = 4;
    push_back(&l, (void*) &aa);
    push_back(&l, (void*) &bb);
    push_back(&l, (void*) &cc);
    push_back(&l, (void*) &dd);
    printf("size%zu\n", size(&l));
    for(Node* nd = l.head->next; nd != l.head; nd = nd->next) printf("%d ", *(int*) nd->data);
    printf("\n\n");
    Node* ins = l.head->next->next;
    int ee = 5;
    insert(&l, ins, (void*) &ee);
    for(Node* nd = l.head->next; nd != l.head; nd = nd->next) printf("%d ", *(int*) nd->data);
    printf("\n\n");
    erase(&l, ins);
    for(Node* nd = l.head->next; nd != l.head; nd = nd->next) printf("%d ", *(int*) nd->data);
    printf("\n\n");
    Node* fd = find(&l, (void*) &cc);
    printf("%d", *(int*)fd->data);
    return 0;
}