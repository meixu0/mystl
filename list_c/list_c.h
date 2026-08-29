#ifndef LIST_C_H
#define LIST_C_H
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>

typedef struct Node Node;
typedef struct List List;

typedef int (*compare)(const void *a, const void *b);
typedef void (*destory_node)(void *);

struct Node {
  Node *next, *prev;
  void *data;
};
struct List {
  Node *head;
  size_t sz;
  destory_node destory;
};

static inline void init(List *list, destory_node destory) {
  Node *node = malloc(sizeof *node);
  node->data = NULL;
  list->head = node;
  list->head->next = list->head;
  list->head->prev = list->head;
  list->sz = 0;
  list->destory = destory;
}

static inline int empty(const List *list) { return list->sz == 0 ? 1 : 0; }

static inline size_t size(const List *list) { return list->sz; }

static inline void delete_node(List *list, Node *node, destory_node destory) {
  if (node == NULL || node == list->head)
    return;
  node->prev->next = node->next;
  node->next->prev = node->prev;
  if (list->destory != NULL)
    destory(node);
  free(node);
  list->sz--;
}

static inline void delete_list(List *list, destory_node destory) {
  if (empty(list) == 1)
    return;
  Node *tmp = malloc(sizeof *tmp);
  tmp = list->head->next;
  while (tmp != list->head) {
    Node *nextnode = tmp;
    tmp = tmp->next;
    destory(nextnode);
    free(nextnode);
    nextnode = NULL;
    list->sz--;
  }
  list->sz = 0;
}

static inline void push_back(List *list, void *data) {
  Node *newnode = malloc(sizeof *newnode);
  newnode->data = data;
  if (newnode == NULL || data == NULL)
    return;
  Node *tmp = list->head->prev;
  tmp->next = newnode;
  newnode->prev = tmp;
  list->head->prev = newnode;
  newnode->next = list->head;
  list->sz++;
}
static inline void push_front(List *list, void *data) {
  Node *newnode = malloc(sizeof *newnode);
  newnode->data = data;
  if (newnode == NULL || data == NULL)
    return;
  Node *tmp = list->head->next;
  tmp->prev = newnode;
  newnode->next = tmp;
  list->head->next = newnode;
  newnode->prev = list->head;
  list->sz++;
}

static inline Node *pop_back(List *list) {
  if (empty(list) == 1)
    return NULL;
  Node *tmp = list->head->prev;
  list->head->prev = tmp->prev;
  tmp->prev->next = list->head;
  list->sz--;
  return tmp;
  free(tmp);
  tmp = NULL;
}
static inline Node *pop_front(List *list) {
  if (empty(list) == 1)
    return NULL;
  Node *tmp = list->head->next;
  list->head->next = tmp->next;
  tmp->next->prev = list->head;
  list->sz--;
  return tmp;
  free(tmp);
  tmp = NULL;
}

static inline void insert(List *list, Node *node, void *data) {
  if (node == list->head->next) {
    push_front(list, data);
    list->sz++;
    return;
  }
  if (node->next == list->head->prev) {
    push_back(list, data);
    list->sz++;
    return;
  }
  Node *newnode = malloc(sizeof *newnode);
  if (newnode == NULL || data == NULL)
    return;
  newnode->data = data;
  node->prev->next = newnode;
  node->prev = newnode;
  newnode->next = node;
  list->sz++;
}

static inline Node *front(List *list) {
  if (empty(list) == 1)
    return NULL;
  return list->head->next;
}
static inline Node *back(List *list) {
  if (empty(list) == 1)
    return NULL;
  return list->head->prev;
}

static inline void erase(List *list, void *data,
                         int (*compare)(const void *a, const void *b)) {
  if (empty(list) == 1)
    return;
  if (size(list) == 1)
    pop_back(list);
  Node *n = list->head->next;
  while (n->next != list->head->next) {
    if (compare(data, n->data) == 0) {
      n->next->prev = n->prev;
      n->prev->next = n->next;
      list->sz--;
      return;
    }
    n = n->next;
  }
}

static inline Node *find(List *list, void *data,
                         int (*compare)(const void *a, const void *b)) {
  Node *n = list->head->next;
  while (n->next != list->head->next) {
    if (compare(data, n->data) == 0)
      return n;
    n = n->next;
  }
  return NULL;
}

#endif
