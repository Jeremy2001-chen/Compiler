//
// Created by chenjiyuan3 on 2021/12/3.
//

#ifndef COMPILER_TREE_H
#define COMPILER_TREE_H

#include "../ir/ir_block.h"

extern vector<bool> domFlag;

class Tree{
private:
    vector<IrBlock*>* edges;
    vector<Tree*>* ch;
    Tree* fa;
    IrBlock* irBlock;
    int id;
public:
    Tree(int _id, IrBlock* block, vector<IrBlock*>* _edges) {
        edges = _edges;
        ch = new vector<Tree*>();
        fa = nullptr;
        id = _id;
        irBlock = block;
    }

    void setFa(Tree* _fa) {
        fa = _fa;
    }

    int getId() const {
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

    void ssaReName() {
        if (domFlag[id])
            return ;
        domFlag[id] = true;
        irBlock -> ssaReName();
        map<string, string>* names = irBlock -> getFinalNames();
        for (auto &it : *names) {
            for (auto c: *ch) {
                c->irBlock->putNameIntoFinalNames(it.first, it.second);
            }
        }
        for (auto c: *edges) {
            map<string, MyList*>* phi = c->getPhiList();
            for (auto &it : *phi) {
                ((IrPhi*)it.second->getCode())->putVar((*names)[it.first]);
            }
        }
        for (auto c: *ch)
            c -> ssaReName();
    }
};

#endif //COMPILER_TREE_H
