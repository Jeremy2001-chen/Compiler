//
// Created by chenjiyuan3 on 2021/10/21.
//

#ifndef COMPILER_COMP_UNIT_H
#define COMPILER_COMP_UNIT_H

#include "node.h"
#include "value.h"
#include "function.h"

class CompUnit: public Node {
private:
    vector<VariableDecl*> declList;
    vector<FunF*> funList;
    Block* mainBlock;
public:
    CompUnit() = default;
    void setVar(VariableDecl* var) {
        declList.push_back(var);
    }
    void setFun(FunF* fun) {
        funList.push_back(fun);
    }
    void setMainBlock(Block* main) {
        mainBlock = main;
    }
    void check() override {

    }
    void traversal() override {

    }
};
#endif //COMPILER_COMP_UNIT_H
