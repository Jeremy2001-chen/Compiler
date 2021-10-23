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
    void addLine(string str) {
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
            for (auto& it: (*variableDeclList->getDecl())) {
                compUnit->setVar(it);
            }
            if (variableDeclList != nullptr) {
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
            compUnit = nullptr;
            return compUnit;
        }
        compUnit->setMainBlock(mainFunction);
        addLine("<CompUnit>");
        return compUnit;
    }

    DeclStmt* checkDecl() {
        DeclStmt* variableDeclList = nullptr;
        int startIndex = wordIndex;
        variableDeclList = checkConstDecl();
        if (variableDeclList != nullptr) {
            return variableDeclList;
        }
        setIndex(startIndex);
        variableDeclList = checkVarDecl();
        if (variableDeclList != nullptr) {
            return variableDeclList;
        }
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

        int startIndex = wordIndex, errorLine = currentWord.getLine();
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
            errorLine = currentWord.getLine();
            variableDeclList->addDecl(variableDecl);
            symbolTable.insertVarTable(variableDecl->getName(), variableDecl,
                                       variableDecl->getConstType(), variableDecl->getLine());
            startIndex = wordIndex;
        }
        if (!currentWord.checkType("SEMICN")) { //';'
            output.addError(new NoSemicolonError(errorLine));
        }
        move();
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
        VariableDecl* variableDecl = nullptr;
        int currentLine = currentWord.getLine();
        string name = checkIdent();
        if (name.empty())
            return variableDecl;
        int startIndex = wordIndex, errorLine = 0;
        Node* addExp = nullptr;
        vector<Node*>* offsetList = new vector<Node*>();
        while (startIndex < totalWord) {
            if (!currentWord.checkType("LBRACK")) //'['
                break;
            move();
            errorLine = currentWord.getLine();
            addExp = checkConstExp();
            if (addExp != nullptr) {
                setIndex(startIndex);
                break;
            }
            offsetList->push_back(addExp);
            if (!currentWord.checkType("RBRACK")) { //']'
                //setIndex(startIndex);
                //break;
                output.addError(new NoRightBracketsError(errorLine)); //can output first? I think yes
            } else {
                move();
            }
            startIndex = wordIndex;
        }
        if (!currentWord.checkType("ASSIGN")) //'=
            return variableDecl;
        move();
        vector<Node*>* valueList = checkConstInitVal();
        if (valueList == nullptr)
            return variableDecl;
        variableDecl = new VariableDecl(name, (*offsetList)[(int)offsetList->size()-1],
                                        valueList, true, currentLine);
        addLine("<ConstDef>");
        return variableDecl;
    }

    vector<Node*>* checkConstInitVal() {
        vector<Node*>* list = new vector<Node*>();
        int startIndex = wordIndex;
        Node* exp = checkConstExp();
        if (exp != nullptr) {
            list->push_back(exp);
            addLine("<ConstInitVal>");
            return list;
        }
        setIndex(startIndex);
        if (!currentWord.checkType("LBRACE")) { //'{'
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
        int startIndex = wordIndex, errorLine = currentWord.getLine();
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
            errorLine = currentWord.getLine();
            startIndex = wordIndex;
            varList->addDecl(variableDecl);
            symbolTable.insertVarTable(variableDecl->getName(), variableDecl,
                                       variableDecl->getConstType(), variableDecl->getLine());
        }
        if (!currentWord.checkType("SEMICN")) { //';'
            //return false;
            output.addError(new NoSemicolonError(errorLine));
        } else {
            move();
        }
        addLine("<VarDecl>");
        return varList;
    }

    VariableDecl* checkVarDef() {
        int currentLine = currentWord.getLine();
        VariableDecl* variableDecl = nullptr;
        string name = checkIdent();
        if (name.empty()) {
            return variableDecl;
        }
        int startIndex = wordIndex, errorLine = 0;
        Node* addExp = nullptr;
        vector<Node*>* offsetList = new vector<Node*>();
        while (startIndex < totalWord) {
            if (!currentWord.checkType("LBRACK")) { //'['
                break;
            }
            move();
            addExp = checkConstExp();
            errorLine = currentWord.getLine();
            if (addExp == nullptr) {
                setIndex(startIndex);
                break;
            }
            offsetList->push_back(addExp);
            if (!currentWord.checkType("RBRACK")) { //']'
                //setIndex(startIndex);
                //break;
                output.addError(new NoRightBracketsError(errorLine)); //can output first? I think yes
            } else
                move();
            startIndex = wordIndex;
        }
        vector<Node*>* valueList = nullptr;
        if (currentWord.checkType("ASSIGN")) { //'='
            move();
            valueList = checkInitVal();
            if (valueList == nullptr)
                return nullptr;
        }
        variableDecl = new VariableDecl(name, (*offsetList)[(int)offsetList->size()-1],
                                        valueList, false, currentLine);
        addLine("<VarDef>");
        return variableDecl;
    }

    vector<Node*>* checkInitVal() {
        int startIndex = wordIndex;
        Node* addExp = checkExp();
        vector<Node*>* list = new vector<Node*>();
        if (addExp != nullptr) {
            list->push_back(addExp);
            addLine("<InitVal>");
            return list;
        }
        setIndex(startIndex);
        if (!currentWord.checkType("LBRACE")) { //'{'
            return nullptr;
        }
        move();
        vector<Node*>* initValList = checkInitVal();
        if (initValList != nullptr)  {
            for (auto &val: *initValList) {
                list->push_back(val);
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
                }
                startIndex = wordIndex;
            }
        }
        if (!currentWord.checkType("RBRACE")) { //'}'
            return nullptr;
        }
        move();
        addLine("<InitVal>");
        return list;
    }

    FunF* checkFuncDef() {
        FunF* fun = nullptr;
        string type = checkFuncType();
        if (type.empty()) {
            return nullptr;
        }
        int identLine = currentWord.getLine(), noEndLine = currentWord.getLine();
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
        if (param == nullptr)
            setIndex(startIndex);
        else {
            noEndLine = (*param)[(*param).size()-1]->getLine();
        }
        if (!currentWord.checkType("RPARENT")) { //')'
            //return false;
            output.addError(new NoRightParenthesesError(noEndLine));
        } else {
            move();
        }
        fun = new FunF(name, param, type == "void" ? -1 : 0);
        symbolTable.insertFunTable(name, fun, identLine);

        /* into the function body*/
        symbolTable.addLayer();
        for (auto &para: (*param)) {
            symbolTable.insertVarTable(para->getName(), para, para->getConstType(), para->getLine());
        }
        Block* block = checkBlock();
        symbolTable.popLayer();

        if (block == nullptr) {
            return nullptr;
        }
        fun->setBlock(block);
        // not void function
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
        int currentLine = currentWord.getLine();
        move();
        if (!currentWord.checkType("RPARENT")) { //')'
            //return false;
            output.addError(new NoRightParenthesesError(currentLine));
        } else {
            move();
        }
        FunF* mainFun = new FunF("main", nullptr, 0);
        symbolTable.insertFunTable("main", mainFun, currentWord.getLine());
        symbolTable.addLayer();
        Block* block = checkBlock();
        mainFun->setBlock(block);
        if (block == nullptr) {
            return nullptr;
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
                output.addError(new NoRightBracketsError(currentLine));
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
                    output.addError(new NoRightBracketsError(currentLine));
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
        Block* block = new Block();
        move();
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
            return nullptr;
        }
        move();
        addLine("<Block>");
        return block;
    }

    Node* checkBlockItem() {
        int startIndex = wordIndex;
        DeclStmt* declStmt = checkDecl();
        if (declStmt != nullptr) {
            return declStmt;
        }
        setIndex(startIndex);
        Node* stmt = checkStmt();
        if (stmt != nullptr) {
            return stmt;
        }
        return nullptr;
    }

    Node* checkStmt() {
        int startIndex = wordIndex, currentLine = 0;
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
                else
                    move();
                if (variable->getConstType() && !exp->getConstType()) { // const int -> int
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
                } else
                    move();
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
                for (int i = 0; i < len; i++) {
                    if (format[i] != 32 && format[i] != 33 && (format[i] < 40 || format[i] > 126))
                        errorChar = true;
                    if (format[i] == '\\') {
                        if (!(i < len - 1 && format[i+1] == '\n'))
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
            if (printfStmt->expectNum() == printfStmt->realNum()) {
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
            ifStmt->addTran(cond, ifSt);
            if (ifSt == nullptr)
                return nullptr;
            startIndex = wordIndex;
            if (currentWord.checkType("ELSETK")) { //'else'
                move();
                Node* elseStmt = checkStmt();
                if (elseStmt == nullptr)
                    return nullptr;
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
        int startIndex = wordIndex;
        vector<Node*> addList;
        while (startIndex < totalWord) {
            if (!currentWord.checkType("LBRACK")) //'['
                return nullptr;
            move();
            Node* addExp = checkExp();
            if (addExp == nullptr) {
                setIndex(startIndex);
                break;
            }
            addList.push_back(addExp);
            if (!currentWord.checkType("RBRACK")) { //']'
                //break;
                output.addError(new NoRightBracketsError(getPrevLine()));
            } else {
                move();
            }
            startIndex = wordIndex;
        }
        Variable* variable = nullptr;
        if (addList.size() == 0) {
            variable = new Variable(name, nullptr, 0, table->getConstFlag());
        } else if (addList.size() == 1) {
            variable = new Variable(name, addList[0], 1, table->getConstFlag());
        } else if (addList.size() == 2) {
            AddExp* addExp = new AddExp("+");
            MulExp* mulExp = new MulExp("*");
            Node* offset = ((VariableDecl*)table->getAstNode())->getOffsetTree();
            addExp->setRch(addList[1]);
            mulExp->setLch(addList[0]);
            mulExp->setRch(offset);
            addExp->setLch(mulExp);
            variable = new Variable(name, addExp, 2, table->getConstFlag());
        }
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
        if (!currentWord.checkType("RPARENT")) //')'
            return nullptr;
        move();
        addLine("<PrimaryExp>");
        return addExp;
    }

    Number* checkNumber() {
        string number = checkIntConst();
        if (number == "") {
            return nullptr;
        }
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
                if (funRParams == nullptr)
                    setIndex(startIndex);
                if (!currentWord.checkType("RPARENT")) { //')'
                    output.addError(new NoRightParenthesesError(getPrevLine()));
                    //return false;
                } else {
                    move();
                }
                if (!symbolTable.checkUse(name, "fun")) {
                    output.addError(new NameRedefineError(currentLine, name));
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
            UnaryExp* unaryExp = nullptr;
            Node* Lch = checkUnaryExp();
            if (Lch == nullptr)
                return nullptr;
            unaryExp = new UnaryExp(op);
            unaryExp->setLch(Lch);
            addLine("<UnaryExp>");
            return unaryExp;
        }
        return nullptr;
    }

    string checkUnaryOp() {
        if (currentWord.checkType("PLUS") || currentWord.checkType("MINU") || currentWord.checkType("NOT")) {
            move();
            addLine("<UnaryOp>");
            return currentWord.getValue();
        }
        return "";
    }

    vector<Node*>* checkFuncRParams() {
        vector<Node*>* funRList = new vector<Node*>();
        Node* exp = checkExp();
        if (exp == nullptr) {
            return nullptr;
        }
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
            last = mulExp;
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
            last = addExp;
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
            last = relExp;
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
            last = eqExp;
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
            last = lAndExp;
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
            move();
            return currentWord.getValue();
        }
        return "";
    }

    string checkIntConst() {
        if (currentWord.checkType("INTCON")) {
            move();
            return currentWord.getValue();
        }
        return "";
    }

public:
    Grammar() = default;
    Grammar(string article) {
        lexical = Lexical(article);
        totalWord = lexical.totalWordCount();
        currentWord = lexical.getWord(0);
        Node* state = checkCompUnit();
        if (state == nullptr) {
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
