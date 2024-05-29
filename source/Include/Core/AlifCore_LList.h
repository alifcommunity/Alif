#pragma once

#include <stddef.h>


class LListNode {
public:
    class LListNode* next_;
    class LListNode* prev_;
};

    // Get the struct containing a node.
#define LLIST_DATA(_node, _type, _member) (ALIFSUB_CONTAINER_OF(_node, _type, _member))

// Iterate over a list.
#define LLIST_FOR_EACH(_node, _head) \
    for (_node = (_head)->next_; _node != (_head); _node = _node->next_)

// Iterate over a list, but allow removal of the current node.
#define LLIST_FOR_EACH_SAFE(_node, _head) \
    for (class LLISTNODE *next_ = (_node = (_head)->next_, _node->next_); \
         _node != (_head); _node = next_, next_ = _node->next_)

#define LLIST_INIT(_head) { &_head, &_head }

static inline void lList_init(class LListNode* _head)
{
    _head->next_ = _head;
    _head->prev_ = _head;
}

// Returns 1 if the list is empty, 0 otherwise.
static inline int lList_empty(class LListNode* _head)
{
    return _head->next_ == _head;
}

// Appends to the tail of the list.
static inline void lList_insert_tail(class LListNode* _head, class LListNode* _node)
{
    _node->prev_ = _head->prev_;
    _node->next_ = _head;
    _head->prev_->next_ = _node;
    _head->prev_ = _node;
}

// Remove a node from the list.
static inline void lList_remove(class LListNode* _node)
{
    class LListNode* prev_ = _node->prev_;
    class LListNode* next_ = _node->next_;
    prev_->next_ = next_;
    next_->prev_ = prev_;
    _node->prev_ = nullptr;
    _node->next_ = nullptr;
}

// Append all nodes from head2 onto head1. head2 is left empty.
static inline void lList_concat(class LListNode* _head1, class LListNode* _head2)
{
    if (!lList_empty(_head2)) {
        _head1->prev_->next_ = _head2->next_;
        _head2->next_->prev_ = _head1->prev_;

        _head1->prev_ = _head2->prev_;
        _head2->prev_->next_ = _head1;
        lList_init(_head2);
    }
}