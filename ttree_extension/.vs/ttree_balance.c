/* Integrante 4: rotaciones AVL y redistribución especial LR/RL del T-Tree. */
#include "ttree_internal.h"
#include <string.h>
int tt_height(const TTNode *n) { return n?n->height:0; }
void tt_update_height(TTNode *n) {
    int a=tt_height(n->left), b=tt_height(n->right);
    n->height=1+(a>b?a:b);
}
static TTNode *rotate_right(TTNode *a) {
    TTNode *b=a->left; a->left=b->right; b->right=a;
    tt_update_height(a); tt_update_height(b); return b;
}
static TTNode *rotate_left(TTNode *a) {
    TTNode *b=a->right; a->right=b->left; b->left=a;
    tt_update_height(a); tt_update_height(b); return b;
}
TTNode *tt_rebalance(TTNode *a) {
    int balance;
    tt_update_height(a);
    balance=tt_height(a->left)-tt_height(a->right);
    if (balance>1) {
        TTNode *b=a->left;
        if (tt_height(b->right)>tt_height(b->left)) {
            TTNode *c=b->right;
            /* C hoja pasa a interno. Tomar las claves mayores de B dejando una. */
            if (!c->left && !c->right && c->count<TT_CAPACITY && b->count>1) {
                size_t move=TT_CAPACITY-c->count;
                if (move>b->count-1) move=b->count-1;
                memmove(c->keys+move,c->keys,c->count*sizeof(int32_t));
                memcpy(c->keys,b->keys+b->count-move,move*sizeof(int32_t));
                b->count-=move; c->count+=move;
            }
            a->left=rotate_left(b);
        }
        return rotate_right(a);
    }
    if (balance < -1) {
        TTNode *b=a->right;
        if (tt_height(b->left)>tt_height(b->right)) {
            TTNode *c=b->left;
            if (!c->left && !c->right && c->count<TT_CAPACITY && b->count>1) {
                size_t move=TT_CAPACITY-c->count;
                if (move>b->count-1) move=b->count-1;
                memcpy(c->keys+c->count,b->keys,move*sizeof(int32_t));
                memmove(b->keys,b->keys+move,(b->count-move)*sizeof(int32_t));
                b->count-=move; c->count+=move;
            }
            a->right=rotate_right(b);
        }
        return rotate_left(a);
    }
    return a;
}
