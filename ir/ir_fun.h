//
// Created by chenjiyuan3 on 2021/11/30.
//

#ifndef COMPILER_IR_FUN_H
#define COMPILER_IR_FUN_H

#include "ir_code.h"
#include <algorithm>
#include <utility>
#include "../list/mylist.h"
#include "../graph/graph.h"
#include "../graph/graph_ssa.h"
#include "ir_block.h"
#include "../graph/dom_tree.h"
#include "ir_table.h"
#include "../list/mylist_block.h"
#include "../mips/register.h"

extern IrTable IrTableList;
extern Register* aRegister;
extern MipsOutput* mipsOutput;
extern MipsTable* mipsTable;


class DataFlow {
private:
    int blockNum;
    string var;
public:
    DataFlow(int _block, string _var) {
        blockNum = _block;
        var = _var;
    }

    int getBlockNum() const {
        return blockNum;
    }

    string getVar() {
        return var;
    }
};

map<string, string>* varToRegister;

class IrFun {
private:
    vector<IrCode*>* codes;
    MyListBlock *fBlock, *eBlock;
    vector<MyListBlock*>* myBlocks;
    map<IrBlock*, MyListBlock*> blockToMy;
    vector<IrBlock*>* blocks;

    map<string, string>* funVarRegister;

    map<string, int> labelMp;
    vector<int> pos;
    vector<int> belong;
    vector<int> endBlocks;
    vector<bool> useful;
    string name;

    map<string, int> funNames;

    Graph* graph;
    DomTree* domTree;

public:
    explicit IrFun(vector<IrCode*>* _codes) {
        funVarRegister = new map<string, string>();

        codes = _codes;
        fBlock = eBlock = nullptr;
        if ((*codes)[0]->getCodeType() != IrFunDefineType) {
            exit(19373469);
        }
        IrFunDefine *funDefine = (IrFunDefine*)((*codes)[0]);
        name = funDefine->getName();
        cout << name << endl;
        for (int i = 2; i < (*codes).size() - 1; ++ i) {
            if ((*codes)[i]->getCodeType() == IrLabelLineType) {
                auto* line = (IrLabelLine*)(*codes)[i];
                labelMp[line->getLabel()] = i;
            }
        }
//      pos.push_back(1);
        for (int i = 2; i < (*codes).size() - 1; ++ i) {
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
        pos.erase(unique(pos.begin(), pos.end()), pos.end());
        blocks = new vector<IrBlock*>();
        myBlocks = new vector<MyListBlock*>();
        belong.push_back(0);
        int st = 0;
        vector<IrCode*>* bCode = new vector<IrCode*>();
        IrBlock* block = nullptr;
        bool onlyLabel = true;
        int block_cnt = 0;
        for (int i = 2; i < (*codes).size() - 1; ++ i) {
            if (st < pos.size() && i == pos[st]) {
                st ++;
                if (i > 1 && !onlyLabel) {
                    block = new IrBlock(bCode, block_cnt);
                    MyListBlock* nb = new MyListBlock(block);
                    blockToMy[block] = nb;
                    if (fBlock == nullptr)
                        fBlock = eBlock = nb;
                    else {
                        eBlock -> setNext(nb);
                        eBlock = nb;
                    }
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
        block = new IrBlock(bCode, block_cnt);
        MyListBlock* nb = new MyListBlock(block);
        blockToMy[block] = nb;
        if (fBlock == nullptr)
            fBlock = eBlock = nb;
        else {
            eBlock -> setNext(nb);
            eBlock = nb;
        }
        (*blocks).push_back(block);
        endBlocks.erase(unique(endBlocks.begin(), endBlocks.end()), endBlocks.end());

        //set final block
        block_cnt ++;
        bCode = new vector<IrCode*>();
        block = new IrBlock(bCode, block_cnt);
        nb = new MyListBlock(block);
        blockToMy[block] = nb;
        eBlock -> setNext(nb);
        eBlock = nb;
        (*blocks).push_back(block);

        int N = block_cnt + 1;
        graph = new Graph(N); // from 0 to N - 1
        useful.resize(N); // record the useful block
        block_cnt ++;

        //now we change the labelMp to record the block of label
        labelMp.clear();
        for (int i = 0; i < N; ++ i) {
            MyList* start = (*blocks)[i]->getStartCode();
            while (start != nullptr) {
                IrCode* code = start->getCode();
                if (code -> getCodeType() == IrLabelLineType) {
                    auto* line = (IrLabelLine*)code;
                    labelMp[line -> getLabel()] = i;
                }
                start = start -> getNext();
            }
        }

        for (int i = 0; i < N; ++ i) {
            MyList* start = (*blocks)[i]->getStartCode();
            while (start != nullptr) {
                IrCode* code = start->getCode();
                if (code -> getCodeType() == IrGotoStmtType) {
                    auto* line = (IrGotoStmt*)code;
                    int target = labelMp[line->getLabel()];
                    graph -> link(i, target);
                } else if (code -> getCodeType() == IrBranchStmtType) {
                    auto* line = (IrBranchStmt*)code;
                    int target = labelMp[line->getLabel()];
                    graph->link(i, target);
                }
                start = start -> getNext();
            }
            start = (*blocks)[i] -> getEndCode();
            if (start != nullptr && i < N - 1) {
                IrCode* code = start->getCode();
                //cout << i << " " << code->toString() << endl;
                if (code -> getCodeType() != IrReturnStmtType &&
                    code -> getCodeType() != IrGotoStmtType &&
                    code -> getCodeType() != IrExitType) {
                    graph->link(i, i + 1);
                }
            }
        }

        // link to end
        for (auto c: endBlocks)
            graph->link(c, N - 1);

        graph->ssaInit();

        vector<bool>* reach = graph->getReach();

        for (int i = 0; i < N; ++ i) {
            useful[i] = (*reach)[i];
            if (!useful[i])
                (*blocks)[i] -> kill();
        }

        // build dominate tree
        vector<vector<int>* >* dom = graph -> getDominate(); //x dominate who
        vector<vector<int> > newDom; //who dominate x, reserve for dominate tree
        newDom.resize(N);
        for (int i = 0; i < N; ++ i)
            for (int j = 0; j < (*dom)[i] ->size(); ++ j) {
                int c = (*(*dom)[i])[j];
                if (c && i != j) {
                    //cout << "hi: " << i << " " << j << endl;
                    newDom[j].push_back(i);
                }
            }
        domTree = new DomTree(N);
        for (int i = 0; i < N; ++ i) {
            if (!useful[i])
                continue;
            vector<int> *edges = graph -> getEdges(i);
            auto* blockT = new vector<IrBlock*>();
            for (auto edge: *edges)
                blockT->push_back((*blocks)[edge]);
            Tree* tree = new Tree(i, (*blocks)[i], blockT);
            if (i == 0)
                domTree -> setRoot(tree);
            else
                domTree -> insert(tree, &newDom[i]);
        }

        // init the name set
        for (int i = 0; i < N; ++ i) {
            auto* set = (*blocks)[i]->getNameSet();
            for (const auto& value: *set) {
                if (funNames.find(value) == funNames.end()) {
                    funNames[value] = 1;
                } else {
                    funNames[value] = funNames[value] + 1;
                }
            }
        }

        for (auto it = funNames.begin(); it != funNames.end(); ) {
            if (it -> second == 1 || it->first[0] == '@')
                funNames.erase(it++);
            else
                it++;
        }

        /*
        for (auto it = funNames.begin(); it != funNames.end(); it++) {
            cout << "var: " << (*it).first << endl;
        }
         */

        for (auto & funName : funNames) {
            string var = funName.first;
            vector<int>* tem = new vector<int>();
            for (int i = 0; i < N; ++ i) {
                auto* set = (*blocks)[i]->getNameSet();
                if (set -> find(var) != set -> end())
                    tem -> push_back(i);
            }
            vector<int>* result = graph -> getPhi(tem);
            for (auto c: *result) {
                IrPhi* phi = new IrPhi(var);
                (*blocks)[c] -> addIrCodeFront(phi);
            }
        }

        domTree -> ssaReName();
    }

    void dataAnalyse() {
        int N = blocks -> size();
        int block_cnt = N;

        //Init Def & Use
        for (int i = 0; i < N; ++ i) {
            if (!useful[i])
                continue;
            (*blocks)[i] -> calcDef();
            (*blocks)[i] -> calcUse();
        }

        //Work for the Phi Point
        for (int i = 0; i < N; ++ i) {
            if (!useful[i])
                continue;
            auto* phis = (*blocks)[i] -> getPhiList();
            for (const auto& c: *phis) {
                IrPhi* code = (IrPhi*)c.second->getCode();
                auto* from = code -> getFrom();
                auto* blockNum = code -> getBlockNum();
                for (int j = 0; j < from -> size(); ++ j) {
                    int to = (*blockNum)[j];
                    (*blocks)[to] -> putVarIntoUse((*from)[j]);
                    (*blocks)[to] -> putVarIntoPhiUse((*from)[j]);
                }
            }
        }

        //Start Calc In & Out
        queue <DataFlow*> Q;
        for (int i = 0; i < N; ++ i) {
            if (!useful[i])
                continue;
            (*blocks)[i] -> dataFlowInit();
            auto* def = (*blocks)[i] -> getDef();
            auto* use = (*blocks)[i] -> getUse();
            for (auto& c: *use) {
                if (def -> find(c) == def -> end())
                    Q.push(new DataFlow(i, c));
            }
        }

        while (!Q.empty()) {
            DataFlow* now = Q.front(); Q.pop();
            int blockId = now -> getBlockNum();
            string var = now -> getVar();
            auto* def = (*blocks)[blockId] -> getDef();
            if (def -> find(var) != def -> end())
                continue;
            (*blocks)[blockId] -> putVarIntoIn(var);
            auto* backEdges = graph -> getBackEdges(now -> getBlockNum());
            for (auto &to: *backEdges) {
                if (!useful[to])
                    continue;
                auto* out = (*blocks)[to] -> getOut();
                if (out -> find(var) == out -> end()) {
                    (*blocks)[to] -> putVarIntoOut(var);
                    DataFlow* next = new DataFlow(to, var);
                    Q.push(next);
                }
            }
        }

        /*
        for (int i = 0; i < N; ++ i) {
            if (!useful[i])
                continue;
            cout << "Now in block : " << i << endl;
            auto* in = (*blocks)[i] -> getIn();
            auto* out = (*blocks)[i] -> getOut();
            auto* def = (*blocks)[i] -> getDef();
            auto* use = (*blocks)[i] -> getUse();
            cout << "In : " << endl;
            for (auto &c: *in)
                cout << c << " ";
            cout << endl;
            cout << "Out : " << endl;
            for (auto &c: *out)
                cout << c << " ";
            cout << endl;
            cout << "Def : " << endl;
            for (auto &c: *def)
                cout << c << " ";
            cout << endl;
            cout << "Use : " << endl;
            for (auto &c: *use)
                cout << c << " ";
            cout << endl;
        }
        */
        //todo: kill not use code
        for (int i = 0; i < N; ++ i) {
            if (!useful[i])
                continue;
            auto* in = (*blocks)[i] -> getIn();
            auto* out = (*blocks)[i] -> getOut();
            auto* def = (*blocks)[i] -> getDef();
            auto* use = (*blocks)[i] -> getUse();
            auto* start = (*blocks)[i] -> getStartCode();
            while (start != nullptr) {
                if (start -> getCode() -> getCodeType() != IrReadIntegerType &&
                    start -> getCode() -> getCodeType() != IrParaDefineType) {
                    string target = start -> getCode() -> getTarget();
                    if (!target.empty() && target[0] == '%') {
                        if (use->find(target) == use->end() &&
                        out->find(target) == out->end())
                            (*blocks)[i] -> remove(start);
                    }
                }
                start = start -> getNext();
            }
        }

        //todo: set register
        aRegister -> clear();
//        varToRegister = new map<string, string>();
        varToRegister = funVarRegister;
        domTree -> setRegister();

        //cout << toString() << endl;
        //change phi to assign
        for (int i = 0; i < N; ++ i) {
            if (!useful[i])
                continue;
            auto* start = (*blocks)[i] -> getStartCode();
            while (start != nullptr) {
                if (start->getCode()->getCodeType() == IrLabelLineType) {
                    start = start -> getNext();
                } else
                    break;
            }
            while (start != nullptr) {
                if (start -> getCode() -> getCodeType() == IrPhiType) {
                    auto* code = (IrPhi*)(start -> getCode());
                    auto* from = code -> getFrom();
                    auto* blockNum = code->getBlockNum();
                    for (int j = 0; j < from -> size(); ++ j) {
                        int num = (*blockNum)[j];
                        MyList* endStmt = (*blocks)[num] -> getEndCode();
                        IrCode* endLine = endStmt -> getCode();
                        MyList* firstStmt = (*blocks)[i] -> getStartCode();
                        IrCode* firstLine = firstStmt -> getCode();
                        if (endLine -> getCodeType() != IrBranchStmtType)
                            (*blocks)[num] -> addIrCodeBack(new IrPhiAssign(code -> getTarget(), (*from)[j]));
                        else {
                            int bel = graph -> getBlockNum(num , i);
                            if (bel == -1) {
                                if (firstLine -> getCodeType() != IrLabelLineType)
                                    exit(999222);
                                MyListBlock* nb;
                                vector<IrCode*>* _code = new vector<IrCode*>();
                                if (num != i - 1) {
                                    string oldLabel = ((IrLabelLine*)firstLine) -> getLabel();
                                    string newLabel = irTableList_1.allocBranch();
                                    _code -> push_back(new IrLabelLine(newLabel));
                                    IrBranchStmt* branch = (IrBranchStmt*)endLine;
                                    branch -> setLabel(newLabel);
                                    labelMp[newLabel] = block_cnt;
                                    _code -> push_back(new IrGotoStmt(oldLabel));
                                    IrBlock* block1 = new IrBlock(_code, block_cnt);
                                    nb = new MyListBlock(block1);
                                    eBlock -> setNext(nb);
                                    eBlock = nb;
                                    blocks -> push_back(block1);
                                } else {
                                    IrBlock* block1 = new IrBlock(_code, block_cnt);
                                    nb = new MyListBlock(block1);
                                    MyListBlock* prev = blockToMy[(*blocks)[num]], *next = blockToMy[(*blocks)[i]];
                                    prev -> setNext(nb);
                                    nb -> setNext(next);
                                    blocks -> push_back(block1);
                                }
                                useful.push_back(true);
                                bel = block_cnt;
                                graph -> setBlockNum(num, i, bel);
//                                cout << num << " " << i << " " << bel << endl;
                                block_cnt ++;
                            }
                            (*blocks)[bel] -> addIrCodeBack(new IrPhiAssign(code -> getTarget(), (*from)[j]));
                        }
                    }
                    //(*blocks)[i] -> remove(start); //can't remove, for when you want to def the var, you need to use the phi point to alloc space
                } else
                    break;
                start = start -> getNext();
            }
        }

        //Remove Phi assign
        for (int i = 0; i < block_cnt; ++ i) {
            if (!useful[i])
                continue;
            //cout << "Remove Start : " << i << endl;
            auto* phi = (*blocks)[i] -> removePhiAssign();
            auto* graphSSA = new GraphSSA(phi);
            auto* newCodes = graphSSA -> getNewCode();
            for (auto g: *newCodes) {
                (*blocks)[i] -> addIrCodeBack(g);
            }
        }
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

    vector<IrCode*>* toIR() {
        vector<IrCode*>* newIR = new vector<IrCode*>();
        newIR -> push_back((*codes)[0]);
        newIR -> push_back((*codes)[1]);
        /*for (auto block: *blocks) {
            vector<IrCode*>* codesN = block -> toIR();
            for (auto code: *codesN)
                newIR -> push_back(code);
        }*/
        MyListBlock* start = fBlock;
        while (start != nullptr) {
            vector<IrCode*>* codesN = start -> getBlock() -> toIR();
            for (auto code: *codesN)
                newIR -> push_back(code);
            start = start -> getNext();
        }
        newIR->push_back(new IrFunEnd(0));
        return newIR;
    }

    void toMips() {
        varToRegister = funVarRegister;

        //def function
        mipsTable -> setLayer(1);

        (*codes)[1] -> toMips(); // name
        int spMove = 0;
        for (auto c: *blocks) {
            MyList* start = c -> getStartCode();
            while (start != nullptr) {
                IrCode* code = start -> getCode();
                spMove += code -> defVar();
                start = start -> getNext();
            }
        }

        set <string> reg;
        for (const auto& c: *varToRegister) {
            reg.insert(c.second);
        }

        if (name != "main") {
            for (const auto& c: reg) {
                spMove += mipsTable -> funInitStack(c, 1, false);
            }
        }

        spMove <<= 2;
        if (spMove > 0)
            mipsOutput -> push_back(new MipsAddI("subi", "$sp", "$sp", to_string(spMove)));

        int off = 0;
        if (name != "main") {
            for (auto it = reg.rbegin(); it != reg.rend(); ++ it, off+=4) {
                mipsOutput -> push_back(new MipsStore("sw", *it, off, "$sp"));
            }
        }

        MyListBlock* bl = fBlock;
        while (bl != nullptr) {
            int i = bl -> getBlock() -> getBlockNum();
            if (!useful[i]) {
                bl = bl -> getNext();
                continue;
            }
            MyList* start = (*blocks)[i] -> getStartCode();
            while (start != nullptr) {
                IrCode* code = start -> getCode();
//                cout << "now is : " << code -> toString() << endl;
                code -> toMips();
                if (code -> getCodeType() == IrReturnStmtType) {
                    off = 0;
                    if (name != "main") {
                        for (auto it = reg.rbegin(); it != reg.rend(); ++ it, off+=4) {
                            mipsOutput -> push_back(new MipsLoad("lw", *it, off, "$sp"));
                        }
                    }
                    if (spMove > 0)
                        mipsOutput -> push_back(new MipsAddI("addi", "$sp", "$sp", to_string(spMove)));
                    mipsOutput -> push_back(new MipsJRegister("jr", "$ra"));
                }
                start = start -> getNext();
            }
            bl = bl -> getNext();
        }

        // end function
        mipsTable -> setLayer(-1);
    }

    MyListBlock* getFirstBlock () {
        return fBlock;
    }
};
#endif //COMPILER_IR_FUN_H
