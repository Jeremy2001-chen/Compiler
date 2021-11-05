//
// Created by chenjiyuan3 on 2021/10/5.
//

#ifndef GRAMMAR_1005_GRAMMAR_H
#define GRAMMAR_1005_GRAMMAR_H

#include "output.h"
#include "lib/node.h"
#include "lib/single_exp.h"
#include "lib/value.h"
#include "lib/block.h"
#include "lib/function.h"
#include "lib/binary_exp.h"
#include "lib/comp_unit.h"
#include "table.h"
#include "error.h"

extern Output output;

class Grammar{
private:
    Lexical lexical;
    Word currentWord;
    SymbolTable symbolTable;
    int whileCnt = 0;
    int wordIndex = 0, totalWord;

    int getPrevLine() {
        if (wordIndex == 0)
            return 0;
        return lexical.getWord(wordIndex-1).getLine();
    }

    void addLine(const string& str) const {
        output.addLine(str, wordIndex - 1);
    }

    void move() {
        currentWord.pushToOutput(wordIndex);
        wordIndex += 1;
        currentWord = lexical.getWord(wordIndex);
    }

    void setIndex(int id) {
        output.setIndex(id);
        wordIndex = id;
        currentWord = lexical.getWord(wordIndex);
    }

    CompUnit* checkCompUnit(){
        CompUnit* compUnit = new CompUnit();
        int startIndex = wordIndex;
        while (startIndex < totalWord) {
            DeclStmt* variableDeclList = checkDecl();
            if (variableDeclList != nullptr) {
                for (auto& it: (*variableDeclList->getDecl()))
                    compUnit->setVar(it);
                startIndex = wordIndex;
            } else {
                setIndex(startIndex);
                break;
            }
        }

        while (startIndex < totalWord) {
            FunF* funF = checkFuncDef();
            if (funF != nullptr) {
                compUnit->setFun(funF);
                startIndex = wordIndex;
            } else {
                setIndex(startIndex);
                break;
            }
        }

        FunF* mainFunction = checkMainFuncDef();
        if (mainFunction == nullptr) {
            free(compUnit);
            return nullptr;
        }

        compUnit->setMainBlock(mainFunction);
        addLine("<CompUnit>");
        return compUnit;
    }

    DeclStmt* checkDecl() {
        DeclStmt* variableDeclList;
        int startIndex = wordIndex;
        variableDeclList = checkConstDecl();
        if (variableDeclList != nullptr)
            return variableDeclList;

        setIndex(startIndex);
        variableDeclList = checkVarDecl();
        if (variableDeclList != nullptr)
            return variableDeclList;

        return nullptr;
    }

    DeclStmt* checkConstDecl() {
        if (!currentWord.checkType("CONSTTK")) //'const'
            return nullptr;

        move();
        if (!checkBType())
            return nullptr;

        VariableDecl* variableDecl = checkConstDef();
        if (variableDecl == nullptr)
            return nullptr;

        DeclStmt* variableDeclList = new DeclStmt();
        variableDeclList->addDecl(variableDecl);
        symbolTable.insertVarTable(variableDecl->getName(), variableDecl,
                                   variableDecl->getConstType(), variableDecl->getLine());

        int startIndex = wordIndex;
        while (startIndex < totalWord) {
            if (!currentWord.checkType("COMMA")) { //','
                break;
            }

            move();
            variableDecl = checkConstDef();
            if (variableDecl == nullptr) {
                setIndex(startIndex);
                break;
            }
            variableDeclList->addDecl(variableDecl);
            symbolTable.insertVarTable(variableDecl->getName(), variableDecl,
                                       variableDecl->getConstType(), variableDecl->getLine());
            startIndex = wordIndex;
        }

        if (!currentWord.checkType("SEMICN")) { //';'
            output.addError(new NoSemicolonError(getPrevLine()));
        } else {
            move();
        }

        addLine("<ConstDecl>");
        return variableDeclList;
    }

    bool checkBType() {
        if (!currentWord.checkType("INTTK")) //'int'
            return false;

        move();
        return true;
    }

    VariableDecl* checkConstDef() {
        VariableDecl* variableDecl;
        int currentLine = currentWord.getLine();
        string name = checkIdent();
        if (name.empty())
            return nullptr;

        int startIndex = wordIndex;
        Node* addExp;
        vector<Node*>* offsetList = new vector<Node*>();
        int type = 0;
        while (startIndex < totalWord) {
            if (!currentWord.checkType("LBRACK")) //'['
                break;

            move();
            addExp = checkConstExp();
            if (addExp == nullptr) {
                setIndex(startIndex);
                break;
            }
            offsetList->push_back(addExp);
            if (!currentWord.checkType("RBRACK")) { //']'
                //setIndex(startIndex);
                //break;
                output.addError(new NoRightBracketsError(getPrevLine())); //can output first? I think yes
            } else {
                move();
            }
            type += 1;
            startIndex = wordIndex;
        }

        if (!currentWord.checkType("ASSIGN")) { //'=
            free(offsetList);
            return nullptr;
        }

        move();
        vector<Node*>* valueList = checkConstInitVal();
        if (valueList == nullptr) {
            free(offsetList);
            return nullptr;
        }

        Node* offsetTree = (int)offsetList->size() == 0 ? nullptr : (*offsetList)[(int)offsetList->size()-1];
        variableDecl = new VariableDecl(name, offsetTree,
                                        valueList, type, true, currentLine);

        addLine("<ConstDef>");
        return variableDecl;
    }

    vector<Node*>* checkConstInitVal() {
        vector<Node*>* list = new vector<Node*>();
        int startIndex = wordIndex;
        Node* exp = checkConstExp();
        if (exp != nullptr) {
            list->push_back(exp);
            cout << "const var traversal: " << endl;
            exp->traversal();
            addLine("<ConstInitVal>");
            return list;
        }

        setIndex(startIndex);
        if (!currentWord.checkType("LBRACE")) { //'{'
            free(list);
            return nullptr;
        }

        move();
        vector<Node*>* initVal = checkConstInitVal();
        if (initVal != nullptr)  {
            startIndex = wordIndex;
            for (auto &val: (*initVal)) {
                list->push_back(val);
            }
            while (startIndex < totalWord) { //','
                if (!currentWord.checkType("COMMA")) { //','
                    break;
                }
                move();
                initVal = checkConstInitVal();
                if (initVal == nullptr) {
                    setIndex(startIndex);
                    break;
                }
                for (auto &val: (*initVal)) {
                    list->push_back(val);
                }
                startIndex = wordIndex;
            }
        }

        if (!currentWord.checkType("RBRACE")) { //'}'
            free(list);
            free(initVal);
            return nullptr;
        }

        move();
        addLine("<ConstInitVal>");
        return list;
    }

    DeclStmt* checkVarDecl() {
        if (!checkBType()) {
            return nullptr;
        }
        VariableDecl* variableDecl = checkVarDef();
        if (variableDecl == nullptr) {
            return nullptr;
        }

        DeclStmt* varList = new DeclStmt();
        varList->addDecl(variableDecl);
        symbolTable.insertVarTable(variableDecl->getName(), variableDecl,
                                   variableDecl->getConstType(), variableDecl->getLine());
        int startIndex = wordIndex;
        while (startIndex < totalWord) {
            if (!currentWord.checkType("COMMA")) { //','
                break;
            }

            move();
            variableDecl = checkVarDef();
            if (variableDecl == nullptr) {
                setIndex(startIndex);
                break;
            }
            startIndex = wordIndex;
            varList->addDecl(variableDecl);
            symbolTable.insertVarTable(variableDecl->getName(), variableDecl,
                                       variableDecl->getConstType(), variableDecl->getLine());
        }

        if (!currentWord.checkType("SEMICN")) { //';'
            output.addError(new NoSemicolonError(getPrevLine()));
        } else {
            move();
        }

        addLine("<VarDecl>");
        return varList;
    }

    VariableDecl* checkVarDef() {
        int currentLine = currentWord.getLine();
        VariableDecl* variableDecl;
        string name = checkIdent();
        if (name.empty()) {
            return nullptr;
        }

        if (currentWord.getValue() == "(") { // should not be function
            return nullptr;
        }
        int startIndex = wordIndex;
        Node* addExp = nullptr;
        vector<Node*>* offsetList = new vector<Node*>();
        int type = 0;
        while (startIndex < totalWord) {
            if (!currentWord.checkType("LBRACK")) { //'['
                break;
            }
            move();
            addExp = checkConstExp();
            if (addExp == nullptr) {
                setIndex(startIndex);
                break;
            }
            offsetList->push_back(addExp);
            if (!currentWord.checkType("RBRACK")) { //']'
                //setIndex(startIndex);
                //break;
                output.addError(new NoRightBracketsError(getPrevLine())); //can output first? I think yes
            } else {
                move();
            }
            type += 1;
            startIndex = wordIndex;
        }

        vector<Node*>* valueList = nullptr;
        if (currentWord.checkType("ASSIGN")) { //'='
            move();
            valueList = checkInitVal();
            if (valueList == nullptr) {
                free(offsetList);
                return nullptr;
            }
        }

        Node* offsetTree = (int)offsetList->size() == 0 ? nullptr : (*offsetList)[(int)offsetList->size()-1];
        variableDecl = new VariableDecl(name, offsetTree,
                                        valueList, type, false, currentLine);
        addLine("<VarDef>");
        return variableDecl;
    }

    vector<Node*>* checkInitVal() {
        int startIndex = wordIndex;
        Node* addExp = checkExp();
        vector<Node*>* list = new vector<Node*>();
        if (addExp != nullptr) {
            list->push_back(addExp);
            cout << "var traversal: " << endl;
            addExp->traversal();
            addLine("<InitVal>");
            return list;
        }

        setIndex(startIndex);
        if (!currentWord.checkType("LBRACE")) { //'{'
            free(list);
            return nullptr;
        }

        move();
        vector<Node*>* initValList = checkInitVal();
        if (initValList != nullptr)  {
            for (auto &val: *initValList) {
                list->push_back(val);
                cout << "var traversal: " << endl;
                val->traversal();
            }
            startIndex = wordIndex;
            while (startIndex < totalWord) { //','
                if (!currentWord.checkType("COMMA")) { //','
                    break;
                }
                move();
                initValList = checkInitVal();
                if (initValList == nullptr) {
                    setIndex(startIndex);
                    break;
                }
                for (auto &val: *initValList) {
                    list->push_back(val);
                    cout << "var traversal: " << endl;
                    val->traversal();
                }
                startIndex = wordIndex;
            }
        }
        if (!currentWord.checkType("RBRACE")) { //'}'
            free(list);
            return nullptr;
        }

        move();
        addLine("<InitVal>");
        return list;
    }

    FunF* checkFuncDef() {
        FunF* fun;
        string type = checkFuncType();
        if (type.empty()) {
            return nullptr;
        }

        int identLine = currentWord.getLine();
        string name = checkIdent();
        if (name.empty()) {
            return nullptr;
        }

        if (!currentWord.checkType("LPARENT")) { //'('
            return nullptr;
        }

        move();
        int startIndex = wordIndex;
        vector<FunFParam*>* param = checkFuncFParams();

        if (param == nullptr) {
            setIndex(startIndex);
            param = new vector<FunFParam*>();
        }

        if (!currentWord.checkType("RPARENT")) { //')'
            //return false;
            output.addError(new NoRightParenthesesError(getPrevLine()));
        } else {
            move();
        }
        fun = new FunF(name, param, type == "void" ? -1 : 0);
        symbolTable.insertFunTable(name, fun, identLine);

        /* into the function body*/
        symbolTable.addLayer();
        for (auto &para: (*param)) {
            //cout << "param: " << para->getName() << endl;
            VariableDecl* variableDecl = new VariableDecl(para->getName(), para->getOffset(), nullptr,
                                                          para->getType(), para->getConstType(), para->getLine());
            symbolTable.insertVarTable(para->getName(), variableDecl, para->getConstType(), para->getLine());
        }
        Block* block = checkBlock();
        symbolTable.popLayer();

        if (block == nullptr) {
            free(param);
            free(fun);
            return nullptr;
        }
        fun->setBlock(block);

        if (fun->getType() != -1) {
            bool hasReturnStmt = false;
            for (auto &item: block->getBlockItem()) {
                if (item->getClassType() == ReturnStmtType) {
                    hasReturnStmt = true;
                    break;
                }
            }
            if (!hasReturnStmt) {
                output.addError(new NoReturnError(getPrevLine(), fun->getName()));
            }
        }
        addLine("<FuncDef>");
        return fun;
    }

    FunF* checkMainFuncDef() {
        if (!currentWord.checkType("INTTK")) { //'int'
            return nullptr;
        }

        move();
        if (!currentWord.checkType("MAINTK")) { //'main'
            return nullptr;
        }

        move();
        if (!currentWord.checkType("LPARENT")) { //'('
            return nullptr;
        }

        move();
        if (!currentWord.checkType("RPARENT")) { //')'
            //return false;
            output.addError(new NoRightParenthesesError(getPrevLine()));
        } else {
            move();
        }

        FunF* mainFun = new FunF("main", new vector<FunFParam*>(), 0);
        symbolTable.insertFunTable("main", mainFun, currentWord.getLine());
        symbolTable.addLayer();
        Block* block = checkBlock();
        mainFun->setBlock(block);
        if (block == nullptr) {
            free(mainFun);
            symbolTable.popLayer();
            return nullptr;
        }

        bool hasReturnStmt = false;
        vector<Node*> blockItem = block->getBlockItem();
        for (auto & i : blockItem) {
            if (i->getClassType() == ReturnStmtType) {
                hasReturnStmt = true;
                break;
            }
        }
        if (!hasReturnStmt) {
            output.addError(new NoReturnError(getPrevLine(), "main"));
        }

        symbolTable.popLayer();
        addLine("<MainFuncDef>");
        return mainFun;
    }

    string checkFuncType() {
        if (currentWord.checkType("INTTK")) { //'int'
            move();
            addLine("<FuncType>");
            return "int";
        } else if (currentWord.checkType("VOIDTK")) { //'void'
            move();
            addLine("<FuncType>");
            return "void";
        }
        return "";
    }

    vector<FunFParam*>* checkFuncFParams() {
        vector<FunFParam*>* funFParams = new vector<FunFParam*>();
        FunFParam* funFParam = checkFuncFParam();
        if (funFParam == nullptr) {
            free(funFParams);
            return nullptr;
        }

        funFParams->push_back(funFParam);
        int startIndex = wordIndex;
        while (startIndex < totalWord) {
            if (!currentWord.checkType("COMMA")) { //','
                break;
            }

            move();
            funFParam = checkFuncFParam();
            if (funFParam == nullptr) {
                setIndex(startIndex);
                break;
            }
            funFParams->push_back(funFParam);
            startIndex = wordIndex;
        }
        addLine("<FuncFParams>");
        return funFParams;
    }

    FunFParam* checkFuncFParam() {
        if (!checkBType()) {
            return nullptr;
        }

        int currentLine = currentWord.getLine();
        string name = checkIdent();
        if (name.empty()) {
            return nullptr;
        }

        int startIndex = wordIndex;
        Node* offsetTree = nullptr;
        int type = 0;
        if (currentWord.checkType("LBRACK")) { //'['
            move();
            startIndex = wordIndex;
            type += 1;
            if (currentWord.checkType("RBRACK")) { //']'
                move();
                startIndex = wordIndex;
            } else {
                output.addError(new NoRightBracketsError(getPrevLine()));
            }

            while (startIndex < totalWord) {
                if (!currentWord.checkType("LBRACK")) { //'['
                    break;
                }
                type += 1;
                move();
                offsetTree = checkConstExp();
                if (offsetTree == nullptr) {
                    setIndex(startIndex);
                    break;
                }
                if (!currentWord.checkType("RBRACK")) { //']'
                    //setIndex(startIndex);
                    //break;
                    output.addError(new NoRightBracketsError(getPrevLine()));
                } else {
                    move();
                }
                startIndex = wordIndex;
            }
        }

        FunFParam* funFParam = new FunFParam(name, offsetTree, type, currentLine);
        addLine("<FuncFParam>");
        return funFParam;
    }

    Block* checkBlock() {
        if (!currentWord.checkType("LBRACE")) { //'{'
            return nullptr;
        }

        move();
        Block* block = new Block();
        int startIndex = wordIndex;
        while (true) {
            Node* blockItem = checkBlockItem();
            if (blockItem == nullptr)
                break;
            if (blockItem->getClassType() == DeclStmtType) {
                vector<VariableDecl*>* decl = ((DeclStmt*)blockItem)->getDecl();
                for (auto &vd: *decl) {
                    block->addBlockItem(vd);
                }
            } else if (blockItem->getClassType() != NullStmtType)
                block->addBlockItem(blockItem);
            startIndex = wordIndex;
        }

        setIndex(startIndex);
        if (!currentWord.checkType("RBRACE")) { //'}'
            free(block);
            return nullptr;
        }

        move();
        addLine("<Block>");
        return block;
    }

    Node* checkBlockItem() {
        int startIndex = wordIndex;
        DeclStmt* declStmt = checkDecl();
        if (declStmt != nullptr)
            return declStmt;

        setIndex(startIndex);
        Node* stmt = checkStmt();
        if (stmt != nullptr)
            return stmt;
        return nullptr;
    }

    Node* checkStmt() {
        int startIndex = wordIndex, currentLine = 0, errorCnt = output.getErrorSum();
        Node* exp = checkExp();
        if (exp != nullptr) {
            if (!currentWord.checkType("ASSIGN")) { //not '='
                if (!currentWord.checkType("SEMICN")) { //';'
                    output.addError(new NoSemicolonError(getPrevLine()));
                } else {
                    move();
                }
                addLine("<Stmt>");
                return exp;
            }
        }
        output.setError(errorCnt);

        setIndex(startIndex);
        if (currentWord.checkType("SEMICN")) {
            NullStmt* nullStmt = new NullStmt();
            move();
            addLine("<Stmt>");
            return nullStmt;
        }

        currentLine = currentWord.getLine();
        Variable* variable = checkLVal();
        if (variable != nullptr) {
            if (!currentWord.checkType("ASSIGN")) { //'='
                return nullptr;
            }
            move();
            startIndex = wordIndex;
            AssignExp* assignExp = new AssignExp();
            assignExp->setLch(variable);
            exp = checkExp();
            if (exp != nullptr) {
                if (!currentWord.checkType("SEMICN")) { //';'
                    //return false;
                    output.addError(new NoSemicolonError(getPrevLine()));
                }
                else {
                    move();
                }
                if (variable->getConstType()) { // const int -> int
                    output.addError(new ConstVariableChangeError(currentLine,
                                                             variable->getName()));
                }
                assignExp->setRch(exp);
                addLine("<Stmt>");
                return assignExp;
            }
            setIndex(startIndex);
            if (currentWord.checkType("GETINTTK")) { //'getint'
                move();
                if (!currentWord.checkType("LPARENT")) //'('
                    return nullptr;
                move();
                if (!currentWord.checkType("RPARENT")) { //')'
                    output.addError(new NoRightParenthesesError(getPrevLine()));
                } else {
                    move();
                }
                if (!currentWord.checkType("SEMICN")) { //';'
                    //return false;
                    output.addError(new NoSemicolonError(getPrevLine()));
                } else {
                    move();
                }
                if (variable->getConstType()) {
                    output.addError(new ConstVariableChangeError(currentLine,
                                                             variable->getName()));
                }
                ReadValue* readValue = new ReadValue();
                assignExp->setRch(readValue);
                addLine("<Stmt>");
                return assignExp;
            }

            free(assignExp);
            return nullptr;
        }

        setIndex(startIndex);
        if (currentWord.checkType("BREAKTK")) { //'break'
            move();
            BreakStmt* breakStmt = new BreakStmt();
            if (whileCnt == 0) {
                output.addError(new BreakContinueError(getPrevLine(), "break"));
            }
            if (!currentWord.checkType("SEMICN")) { //';'
                //return false;
                output.addError(new NoSemicolonError(getPrevLine()));
            } else {
                move();
            }
            addLine("<Stmt>");
            return breakStmt;
        }

        setIndex(startIndex);
        if (currentWord.checkType("CONTINUETK")) { //'continue'
            move();
            ContinueStmt* continueStmt = new ContinueStmt();
            if (whileCnt == 0) {
                output.addError(new BreakContinueError(getPrevLine(), "continue"));
            }
            if (!currentWord.checkType("SEMICN")) { //';'
                //return false;
                output.addError(new NoSemicolonError(getPrevLine()));
            } else {
                move();
            }
            addLine("<Stmt>");
            return continueStmt;
        }

        setIndex(startIndex);
        if (currentWord.checkType("RETURNTK")) { //'return'
            move();
            ReturnStmt* returnStmt = nullptr;
            startIndex = wordIndex;
            Node* addExp = checkExp();
            if (addExp == nullptr) {
                setIndex(startIndex);
                returnStmt = new ReturnStmt();
            } else {
                Table* table = symbolTable.getTopFun();
                if (table->getType() == -1) {
                    output.addError(new NotMatchReturnError(getPrevLine(), table->getName()));
                }
                returnStmt = new ReturnStmt(addExp);
            }
            if (!currentWord.checkType("SEMICN")) { //';'
                //return false;
                output.addError(new NoSemicolonError(getPrevLine()));
            } else {
                move();
            }
            addLine("<Stmt>");
            return returnStmt;
        }

        setIndex(startIndex);
        if (currentWord.checkType("PRINTFTK")) { //'printf'
            string format;
            move();
            currentLine = getPrevLine();
            if (!currentWord.checkType("LPARENT")) //'('
                return nullptr;
            move();
            if (!currentWord.checkType("STRCON")) //'FormatString'
                return nullptr;
            else {
                bool errorChar = false;
                format = currentWord.getValue();
                int len = (int)format.size();
                for (int i = 1; i < len - 1; i++) { //'"' must remove!
                    if (format[i] == 32 || format[i] == 33 || (format[i] >= 40 && format[i] <= 126 && format[i] != 92)) {
                        continue;
                    } else if (format[i] == '%') {
                        if (format[i+1] != 'd') {
                            errorChar = true;
                            break;
                        }
                        i += 1;
                    } else if (format[i] == '\\') {
                        if (format[i+1] != 'n') {
                            errorChar = true;
                            break;
                        }
                        i += 1;
                    } else {
                        errorChar = true;
                    }
                }
                if (errorChar) {
                    output.addError(new IllegalCharacterError(currentWord.getLine()));
                }
            }
            move();
            int formatLine = getPrevLine();
            vector<Node*>* expList = new vector<Node*>();
            startIndex = wordIndex;
            while(startIndex < totalWord) {
                if (!currentWord.checkType("COMMA")) { //','
                    break;
                }
                move();
                exp = checkExp();
                if (exp == nullptr) {
                    setIndex(startIndex);
                    break;
                }
                expList -> push_back(exp);
                startIndex = wordIndex;
            }
            if (!currentWord.checkType("RPARENT")) { //')'
                //return false;
                output.addError(new NoRightParenthesesError(getPrevLine()));
            } else {
                move();
            }
            if (!currentWord.checkType("SEMICN")) { //';'
                //return false;
                output.addError(new NoSemicolonError(getPrevLine()));
            } else {
                move();
            }
            PrintfStmt* printfStmt = new PrintfStmt(format, expList, currentLine);
            if (printfStmt->expectNum() != printfStmt->realNum()) {
                output.addError(new PrintParameterNumError(currentLine, printfStmt->expectNum(), printfStmt->realNum()));
            }
            addLine("<Stmt>");
            return printfStmt;
        }

        setIndex(startIndex);
        symbolTable.addLayer();
        Block* block = checkBlock();
        symbolTable.popLayer();
        if (block != nullptr) {
            addLine("<Stmt>");
            return block;
        }

        setIndex(startIndex);
        if (currentWord.checkType("WHILETK")) { //'while'
            move();
            if (!currentWord.checkType("LPARENT")) //'('
                return nullptr;
            move();
            Node* cond = checkCond();
            if (cond == nullptr)
                return nullptr;
            if (!currentWord.checkType("RPARENT")) { //')'
                //return false;
                output.addError(new NoRightParenthesesError(getPrevLine()));
            } else {
                move();
            }
            whileCnt += 1;
            Node* stmt = checkStmt();
            whileCnt -= 1;
            if (stmt == nullptr)
                return nullptr;
            WhileStmt* whileStmt = new WhileStmt(cond, stmt);
            addLine("<Stmt>");
            return whileStmt;
        }

        setIndex(startIndex);
        if (currentWord.checkType("IFTK")) { //'if'
            IfStmt* ifStmt = new IfStmt();
            move();
            if (!currentWord.checkType("LPARENT")) //'('
                return nullptr;
            move();
            Node* cond = checkCond();
            if (cond == nullptr)
                return nullptr;
            if (!currentWord.checkType("RPARENT")) { //')'
                //return false;
                output.addError(new NoRightParenthesesError(getPrevLine()));
            } else {
                move();
            }
            Node* ifSt = checkStmt();
            if (ifSt == nullptr) {
                free(cond);
                free(ifStmt);
                return nullptr;
            }
            ifStmt->addTran(cond, ifSt);
            startIndex = wordIndex;
            if (currentWord.checkType("ELSETK")) { //'else'
                move();
                Node* elseStmt = checkStmt();
                if (elseStmt == nullptr) {
                    free(cond);
                    free(ifSt);
                    free(ifStmt);
                    return nullptr;
                }
                ifStmt->addElseTran(elseStmt);
            }
            addLine("<Stmt>");
            return ifStmt;
        }
        return nullptr;
    }

    Node* checkExp() {
        Node* addExp = checkAddExp();
        if (addExp == nullptr) {
            return nullptr;
        }

        addLine("<Exp>");
        return addExp;
    }

    Node* checkCond() {
        Node* lOrExp = checkLOrExp();
        if (lOrExp == nullptr) {
            return nullptr;
        }

        addLine("<Cond>");
        return lOrExp;
    }

    Variable* checkLVal() {
        string name = checkIdent();
        if (name.empty())
            return nullptr;
        if (!symbolTable.checkUse(name, "var")) {
            output.addError(new UndefineNameError(getPrevLine(), name));
        }
        Table* table = symbolTable.getUse(name, "var");
        int beginType = table != nullptr ? table->getType() : 0;
        bool constFlag = table != nullptr && table->getConstFlag();
        int startIndex = wordIndex;
        vector<Node*> addList;
        while (startIndex < totalWord) {
            if (!currentWord.checkType("LBRACK")) //'['
                break;
            move();
            Node* addExp = checkExp();
            if (addExp == nullptr) {
                setIndex(startIndex);
                break;
            }
            addList.push_back(addExp->optimize());
            if (!currentWord.checkType("RBRACK")) { //']'
                //break;
                output.addError(new NoRightBracketsError(getPrevLine()));
            } else {
                move();
            }
            startIndex = wordIndex;
        }

        /* calculate the offset */
        Node* offset = nullptr, *setValue = nullptr;
        if ((int)addList.size() == 0) {
            offset = new Number(1);
            setValue = table ? ((VariableDecl*)table->getAstNode())->getPosValue(0) : nullptr;
        }
        else if((int)addList.size() == 1) {
            offset = addList[0];
            if (table && offset->getConstType() && offset->getSize() == 1) {
                Number* number = (Number*)offset;
                setValue = ((VariableDecl*)table->getAstNode())->getPosValue(number->getValue());
            }
        }
        else {
            Node* defOffset = table ? ((VariableDecl*)table->getAstNode())->getOffsetTree() : new Number(0);
            MulExp* mulExp = new MulExp("*");
            mulExp->setLch(addList[0]);
            mulExp->setRch(defOffset);
            AddExp* addExp = new AddExp("+");
            addExp->setLch(mulExp->optimize());
            addExp->setRch(addList[1]);
            offset = addExp->optimize();
            if (table && offset->getConstType() && offset->getSize() == 1) {
                Number* number = (Number*)offset;
                setValue = ((VariableDecl*)table->getAstNode())->getPosValue(number->getValue());
            } else {
                constFlag = false;
            }
        }

        Variable* variable = new Variable(name, offset, beginType-(int)addList.size(), setValue, constFlag);
        addLine("<LVal>");
        return variable;
    }

    Node* checkPrimaryExp() {
        int startIndex = wordIndex;
        Variable* lVal = checkLVal();
        if (lVal != nullptr) {
            addLine("<PrimaryExp>");
            return lVal;
        }

        setIndex(startIndex);
        Number* number = checkNumber();
        if (number != nullptr) {
            addLine("<PrimaryExp>");
            return number;
        }

        setIndex(startIndex);
        if (!currentWord.checkType("LPARENT")){ //'('
            return nullptr;
        }

        move();
        Node* addExp = checkExp();
        if (addExp == nullptr)
            return nullptr;
        if (!currentWord.checkType("RPARENT")) { //')'
            free(addExp);
            return nullptr;
        }
        move();
        addLine("<PrimaryExp>");
        return addExp;
    }

    Number* checkNumber() {
        string number = checkIntConst();
        if (number == "")
            return nullptr;
        int x = 0;
        for(auto &c: number) {
            x = (x << 3) + (x << 1) + c - '0';
        }
        addLine("<Number>");
        Number* number1 = new Number(x);
        return number1;
    }

    Node* checkUnaryExp() {
        int startIndex = wordIndex;
        int currentLine = currentWord.getLine();
        string name = checkIdent();
        if (!name.empty()) {
            FunR* funR = nullptr;
            if (currentWord.checkType("LPARENT")) { //'('
                move();
                startIndex = wordIndex;
                vector<Node*>* funRParams = checkFuncRParams();
                if (funRParams == nullptr) {
                    setIndex(startIndex);
                    funRParams = new vector<Node*>();
                }
                if (!currentWord.checkType("RPARENT")) { //')'
                    output.addError(new NoRightParenthesesError(getPrevLine()));
                    //return false;
                } else {
                    move();
                }
                if (!symbolTable.checkUse(name, "fun")) {
                    output.addError(new UndefineNameError(currentLine, name));
                    funR = new FunR(name, nullptr, funRParams);
                } else {
                    Table* table = symbolTable.getUse(name, "fun");
                    FunF* funF = (FunF*)table->getAstNode();
                    funR = new FunR(name, funF, funRParams);
                    funF->checkRParam(funRParams, currentLine);
                }
                addLine("<UnaryExp>");
                return funR;
            }
        }

        setIndex(startIndex);
        Node* primaryExp = checkPrimaryExp();
        if (primaryExp != nullptr) {
            addLine("<UnaryExp>");
            return primaryExp;
        }

        setIndex(startIndex);
        string op = checkUnaryOp();
        if (!op.empty()) {
            UnaryExp* unaryExp;
            Node* Lch = checkUnaryExp();
            if (Lch == nullptr)
                return nullptr;
            unaryExp = new UnaryExp(op);
            unaryExp->setLch(Lch);
            Node* root = unaryExp->optimize();
            addLine("<UnaryExp>");
            return root;
        }
        return nullptr;
    }

    string checkUnaryOp() {
        if (currentWord.checkType("PLUS") || currentWord.checkType("MINU") || currentWord.checkType("NOT")) {
            string op = currentWord.getValue();
            move();
            addLine("<UnaryOp>");
            return op;
        }
        return "";
    }

    vector<Node*>* checkFuncRParams() {
        Node* exp = checkExp();
        if (exp == nullptr) {
            return nullptr;
        }

        vector<Node*>* funRList = new vector<Node*>();
        funRList->push_back(exp);
        int startIndex = wordIndex;
        while (startIndex < totalWord) {
            if (!currentWord.checkType("COMMA")) //','
                break;
            move();
            exp = checkExp();
            if (exp == nullptr) {
                setIndex(startIndex);
                break;
            }
            funRList->push_back(exp);
            startIndex = wordIndex;
        }
        addLine("<FuncRParams>");
        return funRList;
    }

    Node* checkMulExp() {
        Node* unaryExp = checkUnaryExp();
        if (unaryExp == nullptr)
            return nullptr;
        int startIndex = wordIndex;
        Node *last = unaryExp;
        while (startIndex < totalWord) {
            if (!currentWord.checkType("MULT") && !currentWord.checkType("DIV") && !currentWord.checkType("MOD")) //'*' or '/' or '%'
                break;
            MulExp* mulExp = new MulExp(currentWord.getValue());
            addLine("<MulExp>");
            move();
            unaryExp = checkUnaryExp();
            if (unaryExp == nullptr) {
                setIndex(startIndex);
                break;
            }
            mulExp->setLch(last);
            mulExp->setRch(unaryExp);
            last = mulExp->optimize();
            startIndex = wordIndex;
        }
        addLine("<MulExp>");
        return last;
    }

    Node* checkAddExp() {
        Node* mulExp = checkMulExp();
        if (mulExp == nullptr)
            return nullptr;
        int startIndex = wordIndex;
        Node *last = mulExp;
        while (startIndex < totalWord) {
            if (!currentWord.checkType("PLUS") && !currentWord.checkType("MINU") ) //'+' or '-'
                break;
            AddExp* addExp = new AddExp(currentWord.getValue());
            addLine("<AddExp>");
            move();
            mulExp = checkMulExp();
            if (mulExp == nullptr) {
                setIndex(startIndex);
                break;
            }
            addExp->setLch(last);
            addExp->setRch(mulExp);
            last = addExp->optimize();
            startIndex = wordIndex;
        }
        addLine("<AddExp>");
        return last;
    }

    Node* checkRelExp() {
        Node* addExp = checkAddExp();
        if (addExp == nullptr)
            return nullptr;
        int startIndex = wordIndex;
        Node *last = addExp;
        while (startIndex < totalWord) {
            if (!currentWord.checkType("LSS") && !currentWord.checkType("LEQ") && !currentWord.checkType("GRE") && !currentWord.checkType("GEQ")) //'<' or '<=' or '>' or '>='
                break;
            RelExp* relExp = new RelExp(currentWord.getValue());
            addLine("<RelExp>");
            move();
            addExp = checkAddExp();
            if (addExp == nullptr) {
                setIndex(startIndex);
                break;
            }
            relExp->setLch(last);
            relExp->setRch(addExp);
            last = relExp->optimize();
            startIndex = wordIndex;
        }
        addLine("<RelExp>");
        return last;
    }

    Node* checkEqExp() {
        Node* relExp = checkRelExp();
        if (relExp == nullptr)
            return nullptr;
        int startIndex = wordIndex;
        Node* last = relExp;
        while (startIndex < totalWord) {
            if (!currentWord.checkType("EQL") && !currentWord.checkType("NEQ")) //'==' or '!='
                break;
            EqExp* eqExp = new EqExp(currentWord.getValue());
            addLine("<EqExp>");
            move();
            relExp = checkRelExp();
            if (relExp == nullptr) {
                setIndex(startIndex);
                break;
            }
            eqExp->setLch(last);
            eqExp->setRch(relExp);
            last = eqExp->optimize();
            startIndex = wordIndex;
        }
        addLine("<EqExp>");
        return last;
    }

    Node* checkLAndExp() {
        Node* eqExp = checkEqExp();
        if (eqExp == nullptr)
            return nullptr;
        int startIndex = wordIndex;
        Node* last = eqExp;
        while (startIndex < totalWord) {
            if (!currentWord.checkType("AND")) //'&&'
                break;
            LAndExp* lAndExp = new LAndExp(currentWord.getValue());
            addLine("<LAndExp>");
            move();
            eqExp = checkEqExp();
            if (eqExp == nullptr) {
                setIndex(startIndex);
                break;
            }
            lAndExp->setLch(last);
            lAndExp->setRch(eqExp);
            last = lAndExp->optimize();
            startIndex = wordIndex;
        }
        addLine("<LAndExp>");
        return last;
    }

    Node* checkLOrExp() {
        Node* lAndExp = checkLAndExp();
        if (lAndExp == nullptr)
            return nullptr;
        int startIndex = wordIndex;
        Node* last = lAndExp;
        while (startIndex < totalWord) {
            if (!currentWord.checkType("OR")) //'&&'
                break;
            LOrExp* lOrExp = new LOrExp(currentWord.getValue());
            addLine("<LOrExp>");
            move();
            lAndExp = checkLAndExp();
            if (lAndExp == nullptr) {
                setIndex(startIndex);
                break;
            }
            lOrExp->setLch(last);
            lOrExp->setRch(lAndExp);
            last = lOrExp->optimize();
            startIndex = wordIndex;
        }
        addLine("<LOrExp>");
        return last;
    }

    Node* checkConstExp() {
        Node* addExp = checkAddExp();
        if (addExp != nullptr) {
            addLine("<ConstExp>");
            return addExp;
        }
        return nullptr;
    }

    string checkIdent() {
        if (currentWord.checkType("IDENFR")) {
            string name = currentWord.getValue();
            move();
            return name;
        }
        return "";
    }

    string checkIntConst() {
        if (currentWord.checkType("INTCON")) {
            string value = currentWord.getValue();
            move();
            return value;
        }
        return "";
    }

public:
    Grammar() = default;
    explicit Grammar(string article) {
        lexical = Lexical(article);
        totalWord = lexical.totalWordCount();
        currentWord = lexical.getWord(0);
        Node* state = checkCompUnit();
        if (state == nullptr) {
            cout << "error when decode" << endl;
            exit(1);
        }
        if (wordIndex != totalWord) {
            cout << "Error, some word don't use!" << endl;
            cout << "Current word: " << currentWord.getValue() << endl;
            exit(1);
        }
    }
};

#endif //GRAMMAR_1005_GRAMMAR_H
