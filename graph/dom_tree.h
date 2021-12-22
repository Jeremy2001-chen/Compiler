//
// Created by chenjiyuan3 on 2021/12/3.
//

#ifndef COMPILER_DOM_TREE_H
#define COMPILER_DOM_TREE_H

#include "tree.h"

vector<bool> domFlag;

class DomTree {
private:
    Tree* root;
    int point;
public:
    DomTree(int _point) {
        root = nullptr;
        point = _point;
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
//        cout << "Link : " << fa->getId() << " to " << tree->getId() << endl;
        fa -> insertChild(tree);
    }

    void ssaReName() {
        domFlag.resize(point);
        for (int i = 0; i < point; ++ i)
            domFlag[i] = false;
        root -> ssaReName();
    }

    void setRegister() {
        root -> setRegister();
    }

};

#endif //COMPILER_DOM_TREE_H
