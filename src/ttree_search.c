/* Integrante 2: búsqueda por intervalo del nodo y búsqueda binaria interna. */
#include "ttree.h"
#include <stdio.h>
#include <limits.h>
bool tt_contains(const TTree *t, int32_t key) {
    const TTNode *n = t->root;
    while (n) {
        size_t lo, hi;
        if (key < n->keys[0]) { n = n->left; continue; }
        if (key > n->keys[n->count-1]) { n = n->right; continue; }
        lo = 0; hi = n->count;
        while (lo < hi) {
            size_t mid = lo + (hi-lo)/2;
            if (n->keys[mid] == key) return true;
            if (n->keys[mid] < key) lo = mid+1; else hi = mid;
        }
        return false;
    }
    return false;
}
static bool visit(const TTNode *n, int32_t lo, int32_t hi, TTVisit fn, void *ctx) {
    size_t i;
    if (!n) return true;
    if (lo < n->keys[0] && !visit(n->left, lo, hi, fn, ctx)) return false;
    for (i = 0; i < n->count; ++i)
        if (n->keys[i] >= lo && n->keys[i] <= hi && !fn(n->keys[i],ctx)) return false;
    if (hi > n->keys[n->count-1] && !visit(n->right, lo, hi, fn, ctx)) return false;
    return true;
}
bool tt_visit_range(const TTree *t, int32_t lo, int32_t hi, TTVisit fn, void *ctx) {
    return lo > hi || visit(t->root,lo,hi,fn,ctx);
}
typedef struct Validation { size_t keys, nodes; const char *error; } Validation;
static int validate_node(const TTNode *n, int64_t lo, int64_t hi, Validation *v) {
    int left, right, height;
    size_t i;
    if (!n || v->error) return 0;
    if (n->count == 0 || n->count > TT_CAPACITY) { v->error="Ocupación inválida"; return 0; }
    if (n->left && n->right && n->count < TT_MIN_INTERNAL) {
        v->error="Nodo interno por debajo de ocupación mínima"; return 0;
    }
    for (i=0;i<n->count;++i) {
        if (n->keys[i] <= lo || n->keys[i] >= hi || (i && n->keys[i-1] >= n->keys[i])) {
            v->error="Orden o límites de claves incorrectos"; return 0;
        }
    }
    v->keys += n->count; v->nodes++;
    left=validate_node(n->left,lo,n->keys[0],v);
    right=validate_node(n->right,n->keys[n->count-1],hi,v);
    height=1+(left>right?left:right);
    if (!v->error && (left-right>1 || right-left>1)) v->error="Desbalance AVL";
    if (!v->error && n->height != height) v->error="Altura almacenada incorrecta";
    return height;
}
bool tt_validate(const TTree *t, char *message, size_t length) {
    Validation v={0,0,NULL};
    validate_node(t->root,(int64_t)INT32_MIN-1,(int64_t)INT32_MAX+1,&v);
    if (!v.error && (v.keys!=t->size || v.nodes!=t->nodes)) v.error="Contadores incorrectos";
    if (message && length) snprintf(message,length,"%s",v.error?v.error:"OK");
    return v.error==NULL;
}
