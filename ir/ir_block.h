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

#endif //COMPILER_IR_BLOCK_H
