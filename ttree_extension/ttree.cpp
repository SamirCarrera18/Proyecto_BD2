#include "ttree.hpp"

// Constructor del Nodo: Asigna valores iniciales seguros
TTreeNode::TTreeNode() 
    : count(0), height(1), left(nullptr), right(nullptr) {}

// Constructor del Árbol
TTree::TTree() 
    : root(nullptr), total_items(0) {}

// Destructor: Se ejecuta automáticamente al salir del scope o hacer 'delete'
TTree::~TTree() {
    destroyNodes(root);
}

// Recorrido Post-Orden usando operadores nativos de C++
void TTree::destroyNodes(TTreeNode* node) {
    if (node == nullptr) {
        return;
    }

    destroyNodes(node->left);
    destroyNodes(node->right);
    delete node; // Libera la memoria instanciada con 'new'
}

// Construcción mediante inserciones sucesivas
bool TTree::build(const std::vector<KeyType>& keys) {
    for (const auto& key : keys) {
        if (!insert(key)) {
            return false; // Error en la inserción delegada a I3
        }
    }
    return true;
}