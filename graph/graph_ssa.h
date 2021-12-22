//
// Created by chenjiyuan3 on 2021/12/3.
//

#ifndef COMPILER_GRAPH_SSA_H
#define COMPILER_GRAPH_SSA_H

#include "../ir/ir_code.h"
#include "../ir/ir_table.h"

extern IrTableList irTableList_1;
extern map<string, string>* varToRegister;

typedef pair<string, string> pss;

class GraphSSA {
private:
    map<string, int> regMap;
    vector<string> regs;
    map<pss, pss> regToVar;
    int cnt = 0;
    vector<IrPhiAssign*>* codes;
    vector<int> out;
    vector<int> degree;
    map<string, string> varToReg;
public:
    GraphSSA(vector<IrCode*>* list) {
        codes = new vector<IrPhiAssign*>();
        for (auto code: *list) {
            assert(code -> getCodeType() == IrPhiAssignType);
//            cout << "before : " << code ->toString() << endl;
            IrPhiAssign* assign = (IrPhiAssign*)code;
            codes -> push_back(assign);
        }
        int x = 0;
        for (auto code: *codes) {
            string nameS = code->getSource(0);
            string regS;
            if (varToReg.find(nameS) == varToReg.end()) {

                if (varToRegister -> find(nameS) == varToRegister -> end()) {
                    x ++;
                    regS = "X" + to_string(x);
                    varToReg[nameS] = regS;
                } else {
                    regS = (*varToRegister)[nameS];
                    //assert(regToVar.find(reg) == regToVar.end());
                    varToReg[nameS] = regS;
                }
                regs.push_back(regS);
//                cout << "local : " << nameS << " " << regS << endl;
                regMap[regS] = cnt ++;
            }
            string nameT, regT;
            nameT = code -> getTarget();
            if (varToReg.find(nameT) == varToReg.end()) {
//                string reg;
                if (varToRegister -> find(nameT) == varToRegister -> end()) {
                    x ++;
                    regT = "X" + to_string(x);
                    varToReg[nameT] = regT;
                } else {
                    regT = (*varToRegister)[nameT];
                    //assert(regToVar.find(reg) == regToVar.end());
                    varToReg[nameT] = regT;
                }
                regs.push_back(regT);
//                cout << "local : " << nameT << " " << regT << endl;
                regMap[regT] = cnt ++;
            }
            regToVar[make_pair(regS, regT)] = make_pair(nameS, nameT);
        }
        out.resize(cnt);
        for (int i = 0; i < cnt; ++ i)
            out[i] = -1;
        degree.resize(cnt);
        for (auto code: *codes) {
            string source = code -> getSource(0), target = code -> getTarget();
            link(regMap[varToReg[target]], regMap[varToReg[source]]);
        }
    }

    void link(int s, int t) {
//        cout << "ssa link : " << regs[s] << " " << regs[t] << endl;
        out[s] = t;
        degree[t] ++;
    }

    vector<IrCode*>* getNewCode() {
        vector<IrCode*>* ret = new vector<IrCode*>();
        IrMove* temp = nullptr;
        set <int> leftPoint;
        queue <int> Q;
        for (int i = 0; i < cnt; ++ i) {
            leftPoint.insert(i);
            if (degree[i] == 0)
                Q.push(i);
        }
        while (!leftPoint.empty()) {
            while (!Q.empty()) {
                int now = Q.front(); Q.pop();
                leftPoint.erase(now);
                int to = out[now];
                if (to == -1)
                    continue;
                pss var = regToVar[make_pair(regs[to], regs[now])];
                ret -> push_back(new IrMove(var.second, var.first, false));
                degree[to] --;
                if (degree[to] == 0)
                    Q.push(to);
            }
            if (temp != nullptr)
                ret -> push_back(temp);
            if (leftPoint.empty()) break;
            auto it = leftPoint.begin();
            string tem = irTableList_1.allocTemForSSA("_0");
            int to = out[*it];
            if (to == -1) continue;
            pss var = regToVar[make_pair(regs[to], regs[*it])];
            ret -> push_back(new IrBinaryOp(tem, var.first, "+", "%0"));
            temp = new IrMove(var.second, tem, false);
            (*varToRegister)[tem] = "$t8";
            out[*it] = -1;
            degree[to] --;
            if (degree[to] == 0)
                Q.push(to);
        }
        return ret;
    }
};
#endif //COMPILER_GRAPH_SSA_H
