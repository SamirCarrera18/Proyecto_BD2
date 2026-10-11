/* Integrante 3: inserción T-Tree; desborde del nodo acotador hacia su predecesor. */
#include "ttree_internal.h"
#include <string.h>
static void insert_in_node(TTNode *n, int32_t key) {
    size_t p=0;
    while (p<n->count && n->keys[p]<key) ++p;
    memmove(n->keys+p+1,n->keys+p,(n->count-p)*sizeof(int32_t));
    n->keys[p]=key; n->count++;
}
static TTNode *insert_rec(TTree *t, TTNode *n, int32_t key, bool *ok) {
    TTNode *child;
    if (!n) { n=tt_node_new(t,key); *ok=n!=NULL; return n; }
    if (key < n->keys[0]) {
        if (!n->left && n->count<TT_CAPACITY) { insert_in_node(n,key); return n; }
        child=insert_rec(t,n->left,key,ok);
        if (!*ok) return n;
        n->left=child;
    } else if (key > n->keys[n->count-1]) {
        if (!n->right && n->count<TT_CAPACITY) { insert_in_node(n,key); return n; }
        child=insert_rec(t,n->right,key,ok);
        if (!*ok) return n;
        n->right=child;
    } else {
        if (n->count<TT_CAPACITY) { insert_in_node(n,key); return n; }
        /* Primero reservar/insertar la clave desplazada: fallo de memoria no altera n. */
        child=insert_rec(t,n->left,n->keys[0],ok);
        if (!*ok) return n;
        n->left=child;
        memmove(n->keys,n->keys+1,(n->count-1)*sizeof(int32_t));
        n->count--; insert_in_node(n,key);
    }
    return tt_rebalance(n);
}
TTResult tt_insert(TTree *t, int32_t key) {
    bool ok=true;
    if (tt_contains(t,key)) return TT_DUPLICATE;
    t->root=insert_rec(t,t->root,key,&ok);
    if (!ok) return TT_OOM;
    t->size++;
    return TT_INSERTED;
}
