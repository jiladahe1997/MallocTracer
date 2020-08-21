#include <stdbool.h>
#include <stdint.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "MallocTracer.h"



/* ===================== hashmap.c ===============================*/
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
/**
 * author: Ren Mingrui https://github.com/jiladahe1997
 * description: A simple hash map realization using C. 
 * date: 2020/8/11
 * note: 
 * a. 由于C语言没有垃圾回收机制，故hashmap的每个value需要用户手动malloc。
 * b. 暂时只支持查找、添加、更改，不支持删除。
 * b. 使用方法：
 *    见test case.
 */

/* standard includes ---------------------------------------------------------*/
#include <stdbool.h>
#include <stddef.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include "hashmap.h"
/* base struct ---------------------------------------------------------------*/
/* BST二叉搜索树 */

struct Tree {
    struct Tree_Node * root;
    struct Tree_Node * (*search)(struct Tree * tree, char * key);
    bool (*insert)(struct Tree * tree, char * key, void * value);
};
/* hash array */
#define HASH_TABLE_MAX_LENGTH 7


/* function prototypes  -------------------------------------------------------*/
static bool tree_insert(struct Tree * tree, char * key, void * value);
static struct Tree_Node * tree_search(struct Tree * tree, char * key);
static struct HashMap_Iterator * hashmap_create_iterator(struct HashMap * hashmap);
static struct Tree_Node * tree_dfs(struct Tree_Node * node);

/* main code  -----------------------------------------------------------------*/

/* this func is copy from 
 *https://stackoverflow.com/questions/7666509/hash-function-for-string 
 */
static unsigned long
_hash(char *str)
{
    unsigned long hash = 5381;
    int c;

    while (c = *str++)
        hash = ((hash << 5) + hash) + c; /* hash * 33 + c */

    return hash;
}

/**
 * @brief put a key_value to a hash map
 * @param hashmap which hash map to opreate
 * @param key the key in key_value
 * @param value the value in key_value, must be a pointer, the user should malloc it.
 * @return bool true/false
 * 
 * @note 1.due C don't have a garbage collector (GC), user must malloc the value manually.
 * @note 2.when put a already element, this function will free the element's value inside. 
 */
static bool hashmap_put(struct HashMap * hashmap, char * key, void * value) {
    /* find from tab */
    unsigned long hash = _hash(key);
    struct Tree * targetTab = &hashmap->tab[(HASH_TABLE_MAX_LENGTH-1) & hash];
    
    /* new tab */
    if(!targetTab->root) {
        targetTab->root = (struct Tree_Node*)malloc(sizeof(struct Tree_Node));
        targetTab->root->hash = hash;
        targetTab->root->key = (char*)malloc(strlen(key));
        strcpy(targetTab->root->key, key);
        targetTab->root->value = value;
        targetTab->root->left = NULL;
        targetTab->root->right = NULL;
        targetTab->root->root = NULL;
        targetTab->root->next = NULL;
        targetTab->search = tree_search;
        targetTab->insert = tree_insert;
    } 
    /* exist tab */
    else if (targetTab->root) {
        struct Tree_Node * targetNode = targetTab->search(targetTab, key);
        /* new node*/
        if(!targetNode) {
            targetTab->insert(targetTab, key, value);
        } 
        else if (targetNode) {
            free(targetNode->value);
            targetNode->value = value;
        }
    }
		return true;
}

/**
 * @brief get a key_value from a hash map
 * @param hashmap which hash map to opreate
 * @param key the key in key_value
 * @return void * the pointer of value
 */
static void * hashmap_get(struct HashMap * hashmap, char * key) {
    unsigned long hash = _hash(key);
    struct Tree * targetTab = &hashmap->tab[(HASH_TABLE_MAX_LENGTH-1) & hash];
    if(!targetTab->root) return NULL;
    
	struct Tree_Node * t  ;
	return (t = targetTab->search(targetTab, key)) ? t->value : NULL;
}

/**
 * @brief delete a node from a tree, and return the root of the tree
 */
static struct Tree_Node * tree_delete(struct Tree_Node * root, char * key) {
    unsigned long hash = _hash(key);

    if(strcmp(key, root->key)==0) {
        free(root->value);
        free(root->key);
        /* leaf */
        if(!root->right && !root->left) {
            return NULL;
        }

        /* single child root */
        else if ((!root->right && root->left) || (root->right && !root->left)) {
            root = root->right ? root->right : root->left;
            return root;
        }

        /* tow children root , delete root and recursion each childre find another root */
        else if (root->right && root->left) {
            /*  find successor  */
            struct Tree_Node * minNode = root->right;
            for(;minNode->left!=NULL;) {
                minNode=minNode->left;
            }
            root->key = minNode->key;
            root->value = minNode->value;
            root->hash = minNode->hash;

            /* delete successor */
            root->right = tree_delete(root->right, minNode->key);
        }
    }

    else {
        if(hash > root->hash) {
            root->right = tree_delete(root->right, key);
        }
        else if (hash < root->hash){
            root->left = tree_delete(root->left, key);
        } 
    }
    return root;
}

static bool hashmap_delete(struct HashMap * hashmap, char * key) {
    unsigned long hash = _hash(key);
    struct Tree * targetTab = &hashmap->tab[(HASH_TABLE_MAX_LENGTH-1) & hash];
    struct Tree_Node * target = hashmap->get(hashmap, key);
    if(!target) {
        printf("can find delete target!");
        return false;
    }
    targetTab->root = tree_delete(targetTab->root, key);
		return true;
}

static void * hashmap_next(
    struct HashMap_Iterator * iterator, 
    struct HashMap * hashmap
) {
    void * ret = iterator->nextNode;

    /* find next */
    struct Tree_Node * nowNode = iterator->nextNode;
    iterator->nextNode = NULL;
    if(nowNode->next) {
        iterator->nextNode = nowNode->next;
    }
    else if(!nowNode->next){
        /* move to the next tab element */
        iterator->nowTab++;
        while(iterator->nowTab < HASH_TABLE_MAX_LENGTH){
            struct Tree * targetTab = &hashmap->tab[iterator->nowTab];
            if(targetTab->root) {
                iterator->nextNode = targetTab->root;
                break;
            }
            else if (!targetTab->root){
                iterator->nowTab++;
                continue;
            }
        }
    }

    return ret;
}   

static bool hashmap_has_next(struct HashMap_Iterator * iterator) {
    return (bool)iterator->nextNode;
}

static struct HashMap_Iterator * hashmap_create_iterator(struct HashMap * hashmap) {
    struct HashMap_Iterator * iterator = calloc(1, sizeof(struct HashMap_Iterator));
    iterator->next = hashmap_next;
    iterator->hasNext = hashmap_has_next;
    iterator->nowTab = 0;
    /* move to the fisrt element */
    while(iterator->nowTab < HASH_TABLE_MAX_LENGTH){
        struct Tree * targetTab = &(hashmap->tab[iterator->nowTab]);
        if(targetTab->root) {
            iterator->nextNode = targetTab->root;
            break;
        }
        else if (!targetTab->root){
            iterator->nowTab++;
            continue;
        }
    }
    return iterator;
}

/**
 * @brief get a value from a BST by key
 * @param tree which BST map to opreate
 * @param key the key in key_value
 * @return void * the pointer of value
 */
static struct Tree_Node * tree_search(struct Tree * tree, char * key) {
    unsigned long hash = _hash(key);
    /* use iteration insteadof recursion*/
    for(struct Tree_Node * node = tree->root;;) {
        if(strcmp(key, node->key) == 0) {
            return node;
        }
        
        if(hash <= node->hash) node=node->left;
        else if (hash > node->hash) node=node->right;
        if(node == NULL) return NULL;
    }
}

/**
 * @brief insert a value to a BST
 * @param tree which BST map to opreate
 * @param key the key in key_value
 * @param value the value in key_value
 * @return truen/false
 */
static bool tree_insert(struct Tree * tree, char * key, void * value) {
    unsigned long hash = _hash(key);
    struct Tree_Node * last_node = NULL;
    for(struct Tree_Node * node = tree->root;;){
        if(node == NULL) {
            node = (struct Tree_Node*)malloc(sizeof(struct Tree_Node));
            node->hash = hash;
            node->key = (char*)malloc(strlen(key));
            strcpy(node->key, key);
            node->left = NULL;
            node->right = NULL;
            node->root = last_node;
            node->value = value;
            node->next = last_node->next;                                                                                            
            last_node->next = node;
            if(hash<last_node->hash) last_node->left = node;
            else if (hash>last_node->hash) last_node->right = node;
            return true;
        }
        last_node = node;
        if(hash <= node->hash) node=node->left;
        else if (hash > node->hash) node=node->right;
    }
}

/**
 * @brief 构造函数，初始化对象
 * @param void
 * @return Hashmap对象
 * @note 1.due C don't have a garbage collector (GC), the Hashmap object could not free 
 *        automaticlly, 
 * 
 * @note 2.DO NOT FREE THE Hashmap OBJECT manually, becasue there are some other OBJECT, 
 *        which you could not FREE.
 */
struct HashMap * New_HashMap(){
    struct HashMap * hashmap;
    hashmap = (struct HashMap*)malloc(sizeof(struct HashMap));
    hashmap->get = hashmap_get;
    hashmap->put = hashmap_put;
    hashmap->delete = hashmap_delete;
    hashmap->createIterator = hashmap_create_iterator;
    hashmap->tab = (struct Tree*)malloc(HASH_TABLE_MAX_LENGTH * sizeof(struct Tree));
    memset(hashmap->tab, 0, HASH_TABLE_MAX_LENGTH * sizeof(struct Tree));
    return hashmap;
}




/* ===================== hashmap.c end ===============================*/




#define MALLOC_TRANCER_STRING_LENGTH_INFO_POSITION 128
#define MALLOC_TRANCER_SINGLE_POSITION_MAX_TRANCE  64     // 单个位置最多追踪64次，防止无限追踪内存泄漏

#ifndef STATIC
#define STATIC static
#endif

struct MallocTrancerInfo {
    char position[MALLOC_TRANCER_STRING_LENGTH_INFO_POSITION];  // 
    uint32_t ptr_address;
    int mallocCount;
    int freeCount;
};

STATIC bool is_init = false;
STATIC struct MallocTrancer mallocTrancer;
STATIC struct HashMap * hashmapPositionAll;
STATIC struct HashMap * hashmapAddressAll;

STATIC void getMallocInfo(void);

struct MallocTrancer * New_MallocTrancer(void) {
    if(!is_init) {
        mallocTrancer.getMallocInfo = getMallocInfo; 
        hashmapPositionAll = New_HashMap();
        hashmapAddressAll = New_HashMap();
    }
    return &mallocTrancer;
}


STATIC void getMallocInfo(void){
    struct HashMap_Iterator * iteratorPosition = hashmapPositionAll->createIterator(hashmapPositionAll);
    rt_kprintf("\r\n -----------------------------------------------------------------------------------------------------");
    rt_kprintf("\r\n TABLE1: POSTION-MALLOC/FREE                                                                         |");
    rt_kprintf("\r\n                                                                                                     |");
    rt_kprintf("\r\n %-64s | %#-10s | %5s | %5s      |", "POSITION", "ADDRESS" , "MALLOC", "FREE|");
    while(iteratorPosition->hasNext(iteratorPosition)){
        struct Tree_Node * node = (struct Tree_Node*)iteratorPosition->next(iteratorPosition, hashmapPositionAll);
        struct MallocTrancerInfo * info = (struct MallocTrancerInfo*)node->value;
        char str[256] = "";
        snprintf(str, 256, "\r\n %-64s | %#lx | %6d | %6d     |", info->position, (unsigned long)info->ptr_address ,info->mallocCount, info->freeCount);
        rt_kprintf(str);
    } 
    free(iteratorPosition);
    rt_kprintf("\r\n -----------------------------------------------------------------------------------------------------");
    rt_kprintf("\r\n TABLE2: ADDRESS-POSITION                                                                            |");
    rt_kprintf("\r\n                                                                                                     |");
    rt_kprintf("\r\n %-10s | %-64s                       |", "ADDRESS", "POSITION");
    struct HashMap_Iterator * iteratorAddress = hashmapAddressAll->createIterator(hashmapAddressAll);
    while(iteratorAddress->hasNext(iteratorAddress)){
        struct Tree_Node * node = (struct Tree_Node*)iteratorAddress->next(iteratorAddress, hashmapAddressAll);
        struct MallocTrancerInfo * info = (struct MallocTrancerInfo*)node->value;
        char str[256] = "";
        snprintf(str, 256, "\r\n %-10s | %-64s                       |",node->key ,info->position);
        rt_kprintf(str);
    } 
    rt_kprintf("\r\n -----------------------------------------------------------------------------------------------------");
    free(iteratorAddress);
    return;
}

void * _trace_malloc(size_t size,  const char *file, const char *func,const long line) {
    void * ret = malloc(size);
    uint32_t address = (uint32_t)ret;
    char addressStr[16] = "";
    snprintf(addressStr, sizeof(addressStr), "%#X", address);
    if(!ret) {
        log_w("malloc fail!");
        return ret;
    }

    char position[MALLOC_TRANCER_STRING_LENGTH_INFO_POSITION] = "";
    snprintf(position, MALLOC_TRANCER_STRING_LENGTH_INFO_POSITION, 
            "%s-%ld-%s", file, line, func);

    struct MallocTrancerInfo * _mallocTrancerInfo = (struct MallocTrancerInfo*)hashmapPositionAll->get(hashmapPositionAll, position);
    if(!_mallocTrancerInfo) {
        /* this address not trace before */
        struct MallocTrancerInfo * newMallocTrancerInfo = malloc(sizeof(struct MallocTrancerInfo));
        strlcpy(newMallocTrancerInfo->position, position, MALLOC_TRANCER_STRING_LENGTH_INFO_POSITION);
        newMallocTrancerInfo->freeCount=0;
        newMallocTrancerInfo->mallocCount = 1;
        newMallocTrancerInfo->ptr_address = address;
        hashmapPositionAll->put(hashmapPositionAll, position, newMallocTrancerInfo);
    }
    else if (_mallocTrancerInfo) {
        struct MallocTrancerInfo * newMallocTrancerInfo = malloc(sizeof(struct MallocTrancerInfo));
        strlcpy(newMallocTrancerInfo->position, position, MALLOC_TRANCER_STRING_LENGTH_INFO_POSITION);
        newMallocTrancerInfo->freeCount=_mallocTrancerInfo->freeCount;
        newMallocTrancerInfo->mallocCount = _mallocTrancerInfo->mallocCount+1;
        newMallocTrancerInfo->ptr_address = address;
        /* @note: no need to free the old value, the hashmap will free it automaticlly */
        hashmapPositionAll->put(hashmapPositionAll, position, newMallocTrancerInfo);
    }

    char * _p = (char*)malloc(MALLOC_TRANCER_STRING_LENGTH_INFO_POSITION);
    strlcpy(_p, position, MALLOC_TRANCER_STRING_LENGTH_INFO_POSITION);
    hashmapAddressAll->put(hashmapAddressAll, addressStr, _p);

    return ret;

}

void _trace_free(void * ptr,const char *file, const char *func,const long line) {
    free(ptr);
    uint32_t address = (uint32_t)ptr;
    char addressStr[16] = "";
    snprintf(addressStr, sizeof(addressStr), "%#X", address);

    char * position = (char*)hashmapAddressAll->get(hashmapAddressAll, addressStr);
    if(!position) {
        log_w("free trance fial, address not malloc find before free");
        return;
    }
    struct MallocTrancerInfo * _mallocTrancerInfo = (struct MallocTrancerInfo*)hashmapPositionAll->get(hashmapPositionAll, position);
    if(!_mallocTrancerInfo) {
        log_w("free trance fial, position not malloc find before free");
        return;
    }
    struct MallocTrancerInfo * newMallocTrancerInfo = malloc(sizeof(struct MallocTrancerInfo));
    strlcpy(newMallocTrancerInfo->position, position, MALLOC_TRANCER_STRING_LENGTH_INFO_POSITION);
    newMallocTrancerInfo->freeCount=_mallocTrancerInfo->freeCount+1;
    newMallocTrancerInfo->mallocCount = _mallocTrancerInfo->mallocCount;
    newMallocTrancerInfo->ptr_address = address;
    /* @note: no need to free the old value, the hashmap will free it automaticlly */
    hashmapPositionAll->put(hashmapPositionAll, position, newMallocTrancerInfo);
 
    hashmapAddressAll->delete(hashmapAddressAll, addressStr);
    return;
}