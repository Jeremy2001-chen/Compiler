//
// Created by chenjiyuan3 on 2021/12/3.
//

#ifndef COMPILER_DOM_TREE_H
#define COMPILER_DOM_TREE_H

#include "tree.h"

class DomTree {
private:
    Tree* root;
public:
    DomTree() {
        root = nullptr;
    }

    void setRoot(Tree* _new) {
        root = _new;
    }

    void insert(Tree* tree, vector<int>* tran) {
        Tree* fa = root, *next = nullptr;
        for (int i = 1; i < tran->size(); ++ i) {
            next = fa->findChild((*tran)[i]);
            fa = next;
        }
        fa -> insertChild(tree);
    }
};

#endif //COMPILER_DOM_TREE_H
