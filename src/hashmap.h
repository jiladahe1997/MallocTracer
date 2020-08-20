#ifndef __BSL_HASHMAP_H
#define __BSL_HASHMAP_H

struct HashMap * New_HashMap();

struct Tree_Node {
    unsigned long hash;
    char * key;           
    void * value;
    struct Tree_Node * left;
    struct Tree_Node * right;
    struct Tree_Node * root;
    struct Tree_Node * next;
};

struct HashMap {
    struct Tree * tab;
    void * (*get)(struct HashMap * hashmap, char * key);
    bool (*put)(struct HashMap * hashmap, char * key, void * value);
    bool (*delete)(struct HashMap * hashmap, char * key);
    struct HashMap_Iterator * (*createIterator)(struct HashMap * hashmap);
};

struct HashMap_Iterator {
    bool (*hasNext)(struct HashMap_Iterator * iterator);
    void * (*next)(    
        struct HashMap_Iterator * iterator, 
        struct HashMap * hashmap
    );
    int nowTab;
    struct Tree_Node * nextNode;
};

#endif /*__BSL_HASHMAP_H */
