#include "ttree.h"

// Instancia y reserva memoria para un nuevo nodo
TTreeNode* ttree_create_node(void) {
    TTreeNode *node = (TTreeNode*) malloc(sizeof(TTreeNode));
    if (!node) {
        return NULL; // Previene fallos si el SO se queda sin memoria
    }

    node->count = 0;
    node->height = 1; // Altura inicial de un nodo hoja
    node->left = NULL;
    node->right = NULL;
    return node;
}

// Libera un solo nodo de la memoria RAM
void ttree_free_node(TTreeNode *node) {
    if (node) {
        free(node);
    }
}

// Inicializa la estructura controladora del árbol
TTree* ttree_create(void) {
    TTree *tree = (TTree*) malloc(sizeof(TTree));
    if (!tree) {
        return NULL;
    }

    tree->root = NULL;
    tree->total_items = 0;
    return tree;
}

// Función auxiliar en Post-Orden (Izquierda -> Derecha -> Nodo)
static void ttree_destroy_nodes(TTreeNode *node) {
    if (node == NULL) {
        return; // Caso base recursivo
    }
    
    ttree_destroy_nodes(node->left);
    ttree_destroy_nodes(node->right);
    ttree_free_node(node);
}

// Destruye la totalidad del árbol y vacía la memoria
void ttree_destroy(TTree *tree) {
    if (!tree) return;
    
    ttree_destroy_nodes(tree->root);
    free(tree); // Libera la cabecera principal
}

// Construye el árbol llamando secuencialmente al módulo de inserción (I3)
bool ttree_build(TTree *tree, const KeyType *keys, size_t count) {
    if (!tree || !keys) {
        return false;
    }

    for (size_t i = 0; i < count; i++) {
        if (!ttree_insert(tree, keys[i])) {
            return false; // Retorna falso si alguna inserción falla
        }
    }
    return true;
}
