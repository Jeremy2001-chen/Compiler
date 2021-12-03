//
// Created by chenjiyuan3 on 2021/11/30.
//

#ifndef COMPILER_IR_FUN_H
#define COMPILER_IR_FUN_H

#include "ir_code.h"
#include <algorithm>
#include "../list/mylist.h"
#include "../graph/graph.h"

extern map<string, int> globalNameCount;

class IrBlock {
private:
    vector<IrCode*>* codes;
    set<string>* names;
    MyList* fStmt, *eStmt;
    map<string, string>* finalNames;
    map<string, MyList*>* paiList;
    int blockNum;
public:
    IrBlock(vector<IrCode*>* _code, int id) {
        fStmt = eStmt = nullptr;
        codes = _code;
        paiList = new map<string, MyList*>();
        finalNames = new map<string, string>();
        names = new set<string>();
        blockNum = id;
        if (!(*codes).empty()) {
            fStmt = new MyList((*codes)[0]);
            MyList* run = fStmt;
            for (int i = 1; i < (*codes).size(); ++ i) {
                eStmt = new MyList((*codes)[i]);
                run -> linkNext(eStmt);
                run = eStmt;
            }
            eStmt = run;
        }
        MyList* start = fStmt;
        while (start != nullptr) {
            IrCode* code = start->getCode();
            // Remove (Ret x & Exit)
            if (code->getCodeType() == IrReturnStmtType || code->getCodeType() == IrExitType) {
                start -> setNext(nullptr);
                eStmt = start;
                break;
            }
            string target = code->getTarget();
            if (!target.empty())
                names -> insert(target);
            start = start -> getNext();
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

    set<string>* getNameSet() {
        return names;
    }

    void addIrCodeBack(IrCode* irCode) {
        MyList* list = new MyList(irCode);
        if (fStmt == nullptr) {
            eStmt = fStmt = list;
        } else {
            eStmt -> linkNext(list);
            eStmt = list;
        }
        if (irCode->getCodeType() == IrPhiType) {
            (*paiList)[irCode->getTarget()] = list;
        }
    }

    void addIrCodeFront(IrCode* irCode) {
        MyList* list = new MyList(irCode);
        if (fStmt == nullptr) {
            eStmt = fStmt = list;
        } else {
            list -> linkNext(fStmt);
            fStmt = list;
        }
        if (irCode->getCodeType() == IrPhiType) {
            (*paiList)[irCode->getTarget()] = list;
        }
    }

    void ssaReName() {
        MyList* start = fStmt;
        while (start != nullptr) {
            IrCode* code = start->getCode();
            for (int i = 0; i < 2; ++ i) {
                string sc = code->getSource(i);
                if (sc.empty() || sc == "%0" || sc[0] == '@') continue;
                if (globalNameCount.find(sc) == globalNameCount.end()) {
                    cout << "error in IR, can't find : " << sc << endl;
                    exit(7654321);
                }
                int cnt = globalNameCount[sc];
                string newName = sc + "_" + to_string(cnt);
                code->setSource(i, newName);
            }
            string target = code->getTarget();
            if (!target.empty() && target[0] != '@') {
                if (globalNameCount.find(target) == globalNameCount.end())
                    globalNameCount[target] = 1;
                else
                    globalNameCount[target] = globalNameCount[target] + 1;
                int cnt = globalNameCount[target];
                string newName = target + "_" + to_string(cnt);
                code->setTarget(newName);
            }
            start = start->getNext();
        }
        for (const string& name: *names) {
            string newName = name + "_" + to_string(globalNameCount[name]);
            (*finalNames)[name] = newName;
        }
    }

    map<string, MyList*>* getPhiList() {
        return paiList;
    }

    MyList* getStartCode() {
        return fStmt;
    }

    MyList* getEndCode() {
        return eStmt;
    }

    vector<IrCode*>* toIR() {
        vector<IrCode*>* newIR = new vector<IrCode*>();
        MyList* start = fStmt;
        while (start != nullptr) {
            IrCode* code = start->getCode();
            newIR ->push_back(code);
            start = start -> getNext();
        }
        return newIR;
    }

    void kill() {
        names -> clear();
        fStmt = eStmt = nullptr;
        finalNames -> clear();
        paiList -> clear();
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
    vector<bool> useful;
    string name;

    map<string, int> funNames;

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
                    block = new IrBlock(bCode, block_cnt);
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
        (*blocks).push_back(block);
        unique(endBlocks.begin(), endBlocks.end());

        //set final block
        block_cnt ++;
        bCode = new vector<IrCode*>();
        block = new IrBlock(bCode, block_cnt);
        (*blocks).push_back(block);

        int N = block_cnt + 1;
        graph = new Graph(N); // from 0 to N - 1

        useful.resize(N); // record the useful block

        for (int i = 0; i < N; ++ i) {
            MyList* start = (*blocks)[i]->getStartCode();
            while (start != nullptr) {
                IrCode* code = start->getCode();
                if (code -> getCodeType() == IrGotoStmtType) {
                    auto* line = (IrGotoStmt*)code;
                    int target = belong[labelMp[line->getLabel()]];
                    graph -> link(i, target);
                } else if (code -> getCodeType() == IrBranchStmtType) {
                    auto* line = (IrBranchStmt*)code;
                    int target = belong[labelMp[line->getLabel()]];
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
                    code -> getCodeType() != IrExitType)
                    graph->link(i, i + 1);
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
            if (it -> second == 1)
                funNames.erase(it++);
            else
                it++;
        }

        /*
        for (auto it = funNames.begin(); it != funNames.end(); it++) {
            cout << "var: " << (*it).first << endl;
        }
         */

        /*
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

        //SSA Re Name
        for (int i = 0; i < N; ++ i)
            (*blocks)[i]->ssaReName();

         */
        //Remove Phi
        //todo: how to remove the phi points
        /*
        for (int i = 0; i < N; ++ i) {
            vector<int>* out = graph -> getOutBlock(i);
            for (auto c: *out) {
                auto* phiList = (*blocks)[i] -> getPhiList();
                for (auto t: *phiList) {

                }
            }
        }
        */
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
        newIR ->push_back((*codes)[0]);
        for (auto block: *blocks) {
            vector<IrCode*>* codes = block -> toIR();
            for (auto code: *codes)
                newIR -> push_back(code);
        }
        newIR->push_back(new IrFunEnd(0));
        return newIR;
    }
};
#endif //COMPILER_IR_FUN_H
