/* Integrante 1: nodos, memoria y construcción por inserciones. */
#include "ttree_internal.h"
#include <stdlib.h>
#include <string.h>
#include <assert.h>
static void *default_alloc(size_t n, void *ctx) { (void)ctx; return malloc(n); }
static void default_free(void *p, void *ctx) { (void)ctx; free(p); }
void tt_init(TTree *t, TTAlloc a, TTFree f, void *ctx) {
    assert((a == NULL) == (f == NULL));
    memset(t, 0, sizeof(*t));
    t->alloc = a ? a : default_alloc;
    t->release = f ? f : default_free;
    t->allocator_context = ctx;
}
TTNode *tt_node_new(TTree *t, int32_t key) {
    TTNode *n = t->alloc(sizeof(*n), t->allocator_context);
    if (!n) return NULL;
    memset(n, 0, sizeof(*n));
    n->keys[0] = key; n->count = 1; n->height = 1; t->nodes++;
    return n;
}
static void clear_node(TTree *t, TTNode *n) {
    if (!n) return;
    clear_node(t, n->left); clear_node(t, n->right);
    t->release(n, t->allocator_context);
}
void tt_clear(TTree *t) {
    clear_node(t, t->root); t->root = NULL; t->nodes = 0; t->size = 0;
}
TTResult tt_build(TTree *t, const int32_t *keys, size_t count) {
    TTree next;
    size_t i;
    tt_init(&next, t->alloc, t->release, t->allocator_context);
    for (i = 0; i < count; ++i) {
        TTResult result = tt_insert(&next, keys[i]);
        if (result != TT_INSERTED) { tt_clear(&next); return result; }
    }
    tt_clear(t); *t = next;
    return TT_INSERTED;
}
size_t tt_memory_bytes(const TTree *t) { return sizeof(*t) + t->nodes * sizeof(TTNode); }
