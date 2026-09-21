#include "ttree.h"

int ttree_height(const TTreeNode *node) {
    return node ? node->height : 0;
}

void ttree_update_height(TTreeNode *node) {
    if (!node) return;

    int left = ttree_height(node->left);
    int right = ttree_height(node->right);

    node->height = 1 + (left > right ? left : right);
}

int ttree_balance_factor(const TTreeNode *node) {
    return node
        ? ttree_height(node->left) - ttree_height(node->right)
        : 0;
}

static TTreeNode *rotate_right(TTreeNode *root) {
    TTreeNode *pivot = root->left;

    root->left = pivot->right;
    pivot->right = root;

    /* Primero actualizar el nodo que baja. */
    ttree_update_height(root);
    ttree_update_height(pivot);

    return pivot;
}

static TTreeNode *rotate_left(TTreeNode *root) {
    TTreeNode *pivot = root->right;

    root->right = pivot->left;
    pivot->left = root;

    ttree_update_height(root);
    ttree_update_height(pivot);

    return pivot;
}

TTreeNode *ttree_rebalance(TTreeNode *node) {
    if (!node) return NULL;

    ttree_update_height(node);
    int balance = ttree_balance_factor(node);

    if (balance > 1) {
        /* LR: rotación izquierda del hijo antes de la derecha. */
        if (ttree_balance_factor(node->left) < 0) {
            node->left = rotate_left(node->left);
        }

        /* LL: basta con rotación derecha. */
        return rotate_right(node);
    }

    if (balance < -1) {
        /* RL: rotación derecha del hijo antes de la izquierda. */
        if (ttree_balance_factor(node->right) > 0) {
            node->right = rotate_right(node->right);
        }

        /* RR: basta con rotación izquierda. */
        return rotate_left(node);
    }

    return node;
}