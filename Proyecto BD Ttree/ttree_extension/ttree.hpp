#ifndef TTREE_HPP
#define TTREE_HPP

#include <cstddef>
#include <vector>

constexpr size_t NODE_CAPACITY = 8;
using KeyType = int;

// Estructura del nodo con constructor predeterminado
struct TTreeNode {
    KeyType keys[NODE_CAPACITY];
    size_t count;
    int height;
    TTreeNode* left;
    TTreeNode* right;

    TTreeNode(); // Inicializa el nodo seguro
};

// Clase contenedora del Árbol T
class TTree {
private:
    TTreeNode* root;
    size_t total_items;

    // Auxiliar recursivo para la liberación de memoria
    void destroyNodes(TTreeNode* node);

public:
    TTree();
    ~TTree(); // Destructor: evita fugas automáticamente

    // Métodos de acceso (Getters)
    TTreeNode* getRoot() const { return root; }
    size_t getTotalItems() const { return total_items; }

    // Construcción inicial (Tarea de I1)
    bool build(const std::vector<KeyType>& keys);

    // Inserción (Módulo a implementar por el Integrante 3)
    bool insert(KeyType key);
};

#endif