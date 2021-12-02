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
#include "../ir/ir.h"
#include "../ir/ir_code.h"
#include "../ir/ir_table.h"

using namespace std;
extern Output output;

class FunRParam: public Node {
public:
    FunRParam() {
        classType = FunRParamType;
    }
    Node* optimize() override {
        return this;
    }
};

class FunFParam: public Node {
private:
    string name;
    int offset; //[][offset]
    Node* offsetTree;
public:
    FunFParam(string _name, Node* _offsetTree, int _type, int _line) {
        name = std::move(_name);
        if (_offsetTree == nullptr) {
            offset = 0;
            offsetTree = new Number(0);
        } else {
            offsetTree = _offsetTree->optimize();
            offset = ((ConstValue*)offsetTree)->getValue();
        }
        type = _type;
        classType = FunFParamType;
        line = _line;
    }
    void check() override {
        cout << "FunFParam check correct!" << endl;
    }
    void traversal() override {
        string irName = irTableList_1.addVar(name);
        if (type == 0)
            IR_1.add(new IrParaDefine("var", irName));
        else
            IR_1.add(new IrParaDefine("arr", irName));
        //cout << "parameter: " << name << " " << offset << endl;
    }
    string getName() const {
        return name;
    }
    Node* optimize() override {
        return this;
    }
    Node* getOffset() {
        return offsetTree;
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
        IR_1.add(new IrFunDefine(type == -1 ? "void": "int", to_string((*param).size()), name));
        IR_1.add(new IrLabelLine(name));
        irTableList_1.setBlock(1);
        for (int i = (int)((*param).size()) - 1 ; i >= 0; -- i)
            (*param)[i] -> traversal();
        irTableList_1.setBlock(-1);
        IR_1.add(new IrParaDefine("reg", "$ra"));
        funBlock->traversal();
        IR_1.add(new IrFunEnd((*param).size()));
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
    Node* optimize() override {
        return this;
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
        vector<IrCode*> pushCode;
        for (int i = param->size() - 1; i >= 0; -- i) {
            Node* rp = (*param)[i];
            if (rp -> getClassType() == FunRType) {
                rp -> traversal();
                pushCode.push_back(new IrPushVariable(irTableList_1.getTopTemIrName()));
            } else {
                if (rp -> getClassType() == VariableType && ((Variable*)rp) -> getIsArray()) {
                    Variable* var = (Variable*)rp;
                    string irName = irTableList_1.getIrName(var -> getName());
                    Node* offset = var -> getOffsetTree();
                    if (offset -> getClassType() == VariableType && offset -> getSize() == 1) {
                        string irNameOff = irTableList_1.getIrName(((Variable*)offset) -> getName());
                        if (rp -> getType() > 0)
                            pushCode.push_back(new IrPushArray(irName, irNameOff));
                        else {
                            string tem = irTableList_1.allocTem();
                            IR_1.add(new IrArrayGet(tem, irName, irNameOff));
                            pushCode.push_back(new IrPushVariable(tem));
                        }
                    } else {
                        offset -> traversal();
                        string last = irTableList_1.getTopTemIrName();
                        if (rp -> getType() > 0)
                            pushCode.push_back(new IrPushArray(irName, last));
                        else {
                            string tem = irTableList_1.allocTem();
                            IR_1.add(new IrArrayGet(tem, irName, last));
                            pushCode.push_back(new IrPushVariable(tem));
                        }
                    }
                } else if (rp -> getClassType() == VariableType) {
                    Variable* var = (Variable*)rp;
                    string irName = irTableList_1.getIrName(var -> getName());
                    pushCode.push_back(new IrPushVariable(irName));
                } else {
                    rp -> traversal();
                    pushCode.push_back(new IrPushVariable(irTableList_1.getTopTemIrName()));
                }
            }
        }
        for (auto code: pushCode)
            IR_1.add(code);
        IR_1.add(new IrCallFunction(name, param->size()));
        if (fun -> getType() != -1) {
            string tem = irTableList_1.allocTem();
            IR_1.add(new IrReturnValStmt(tem));
        }
        /*cout << "function: " << name << endl;
        for (auto & para : param)
            para.traversal();*/
        //param.traversal();
    }
    string getName() {
        return name;
    }
    Node* optimize() override {
        return this;
    }
};

#endif //COMPILER_FUNCTION_H
