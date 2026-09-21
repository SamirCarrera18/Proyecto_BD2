#include "ttree.h"

static bool insert_into_node_sorted(TTreeNode *node, KeyType key) {
    if (node->count >= NODE_CAPACITY) return false;

    int i = node->count - 1;

    while (i >= 0 && node->keys[i] > key) {
        node->keys[i + 1] = node->keys[i];
        i--;
    }

    node->keys[i + 1] = key;
    node->count++;

    return true;
}

static TTreeNode *insert_recursive(
    TTreeNode *node,
    KeyType key,
    bool *inserted
) {
    if (!node) {
        TTreeNode *new_node = ttree_create_node();
        if (!new_node) return NULL;

        new_node->keys[0] = key;
        new_node->count = 1;
        *inserted = true;

        return new_node;
    }

    if (key < node->keys[0]) {
        node->left = insert_recursive(node->left, key, inserted);
    } else if (key > node->keys[node->count - 1]) {
        node->right = insert_recursive(node->right, key, inserted);
    } else {
        /*
         * Conserva la política actual:
         * admite repetidos cuando hay espacio.
         *
         * La redistribución de nodos llenos sigue pendiente.
         */
        *inserted = insert_into_node_sorted(node, key);
    }

    /* Rebalancear de abajo hacia arriba si se insertó la clave. */
    return *inserted ? ttree_rebalance(node) : node;
}

bool ttree_insert(TTree *tree, KeyType key) {
    if (!tree) return false;

    bool inserted = false;

    tree->root = insert_recursive(tree->root, key, &inserted);

    if (inserted) {
        tree->total_items++;
    }

    return inserted;
}