//
// Created by chenjiyuan3 on 2021/10/20.
//

#ifndef COMPILER_FUNCTION_H
#define COMPILER_FUNCTION_H

#include "node.h"
#include <cassert>
#include <utility>
#include "../error.h"
#include "binary_exp.h"

using namespace std;
extern Output output;

class FunRParam: public Node {
public:
    FunRParam() {
        classType = FunRParamType;
    }
};

class FunFParam: public Node {
private:
    string name;
    int offset;
    Node* offsetTree;
public:
    FunFParam(string _name, Node* _offsetTree, int _type, int _line) {
        name = std::move(_name);
        offsetTree = _offsetTree;
        //offset = ?
        type = _type;
        classType = FunFParamType;
        line = _line;
    }
    void check() override {
        cout << "FunFParam check correct!" << endl;
    }
    void traversal() override {
        //cout << "parameter: " << name << " " << offset << endl;
    }
    string getName() const {
        return name;
    }
};

class FunF: public Node {
private:
    vector<FunFParam*>* param;
    string name;
    Block* funBlock;
public:
    FunF() {
        classType = FunFType;
    }
    FunF(string _name, vector<FunFParam*>* _para, int _returnType) {
        name = std::move(_name);
        param = _para;
        type = _returnType;
        classType = FunFType;
    }
    FunF(string _name, vector<FunFParam*>* _para, Block* _block, int _returnType) {
        name = std::move(_name);
        param = _para;
        funBlock = _block;
        type = _returnType;
        classType = FunFType;
    }
    void setFunBlock(Block* _block) {
        funBlock = _block;
    }
    string getName() {
        return name;
    }
    void setBlock(Block* block) {
        funBlock = block;
    }
    void check() override {
        //assert(name && funBlock);
        //funBlock.check();
        cout << "FunF check correct!" << endl;
    }
    void traversal() override {
        /*cout << "Function : " << name << endl;
        for (auto para: param) {
            para.traversal();
        }
        funBlock.traversal();*/
    }
    static string typeChange(int _type) {
        if (_type == -1)
            return "void";
        else if (_type == 0)
            return "int";
        else if (_type == 1)
            return "int[]";
        else if (_type == 2)
            return "int[][]";
        return "";
    }
    void checkRParam(vector<Node*>* rParam, int line) {
        if (param->size() != rParam->size())
            output.addError(new NotMatchParameterNumError(line, name,
                                                      (int)param->size(), (int)rParam->size()));
        else {
            for (int i = 0; i < param->size(); ++i)
                if ((*param)[i]->getType() != (*rParam)[i]->getType()) {
                    output.addError(new NotMatchParameterTypeError(line, name,
                                                                   typeChange((*param)[i]->getType()),
                                                                   typeChange((*rParam)[i]->getType())));
                }
        }
    }
};

class FunR: public Node {
private:
    FunF* fun;
    vector<Node*>* param;
    string name;
public:
    FunR(string _name, FunF* _func, vector<Node*>* _para) {
        name = std::move(_name);
        fun = _func;
        param = _para;
        classType = FunRType;
        type = _func == nullptr ? 0 : _func->getType();
    }
    void check() override {
        //assert(fun && param);
        assert(name == fun->getName());
        cout << "FunR check correct!" << endl;
        //fun.checkRParam(param);
    }
    void traversal() override {
        /*cout << "function: " << name << endl;
        for (auto & para : param)
            para.traversal();*/
        //param.traversal();
    }
    string getName() {
        return name;
    }
};

#endif //COMPILER_FUNCTION_H
