//
// Created by chenjiyuan3 on 2021/12/3.
//

#ifndef COMPILER_GRAPH_SSA_H
#define COMPILER_GRAPH_SSA_H

#include "../ir/ir_code.h"
#include "../ir/ir_table.h"

extern IrTableList irTableList_1;

class GraphSSA {
private:
    map<string, int> nameMap;
    vector<string> names;
    int cnt = 0;
    vector<IrPhiAssign*>* codes;
    vector<int> out;
    vector<int> degree;
public:
    GraphSSA(vector<IrCode*>* list) {
        codes = new vector<IrPhiAssign*>();
        for (auto code: *list) {
            cout << code->toString() << endl;
            IrPhiAssign* assign = (IrPhiAssign*)code;
            codes -> push_back(assign);
        }
        for (auto code: *codes) {
            string name = code->getSource(0);
            if (nameMap.find(name) == nameMap.end()) {
                names.push_back(name);
                nameMap[name] = cnt ++;
            }
            name = code -> getTarget();
            if (nameMap.find(name) == nameMap.end()) {
                names.push_back(name);
                nameMap[name] = cnt ++;
            }
        }
        out.resize(cnt);
        for (int i = 0; i < cnt; ++ i)
            out[i] = -1;
        degree.resize(cnt);
        for (auto code: *codes) {
            string source = code -> getSource(0), target = code -> getTarget();
            link(nameMap[target], nameMap[source]);
        }
    }

    void link(int s, int t) {
        cout << "small link : " << s << " " << t << endl;
        out[s] = t;
        degree[t] ++;
    }

    vector<IrCode*>* getNewCode() {
        vector<IrCode*>* ret = new vector<IrCode*>();
        vector<IrCode*>* temp = new vector<IrCode*>();
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
                ret -> push_back(new IrMove(names[now], names[to], 0));
                degree[to] --;
                if (degree[to] == 0)
                    Q.push(to);
            }
            if (leftPoint.empty()) break;
            auto it = leftPoint.begin();
            string tem = irTableList_1.allocTemForSSA("_0");
            int to = out[*it];
            ret -> push_back(new IrMove(tem, names[to], 1));
            temp -> push_back(new IrMove(names[*it], tem, 0));
            degree[to] --;
            if (degree[to] == 0)
                Q.push(to);
        }
        for (auto c: *temp) {
            ret -> push_back(c);
        }
        return ret;
    }
};
#endif //COMPILER_GRAPH_SSA_H
