//
// Created by chenjiyuan3 on 2021/10/21.
//

#ifndef COMPILER_COMP_UNIT_H
#define COMPILER_COMP_UNIT_H

#include "node.h"
#include "value.h"
#include "function.h"
#include "../ir/ir.h"
#include "../ir/ir_code.h"
#include "../ir/ir_table.h"

extern IR IR_1;
extern IR IR_1;
extern IrTableList irTableList_1;
class CompUnit: public Node {
private:
    vector<VariableDecl*> declList;
    vector<FunF*> funList;
    FunF* mainFun;
public:
    CompUnit() {
        classType = CompUnitType;
    }
    void setVar(VariableDecl* var) {
        declList.push_back(var);
    }
    void setFun(FunF* fun) {
        funList.push_back(fun);
    }
    void setMainBlock(FunF* main) {
        mainFun = main;
    }
    void check() override {

    }
    void traversal() override {
        for (auto var: declList) {
            var->traversal();
        }
        IR_1.add(new IrGotoStmt("main"));
        for (auto fun: funList) {
            fun->traversal();
        }
        mainFun->traversal();
    }
    Node* optimize() override {
        return this;
    }
};
#endif //COMPILER_COMP_UNIT_H
