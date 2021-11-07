//
// Created by chenjiyuan3 on 2021/10/20.
//

#ifndef COMPILER_BLOCK_H
#define COMPILER_BLOCK_H

#include "node.h"
#include <utility>
#include <vector>
#include <cstring>
#include "../error.h"
#include "../output.h"
#include "../type.h"
#include "../ir/ir.h"
#include "../ir/ir_code.h"
#include "../ir/ir_table.h"

using namespace std;

extern Output output;

extern IR IR_1;
extern IrTableList irTableList_1;

class Block: public Node {
private:
    vector<Node*> blockItem;
public:
    Block() {
        classType = BlockType;
    }
    void addBlockItem(Node* node) {
        blockItem.push_back(node);
    }
    vector<Node*> getBlockItem() {
        return blockItem;
    }
    Node* getLastItem() {
        return blockItem[(int)blockItem.size()-1];
    }
    void check() override {
        //assert(blockItem);
        /*for (auto &blockIt: blockItem) {
            blockIt.check();
        }
        cout << "Block check correct!" << endl;*/
    }
    void traversal() override {
        irTableList_1.intoBlock();
        for (auto &blockIt: blockItem) {
            blockIt->traversal();
        }
        irTableList_1.popBlock();
    }
    Node* optimize() override {
        return this;
    }
};

class IfStmt: public Node {
private:
    vector<Node*> cond;
    vector<Node*> tran;
public:
    IfStmt() {
        classType = IfStmtType;
    }
    void addTran(Node* _cond, Node* _tran) {
        cond.push_back(_cond);
        tran.push_back(_tran);
    }
    void addElseTran(Node* _tran) {
        tran.push_back(_tran);
    }
    void check() override {

    }
    void traversal() override {
        cond[0]->traversal();
        string con = irTableList_1.getTopTemIrName();;
        string br1 = irTableList_1.allocBranch(), br2 = irTableList_1.allocBranch();
        IR_1.add(new IrCmpStmt(con, "%0"));
        IR_1.add(new IrBranchStmt("beq", br2));
        tran[0]->traversal();
        IR_1.add(new IrGotoStmt(br2));
        IR_1.add(new IrLabelLine(br1));
        if ((int)tran.size() > 1)
            tran[1]->traversal();
        IR_1.add(new IrLabelLine(br2));
    }
    Node* optimize() override {
        return this;
    }
};

class WhileStmt: public Node {
private:
    Node* cond;
    Node* block;
public:
    WhileStmt(Node* _cond, Node* _block) {
        cond = _cond;
        block = _block;
        classType = WhileStmtType;
    }
    void check() override {

    }
    void traversal() override {
        string loop = irTableList_1.allocLoop(), loop_begin = loop + "_begin", loop_end = loop + "_end";
        IR_1.add(new IrLabelLine(loop_begin));
        cond -> traversal();
        string con = irTableList_1.getTopTemIrName();;
        IR_1.add(new IrCmpStmt(con, "%0"));
        IR_1.add(new IrBranchStmt("beq", loop_end));
        block -> traversal();
        IR_1.add(new IrLabelLine(loop_end));
        irTableList_1.popLoop();
    }
    Node* optimize() override {
        return this;
    }
};

class BreakStmt: public Node {
public:
    BreakStmt() {
        classType = BreakStmtType;
    }
    void check() override {

    }
    void traversal() override {
        string loop = irTableList_1.getTopLoop();
        IR_1.add(new IrGotoStmt(loop + "_end"));
    }
    Node* optimize() override {
        return this;
    }
};

class ContinueStmt: public Node {
public:
    ContinueStmt() {
        classType = ContinueStmtType;
    }
    void check() override {

    }
    void traversal() override {
        string loop = irTableList_1.getTopLoop();
        IR_1.add(new IrGotoStmt(loop + "_begin"));
    }
    Node* optimize() override {
        return this;
    }
};

class ReturnStmt: public Node {
private:
    Node* returnExp;
public:
    ReturnStmt() {
        returnExp = nullptr;
        type = -1;
        classType = ReturnStmtType;
    }
    ReturnStmt(Node* exp) {
        returnExp = exp;
        type = exp->getType();
        classType = ReturnStmtType;
    }
    void check() override {

    }
    void traversal() override {
        if (returnExp == nullptr) {
            IR_1.add(new IrReturnStmt());
        } else {
            returnExp->traversal();
            IR_1.add(new IrReturnStmt(irTableList_1.getTopTemIrName()));
        }
    }
    Node* optimize() override {
        return this;
    }
};

class PrintfStmt: public Node {
private:
    string format;
    vector<string> form;
    vector<Node*>* exp;
public:
    PrintfStmt(string _format, vector<Node*>* _exp, int _printfLine) {
        format = std::move(_format);
        exp = _exp;
        line = _printfLine;
        classType = PrintfStmtType;
        getForm();
    }
    int expectNum() const {
        return (int)form.size() - 1;
    }
    int realNum() const {
        return (int)exp -> size();
    }
    void getForm() {
        string tmp;
        int len = (int)format.size();
        for (int i = 1; i < len - 1; i++) {
            if (i < len - 2 && format[i] == '%' && format[i+1] == 'd') {
                form.push_back(tmp);
                tmp = "";
            } else {
                tmp += format[i];
            }
        }
        form.push_back(tmp);
    }

    void check() override {

    }
    void traversal() override {
        for (int i = 0; i < (int)form.size(); ++ i) {
            if (form[i] != "") {
                IR_1.add(new IrPrintString(form[i]));
            }
            if (i < (int)(*exp).size()) {
                (*exp)[i]->traversal();
                IR_1.add(new IrPrintInteger(to_string(irTableList_1.getTemNumber())));
            }
        }
    }
    Node* optimize() override {
        return this;
    }
};

class NullStmt: public Node {
public:
    NullStmt() {
        classType = NullStmtType;
    }
    void check() override {

    }
    void traversal() override {

    }
    Node* optimize() override {
        return this;
    }
};
#endif //COMPILER_BLOCK_H
