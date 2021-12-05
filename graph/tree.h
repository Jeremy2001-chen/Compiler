//
// Created by chenjiyuan3 on 2021/12/3.
//

#ifndef COMPILER_TREE_H
#define COMPILER_TREE_H

#include "../ir/ir_block.h"
#include "../mips/register.h"

extern vector<bool> domFlag;

extern Register* aRegister;
extern map<string, string>* varToRegister;

class Tree{
private:
    vector<IrBlock*>* edges;
    vector<Tree*>* ch;
    Tree* fa;
    IrBlock* irBlock;
    int id;
    bool use[32]; int time[32];

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
            for (auto c: *edges) {
                if (!domFlag[c->getBlockNum()]) {
                    //cout << "update: " << id << " " << c->getBlockNum() << endl;
                    c->putNameIntoFinalNames(it.first, it.second);
                }
            }
        }
        for (auto c: *edges) {
            map<string, MyList*>* phi = c->getPhiList();
            for (auto &it : *phi) {
                if (!(*names)[it.first].empty())
                    ((IrPhi*)it.second->getCode())->putVar((*names)[it.first], id);
            }
        }
        for (auto c: *ch)
            c -> ssaReName();
    }

    void setRegister() {
        vector<vector<bool>* > tem;
        MyList* end = irBlock -> getEndCode();
        set<string> hasKill;
        auto* out = irBlock -> getOut();
        while (end != nullptr) {
            vector<bool>* kill = new vector<bool>(2);
            (*kill)[0] = (*kill)[1] = false;
            tem.push_back(kill);
            IrCode* code = end -> getCode();
            if (code -> getCodeType() != IrPhiType) {
                for (int i = 0; i < 2; ++ i) {
                    string source = code -> getSource(i);
                    if (!source.empty() && source != "%0" && source[0] == '%') {
                        if (out->find(source) == out -> end() && hasKill.find(source) == hasKill.end()) {
                            (*kill)[i] = true;
                            hasKill.insert(source);
                        }
                    }
                }
            }
            end = end -> getPrev();
        }

        MyList* start = irBlock -> getStartCode();
        int index = tem.size() - 1;
        while (start != nullptr) {
            IrCode* code = start -> getCode();
            if (code -> getCodeType() != IrPhiType) {
                for (int i = 0; i < 2; ++ i) {
                    string source = code -> getSource(i);
                    if ((*tem[index])[i]) {
                        if (varToRegister -> find(source) != varToRegister -> end()) {
                            aRegister -> release((*varToRegister)[source]);
                        }
                    }
                }
            }
            string target = code -> getTarget();
            if (!target.empty() && target[0] == '%') {
                string reg = aRegister -> alloc();
                if (!reg.empty())
                    (*varToRegister)[target] = reg;
            }
            start = start -> getNext();
            index --;
        }

        auto* gUse = aRegister -> getRegisterUsed();
        auto* gTime = aRegister -> getRegisterTime();
        for (int i = 0; i < 32; ++ i) {
            use[i] = gUse[i];
            time[i] = gTime[i];
        }
        for (auto c: *ch) {
            c -> setRegister();
            aRegister -> setUsed(use);
            aRegister -> setTime(time);
        }
    }
};

#endif //COMPILER_TREE_H
