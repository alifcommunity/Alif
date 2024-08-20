#pragma once



class LListNode { // 34
public:
	LListNode* next{};
	LListNode* prev{};
};


// Get the struct containing a node.
#define LLIST_DATA(_node, _type, _member) (ALIF_CONTAINER_OF(_node, _type, _member)) // 40

// Iterate over a list.
#define LLIST_FOR_EACH(_node, _head) \
    for (_node = (_head)->next; _node != (_head); _node = _node->next)

// Iterate over a list, but allow removal of the current node.
#define LLIST_FOR_EACH_SAFE(_node, _head) \
    for (LListNode *next = (_node = (_head)->next, _node->next); \
         _node != (_head); _node = next, next = _node->next)

#define LLIST_INIT(_head) { &_head, &_head }

static inline void llist_init(LListNode* _head) {
    _head->next = _head;
    _head->prev = _head;
}

// Returns 1 if the list is empty, 0 otherwise.
static inline int llist_empty(LListNode* _head)
{
    return _head->next == _head;
}

// Appends to the tail of the list.
static inline void llist_insertTail(LListNode* _head, LListNode* _node) {
    _node->prev = _head->prev;
    _node->next = _head;
    _head->prev->next = _node;
    _head->prev = _node;
}

// Remove a node from the list.
static inline void llist_remove(LListNode* _node) {
    LListNode* prev_ = _node->prev;
    LListNode* next_ = _node->next;
    prev_->next = next_;
    next_->prev = prev_;
    _node->prev = nullptr;
    _node->next = nullptr;
}

// Append all nodes from head2 onto head1. head2 is left empty.
static inline void llist_concat(LListNode* _head1, LListNode* _head2) {
    if (!llist_empty(_head2)) {
        _head1->prev->next = _head2->next;
        _head2->next->prev = _head1->prev;

        _head1->prev = _head2->prev;
        _head2->prev->next = _head1;
        llist_init(_head2);
    }
}
