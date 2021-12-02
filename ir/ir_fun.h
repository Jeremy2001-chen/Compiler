//
// Created by chenjiyuan3 on 2021/11/30.
//

#ifndef COMPILER_IR_FUN_H
#define COMPILER_IR_FUN_H

#include "ir_code.h"
#include <algorithm>
#include "../list/mylist.h"
#include "../graph/graph.h"

class IrBlock {
private:
    vector<IrCode*>* codes;
    set<string> names;
    MyList* fStmt, *eStmt;
public:
    IrBlock(vector<IrCode*>* _code) {
        fStmt = eStmt = nullptr;
        codes = _code;
        if (!(*codes).empty()) {
            fStmt = new MyList((*codes)[0]);
            MyList* run = fStmt;
            for (int i = 1; i < (*codes).size(); ++ i) {
                eStmt = new MyList((*codes)[i]);
                run->setNext(eStmt);
                run = eStmt;
            }
        }
    }

    string toString() {
        MyList* start = fStmt;
        string ret;
        while (start != nullptr) {
            ret += start->getCode()->toString() + "\n";
            start = start->getNext();
        }
        return ret;
    }
};

class IrFun {
private:
    vector<IrCode*>* codes;
    vector<IrBlock*>* blocks;

    map<string, int> labelMp;
    vector<int> pos;
    vector<int> belong;
    vector<int> endBlocks;
    string name;

    Graph* graph;
public:
    explicit IrFun(vector<IrCode*>* _codes) {
        codes = _codes;
        if ((*codes)[0]->getCodeType() != IrFunDefineType) {
            exit(19373469);
        }
        IrFunDefine *funDefine = (IrFunDefine*)((*codes)[0]);
        name = funDefine->getName();
        for (int i = 2; i < (*codes).size() - 1; ++ i) {
            if ((*codes)[i]->getCodeType() == IrLabelLineType) {
                auto* line = (IrLabelLine*)(*codes)[i];
                labelMp[line->getLabel()] = i;
            }
        }
        pos.push_back(1);
        for (int i = 1; i < (*codes).size() - 1; ++ i) {
            if ((*codes)[i]->getCodeType() == IrGotoStmtType) {
                auto* line = (IrGotoStmt*)(*codes)[i];
                pos.push_back(labelMp[line->getLabel()]);
                pos.push_back(i + 1);
            } else if ((*codes)[i]->getCodeType() == IrBranchStmtType) {
                auto* line = (IrBranchStmt*)(*codes)[i];
                pos.push_back(labelMp[line->getLabel()]);
                pos.push_back(i + 1);
            }
        }
        pos.push_back((int)(*codes).size() - 1);
        sort(pos.begin(), pos.end());
        unique(pos.begin(), pos.end());
        blocks = new vector<IrBlock*>();
        belong.push_back(0);
        int st = 0;
        vector<IrCode*>* bCode = new vector<IrCode*>();
        IrBlock* block = nullptr;
        bool onlyLabel = true;
        int block_cnt = 0;
        for (int i = 1; i < (*codes).size() - 1; ++ i) {
            if (st < pos.size() && i == pos[st]) {
                st ++;
                if (i > 1 && !onlyLabel) {
                    block = new IrBlock(bCode);
                    block_cnt = block_cnt + 1;
                    (*blocks).push_back(block);
                    bCode = new vector<IrCode*>();
                    onlyLabel = true;
                }
            }
            (*bCode).push_back((*codes)[i]);
            if ((*codes)[i]->getCodeType() != IrLabelLineType)
                onlyLabel = false;
            belong.push_back(block_cnt);
            if ((*codes)[i]->getCodeType() == IrExitType || (*codes)[i]->getCodeType() == IrReturnStmtType)
                endBlocks.push_back(block_cnt);
        }
        block = new IrBlock(bCode);
        (*blocks).push_back(block);
        unique(endBlocks.begin(), endBlocks.end());

        //set final block
        block_cnt ++;
        bCode = new vector<IrCode*>();
        block = new IrBlock(bCode);
        (*blocks).push_back(block);

        int N = block_cnt + 1;
        graph = new Graph(N); // from 0 to N - 1

        for (int i = 1; i < (*codes).size() - 1; ++ i) {
            int now = belong[i];
            if ((*codes)[i]->getCodeType() == IrGotoStmtType) {
                auto* line = (IrGotoStmt*)(*codes)[i];
                int target = belong[labelMp[line->getLabel()]];
                graph->link(now, target);
            } else if ((*codes)[i]->getCodeType() == IrBranchStmtType) {
                auto* line = (IrBranchStmt*)(*codes)[i];
                int target = belong[labelMp[line->getLabel()]];
                graph->link(now, target);
            }
        }

        for (int i = 0; i < N - 2; ++ i)
            graph->link(i, i + 1);
        for (auto c: endBlocks)
            graph->link(c, N - 1);

        graph->ssaInit();
    }

    void test() {
        cout << "now in a new fun: " << name << endl;
        for (int i = 0; i < (*blocks).size(); ++ i) {
            cout << "in a new block: " << i << endl;
            cout << (*blocks)[i]->toString();
        }
        /*for(auto & i : *codes)
            cout << i->toString() << endl;*/

    }

    string toString() {
        string ret;
        ret += "now in a new fun: " + name + "\n";
        for (int i = 0; i < (*blocks).size(); ++ i) {
            ret += "in a new block: " + to_string(i) + "\n";
            ret += (*blocks)[i]->toString() + "\n";
        }
        return ret;
    }
};
#endif //COMPILER_IR_FUN_H
