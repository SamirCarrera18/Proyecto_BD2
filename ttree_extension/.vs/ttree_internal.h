#ifndef TTREE_INTERNAL_H
#define TTREE_INTERNAL_H
#include "ttree.h"
TTNode *tt_node_new(TTree *, int32_t);
int tt_height(const TTNode *);
void tt_update_height(TTNode *);
TTNode *tt_rebalance(TTNode *);
#endif
