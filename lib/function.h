//
// Created by chenjiyuan3 on 2021/10/20.
//

#ifndef COMPILER_FUNCTION_H
#define COMPILER_FUNCTION_H

#include "node.h"
#include <cassert>
#include "../error.h"

using namespace std;
extern Output output;

class FunRParam: public Node {
};

class FunFParam: public Node {
private:
    string name;
    int offset;
public:
    FunFParam(string _name, int _offset, int _type) {
        name = _name;
        offset = _offset;
        type = _type;
    }
    void check() override {
        //assert(name && funBlock);
        cout << "FunFParam check correct!" << endl;
    }
    void traversal() override {
        cout << "parameter: " << name << " " << offset << endl;
    }
};

class FunF: public Node {
private:
    vector<FunFParam> param;
    string name;
    Block funBlock;
public:
    FunF() = default;
    FunF(string _name, vector<FunFParam> _para, Block _block, int _returnType) {
        name = _name;
        param = _para;
        funBlock = _block;
        type = _returnType;
    }
    void setFunBlock(Block _block) {
        funBlock = _block;
    }
    string getName() {
        return name;
    }
    void check() override {
        //assert(name && funBlock);
        funBlock.check();
        cout << "FunF check correct!" << endl;
    }
    void traversal() override {
        cout << "Function : " << name << endl;
        for (auto para: param) {
            para.traversal();
        }
        funBlock.traversal();
    }
    string typeChange(int _type) {
        if (_type == -1)
            return "void";
        else if (_type == 0)
            return "int";
        else if (_type == 1)
            return "int[]";
        else if (_type == 2)
            return "int[][]";
    }
    void checkRParam(vector<FunRParam> rParam, int line) {
        if (param.size() != rParam.size())
            output.addError(NotMatchParameterNumError(line, name, param.size(), rParam.size()));
        for (int i = 0; i < param.size(); ++i)
            if (param[i].getType() != rParam[i].getType()) {
                output.addError(NotMatchParameterTypeError(line, name, typeChange(param[i].getType()), typeChange(rParam[i].getType())));
            }
    }
};

class FunR: public Node {
private:
    FunF fun;
    vector<Node> param;
    string name;
public:
    FunR(string _name, FunF _func, vector<Node> _para) {
        name = std::move(_name);
        fun = std::move(_func);
        param = std::move(_para);
    }
    void check() override {
        //assert(fun && param);
        assert(name == fun.getName());
        cout << "FunR check correct!" << endl;
        //fun.checkRParam(param);
    }
    void traversal() override {
        cout << "function: " << name << endl;
        for (auto & para : param)
            para.traversal();
        //param.traversal();
    }
    string getName() {
        return name;
    }
};

#endif //COMPILER_FUNCTION_H
