//
// Created by chenjiyuan3 on 2021/12/3.
//

#ifndef COMPILER_TREE_H
#define COMPILER_TREE_H

#include "../ir/ir_block.h"

class Tree{
private:
    vector<Tree*>* ch;
    Tree* fa;
    IrBlock* irBlock;
    int id;
public:
    Tree(int _id, IrBlock* block) {
        ch = new vector<Tree*>();
        fa = nullptr;
        id = _id;
        irBlock = block;
    }

    void setFa(Tree* _fa) {
        fa = _fa;
    }

    int getId() {
        return id;
    }

    Tree* findChild(int x) {
        for (auto c: *ch)
            if (c -> id == x)
                return c;
        cout << "error when try to find child, now is " << id << " , child is " << x << endl;
        exit(65536);
    }

    void insertChild(Tree* tree) {
        (*ch).push_back(tree);
    }
};

#endif //COMPILER_TREE_H
