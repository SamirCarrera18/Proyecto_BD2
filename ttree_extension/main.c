#include "ttree.h"

int main(void) {
    TTree *tree = ttree_create();
    if (!tree) return EXIT_FAILURE;

    for (int key = 1; key <= 100; key++) {
        if (!ttree_insert(tree, key)) {
            ttree_destroy(tree);
            return EXIT_FAILURE;
        }
    }

    printf("Elementos: %zu\n", tree->total_items);
    printf("Altura: %d\n", ttree_height(tree->root));
    printf("Balance de raiz: %d\n", ttree_balance_factor(tree->root));
    printf("Existe 50: %s\n", ttree_search(tree, 50) ? "si" : "no");

    ttree_destroy(tree);
    return EXIT_SUCCESS;
}