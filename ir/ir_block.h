//
// Created by chenjiyuan3 on 2021/12/3.
//

#ifndef COMPILER_IR_BLOCK_H
#define COMPILER_IR_BLOCK_H

extern map<string, int> globalNameCount;

class IrBlock {
private:
    vector<IrCode*>* codes;
    set<string>* names;
    MyList* fStmt, *eStmt;
    map<string, string>* finalNames;
    map<string, MyList*>* paiList;

    set<string> *def, *use, *in, *out;

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
        if (eStmt == nullptr) {
            eStmt = fStmt = list;
        } else {
            if (eStmt -> getCode() ->getCodeType() == IrBranchStmtType ||
                eStmt -> getCode() ->getCodeType() == IrGotoStmtType ||
                eStmt -> getCode() ->getCodeType() == IrExitType ||
                eStmt -> getCode() ->getCodeType() == IrReturnStmtType) {
                MyList* prev = eStmt -> getPrev();
                list -> linkNext(eStmt);
                if (prev == nullptr) {
                    fStmt = list;
                } else {
                    prev -> linkNext(list);
                }
            } else {
                eStmt -> linkNext(list);
                eStmt = list;
            }
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
            MyList* start = fStmt;
            while (start->getCode()->getCodeType() == IrLabelLineType) {
                if (start -> getNext() == nullptr)
                    break;
                start = start -> getNext();
            }
            MyList* prev = start -> getPrev();
            list -> linkNext(start);
            if (prev != nullptr) {
                prev -> linkNext(list);
            } else
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
                if (sc.empty() || sc == "%0" || sc[0] != '%') continue;
                if (finalNames -> find(sc) == finalNames -> end()) {
                    cout << "error in IR, can't find : " << sc << endl;
                    exit(7654321);
                }
                string newName = (*finalNames)[sc];
                code->setSource(i, newName);
            }
            string target = code->getTarget();
            if (!target.empty() && target[0] != '@' && target[0] != '$') {
                if (globalNameCount.find(target) == globalNameCount.end())
                    globalNameCount[target] = 1;
                else
                    globalNameCount[target] = globalNameCount[target] + 1;
                int cnt = globalNameCount[target];
                string newName = target + "_" + to_string(cnt);
                code->setTarget(newName);
                (*finalNames)[target] = newName;
            }
            start = start->getNext();
        }
    }

    map<string, MyList*>* getPhiList() {
        return paiList;
    }

    map<string, string>* getFinalNames() {
        return finalNames;
    }

    void putNameIntoFinalNames(const string& key, const string& value) {
        (*finalNames)[key] = value;
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

    void remove(MyList* code) {
        if (code == fStmt && code == eStmt) {
            fStmt = eStmt = nullptr;
        } else if (code == fStmt) {
            fStmt = code -> getNext();
            fStmt -> setPrev(nullptr);
        } else if (code == eStmt) {
            eStmt = code -> getPrev();
            eStmt -> setNext(nullptr);
        } else {
            MyList* prev = code -> getPrev(), *next = code -> getNext();
            prev -> linkNext(next);
        }
    }

    vector<IrCode*>* removePhiAssign() {
        auto* ir = new vector<IrCode*>();
        if (eStmt == nullptr)
            return ir;
        MyList* end = eStmt;
        bool change = false;
        if (eStmt -> getCode() ->getCodeType() == IrBranchStmtType ||
            eStmt -> getCode() ->getCodeType() == IrGotoStmtType ||
            eStmt -> getCode() ->getCodeType() == IrExitType ||
            eStmt -> getCode() ->getCodeType() == IrReturnStmtType)
            end = end -> getPrev();
        else
            change = true;
        while (end != nullptr && end -> getCode() -> getCodeType() == IrPhiAssignType) {
            ir -> push_back(end -> getCode());
            end = end -> removeToPrev();
        }
        if (change) eStmt = end;
        return ir;
    }

    int getBlockNum() const {
        return blockNum;
    }

    void calcDef() {
        def = new set<string>();
        MyList* start = fStmt;
        while (start != nullptr) {
            IrCode* code = start -> getCode();
            if (code -> getCodeType() != IrPhiType) {
                string target = code -> getTarget();
                if (!target.empty() && target[0] == '%')
                    def ->insert(target);
            } else {
                //cout << "error, why you find phi point ?" << endl;
                //exit(343434);
            }
            start = start -> getNext();
        }
    }

    void calcUse() {
        use = new set<string>();
        MyList* start = fStmt;
        while (start != nullptr) {
            IrCode* code = start -> getCode();
            if (code -> getCodeType() != IrPhiType) {
                for (int i = 0; i < 2; ++ i) {
                    string source = code -> getSource(i);
                    if (!source.empty() && source != "%0" && source[0] == '%')
                        use -> insert(source);
                }
            } else {
                //cout << "error, why you find phi point ?" << endl;
                //exit(434343);
            }
            start = start -> getNext();
        }
    }

    void dataFlowInit() {
        in = new set<string>;
        out = new set<string>;
    }

    set<string>* getDef() {
        return def;
    }

    set<string>* getUse() {
        return use;
    }

    set<string>* getIn() {
        return in;
    }

    set<string>* getOut() {
        return out;
    }

    void putVarIntoIn(const string& var) {
        in -> insert(var);
    }

    void putVarIntoOut(const string& var) {
        out -> insert(var);
    }
};

#endif //COMPILER_IR_BLOCK_H
