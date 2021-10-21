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

extern Output output;

class Grammar{
private:
    Lexical lexical;
    Word currentWord;
    Node root;
    int wordIndex = 0, totalWord;

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

    bool checkCompUnit(){
        int startIndex = wordIndex;
        while (startIndex < totalWord) {
            if (checkDecl()) {
                startIndex = wordIndex;
            } else {
                setIndex(startIndex);
                break;
            }
        }
        while (startIndex < totalWord) {
            if (checkFuncDef()) {
                startIndex = wordIndex;
            } else {
                setIndex(startIndex);
                break;
            }
        }
        bool state = checkMainFuncDef();
        if (!state) {
            return false;
        }
        addLine("<CompUnit>");
        return true;
    }

    bool checkDecl() {
        int startIndex = wordIndex;
        if (checkConstDecl()) {
            return true;
        }
        setIndex(startIndex);
        if (checkVarDecl()) {
            return true;
        }
        return false;
    }

    bool checkConstDecl() {
        if (!currentWord.checkType("CONSTTK")) //'const'
            return false;
        move();
        if (!checkBType())
            return false;
        if (!checkConstDef())
            return false;
        int startIndex = wordIndex;
        while (startIndex < totalWord) {
            if (!currentWord.checkType("COMMA")) { //','
                break;
            }
            move();
            if (!checkConstDef()) {
                setIndex(startIndex);
                break;
            }
            startIndex = wordIndex;
        }
        if (!currentWord.checkType("SEMICN")) //';'
            return false;
        move();
        addLine("<ConstDecl>");
        return true;
    }

    bool checkBType() {
        if (!currentWord.checkType("INTTK")) //'int'
            return false;
        move();
        return true;
    }

    bool checkConstDef() {
        if (!checkIdent())
            return false;
        int startIndex = wordIndex;
        while (startIndex < totalWord) {
            if (!currentWord.checkType("LBRACK")) //'['
                break;
            move();
            if (!checkConstExp()) {
                setIndex(startIndex);
                break;
            }
            if (!currentWord.checkType("RBRACK")) { //']'
                setIndex(startIndex);
                break;
            }
            move();
            startIndex = wordIndex;
        }
        if (!currentWord.checkType("ASSIGN")) //'=
            return false;
        move();
        if (!checkConstInitVal())
            return false;
        addLine("<ConstDef>");
        return true;
    }

    bool checkConstInitVal() {
        int startIndex = wordIndex;
        if (checkConstExp()) {
            addLine("<ConstInitVal>");
            return true;
        }
        setIndex(startIndex);
        if (!currentWord.checkType("LBRACE")) { //'{'
            return false;
        }
        move();
        if (checkConstInitVal())  {
            startIndex = wordIndex;
            while (startIndex < totalWord) { //','
                if (!currentWord.checkType("COMMA")) { //','
                    break;
                }
                move();
                if (!checkConstInitVal()) {
                    setIndex(startIndex);
                    break;
                }
                startIndex = wordIndex;
            }
        }
        if (!currentWord.checkType("RBRACE")) { //'}'
            return false;
        }
        move();
        addLine("<ConstInitVal>");
        return true;
    }

    bool checkVarDecl() {
        if (!checkBType()) {
            return false;
        }
        if (!checkVarDef()) {
            return false;
        }
        int startIndex = wordIndex;
        while (startIndex < totalWord) {
            if (!currentWord.checkType("COMMA")) { //','
                break;
            }
            move();
            if (!checkVarDef()) {
                setIndex(startIndex);
                break;
            }
            startIndex = wordIndex;
        }
        if (!currentWord.checkType("SEMICN")) { //';'
            return false;
        }
        move();
        addLine("<VarDecl>");
        return true;
    }

    bool checkVarDef() {
        if (!checkIdent()) {
            return false;
        }
        int startIndex = wordIndex;
        while (startIndex < totalWord) {
            if (!currentWord.checkType("LBRACK")) { //'['
                break;
            }
            move();
            if (!checkConstExp()) {
                setIndex(startIndex);
                break;
            }
            if (!currentWord.checkType("RBRACK")) { //']'
                setIndex(startIndex);
                break;
            }
            move();
            startIndex = wordIndex;
        }
        if (currentWord.checkType("ASSIGN")) { //'='
            move();
            if (!checkInitVal())
                return false;
        }
        addLine("<VarDef>");
        return true;
    }

    bool checkInitVal() {
        int startIndex = wordIndex;
        if (checkExp()) {
            addLine("<InitVal>");
            return true;
        }
        setIndex(startIndex);
        if (!currentWord.checkType("LBRACE")) { //'{'
            return false;
        }
        move();
        if (checkInitVal())  {
            startIndex = wordIndex;
            while (startIndex < totalWord) { //','
                if (!currentWord.checkType("COMMA")) { //','
                    break;
                }
                move();
                if (!checkInitVal()) {
                    setIndex(startIndex);
                    break;
                }
                startIndex = wordIndex;
            }
        }
        if (!currentWord.checkType("RBRACE")) { //'}'
            return false;
        }
        move();
        addLine("<InitVal>");
        return true;
    }

    bool checkFuncDef() {
        if (!checkFuncType()) {
            return false;
        }
        if (!checkIdent()) {
            return false;
        }
        if (!currentWord.checkType("LPARENT")) { //'('
            return false;
        }
        move();
        int startIndex = wordIndex;
        if (!checkFuncFParams())
            setIndex(startIndex);
        if (!currentWord.checkType("RPARENT")) { //')'
            return false;
        }
        move();
        if (!checkBlock()) {
            return false;
        }
        addLine("<FuncDef>");
        return true;
    }

    bool checkMainFuncDef() {
        if (!currentWord.checkType("INTTK")) { //'int'
            return false;
        }
        move();
        if (!currentWord.checkType("MAINTK")) { //'main'
            return false;
        }
        move();
        if (!currentWord.checkType("LPARENT")) { //'('
            return false;
        }
        move();
        if (!currentWord.checkType("RPARENT")) { //')'
            return false;
        }
        move();
        if (!checkBlock()) {
            return false;
        }
        addLine("<MainFuncDef>");
        return true;
    }

    bool checkFuncType() {
        if (currentWord.checkType("INTTK") || currentWord.checkType("VOIDTK")) { //'int' or 'void'
            move();
            addLine("<FuncType>");
            return true;
        }
        return false;
    }

    bool checkFuncFParams() {
        if (!checkFuncFParam()) {
            return false;
        }
        int startIndex = wordIndex;
        while (startIndex < totalWord) {
            if (!currentWord.checkType("COMMA")) { //','
                break;
            }
            move();
            if (!checkFuncFParam()) {
                setIndex(startIndex);
                break;
            }
            startIndex = wordIndex;
        }
        addLine("<FuncFParams>");
        return true;
    }

    bool checkFuncFParam() {
        if (!checkBType()) {
            return false;
        }
        if (!checkIdent()) {
            return false;
        }
        int startIndex = wordIndex;
        if (currentWord.checkType("LBRACK")) { //'['
            move();
            if (currentWord.checkType("RBRACK")) { //']'
                move();
                startIndex = wordIndex;
                while (startIndex < totalWord) {
                    if (!currentWord.checkType("LBRACK")) { //'['
                        break;
                    }
                    move();
                    if (!checkConstExp()) {
                        setIndex(startIndex);
                        break;
                    }
                    if (!currentWord.checkType("RBRACK")) { //']'
                        setIndex(startIndex);
                        break;
                    }
                    move();
                    startIndex = wordIndex;
                }
            } else {
                setIndex(startIndex);
            }
        }
        addLine("<FuncFParam>");
        return true;
    }

    bool checkBlock() {
        if (!currentWord.checkType("LBRACE")) { //'{'
            return false;
        }
        move();
        int startIndex = wordIndex;
        while (checkBlockItem())
            startIndex = wordIndex;
        setIndex(startIndex);

        if (!currentWord.checkType("RBRACE")) { //'}'
            return false;
        }
        move();
        addLine("<Block>");
        return true;
    }

    bool checkBlockItem() {
        int startIndex = wordIndex;
        if (checkDecl()) {
            return true;
        }
        setIndex(startIndex);
        if (checkStmt()) {
            return true;
        }
        return false;
    }

    bool checkStmt() {
        int startIndex = wordIndex;
        if (checkExp()) {
            if (currentWord.checkType("SEMICN")) {
                move();
                addLine("<Stmt>");
                return true;
            }
        }
        setIndex(startIndex);
        if (currentWord.checkType("SEMICN")) {
            move();
            addLine("<Stmt>");
            return true;
        }
        if (checkLVal()) {
            if (!currentWord.checkType("ASSIGN")) { //'='
                return false;
            }
            move();
            startIndex = wordIndex;
            if (checkExp()) {
                if (!currentWord.checkType("SEMICN")) //';'
                    return false;
                move();
                addLine("<Stmt>");
                return true;
            }
            setIndex(startIndex);
            if (currentWord.checkType("GETINTTK")) { //'getint'
                move();
                if (!currentWord.checkType("LPARENT")) //'('
                    return false;
                move();
                if (!currentWord.checkType("RPARENT")) //')'
                    return false;
                move();
                if (!currentWord.checkType("SEMICN")) //';'
                    return false;
                move();
                addLine("<Stmt>");
                return true;
            }
            return false;
        }
        setIndex(startIndex);
        if (currentWord.checkType("BREAKTK")) { //'break'
            move();
            if (!currentWord.checkType("SEMICN")) //';'
                return false;
            move();
            addLine("<Stmt>");
            return true;
        }
        setIndex(startIndex);
        if (currentWord.checkType("CONTINUETK")) { //'continue'
            move();
            if (!currentWord.checkType("SEMICN")) //';'
                return false;
            move();
            addLine("<Stmt>");
            return true;
        }
        setIndex(startIndex);
        if (currentWord.checkType("RETURNTK")) { //'return'
            move();
            startIndex = wordIndex;
            if (!checkExp()) {
                setIndex(startIndex);
            }
            if (!currentWord.checkType("SEMICN")) //';'
                return false;
            move();
            addLine("<Stmt>");
            return true;
        }
        setIndex(startIndex);
        if (currentWord.checkType("PRINTFTK")) { //'printf'
            move();
            if (!currentWord.checkType("LPARENT")) //'('
                return false;
            move();
            if (!currentWord.checkType("STRCON")) //'FormatString'
                return false;
            move();
            startIndex = wordIndex;
            while(startIndex < totalWord) {
                if (!currentWord.checkType("COMMA")) { //','
                    break;
                }
                move();
                if (!checkExp()) {
                    setIndex(startIndex);
                    break;
                }
                startIndex = wordIndex;
            }
            if (!currentWord.checkType("RPARENT")) //')'
                return false;
            move();
            if (!currentWord.checkType("SEMICN")) //';'
                return false;
            move();
            addLine("<Stmt>");
            return true;
        }
        setIndex(startIndex);
        if (checkBlock()) {
            addLine("<Stmt>");
            return true;
        }
        setIndex(startIndex);
        if (currentWord.checkType("WHILETK")) { //'while'
            move();
            if (!currentWord.checkType("LPARENT")) //'('
                return false;
            move();
            if (!checkCond())
                return false;
            if (!currentWord.checkType("RPARENT")) //')'
                return false;
            move();
            if (!checkStmt())
                return false;
            addLine("<Stmt>");
            return true;
        }
        setIndex(startIndex);
        if (currentWord.checkType("IFTK")) { //'if'
            move();
            if (!currentWord.checkType("LPARENT")) //'('
                return false;
            move();
            if (!checkCond())
                return false;
            if (!currentWord.checkType("RPARENT")) //')'
                return false;
            move();
            if (!checkStmt())
                return false;
            startIndex = wordIndex;
            if (currentWord.checkType("ELSETK")) { //'else'
                move();
                if (!checkStmt())
                    return false;
            }
            addLine("<Stmt>");
            return true;
        }
        return false;
    }

    bool checkExp() {
        if (!checkAddExp()) {
            return false;
        }
        addLine("<Exp>");
        return true;
    }

    bool checkCond() {
        if (!checkLOrExp()) {
            return false;
        }
        addLine("<Cond>");
        return true;
    }

    bool checkLVal() {
        if (!checkIdent())
            return false;
        int startIndex = wordIndex;
        while (startIndex < totalWord) {
            if (!currentWord.checkType("LBRACK")) //'['
                break;
            move();
            if (!checkExp()) {
                setIndex(startIndex);
                break;
            }
            if (!currentWord.checkType("RBRACK")) //']'
                break;
            move();
            startIndex = wordIndex;
        }
        addLine("<LVal>");
        return true;
    }

    bool checkPrimaryExp() {
        int startIndex = wordIndex;
        if (checkLVal()) {
            addLine("<PrimaryExp>");
            return true;
        }
        setIndex(startIndex);
        if (checkNumber()) {
            addLine("<PrimaryExp>");
            return true;
        }
        setIndex(startIndex);
        if (!currentWord.checkType("LPARENT")) //'('
            return false;
        move();
        if (!checkExp())
            return false;
        if (!currentWord.checkType("RPARENT")) //')'
            return false;
        move();
        addLine("<PrimaryExp>");
        return true;
    }

    bool checkNumber() {
        if (!checkIntConst()) {
            return false;
        }
        addLine("<Number>");
        return true;
    }

    bool checkUnaryExp() {
        int startIndex = wordIndex;
        if (checkIdent()) {
            if (currentWord.checkType("LPARENT")) { //'('
                move();
                startIndex = wordIndex;
                if (!checkFuncRParams())
                    setIndex(startIndex);
                if (!currentWord.checkType("RPARENT")) //')'
                    return false;
                move();
                addLine("<UnaryExp>");
                return true;
            }
        }
        setIndex(startIndex);
        if (checkPrimaryExp()) {
            addLine("<UnaryExp>");
            return true;
        }
        setIndex(startIndex);
        if (checkUnaryOp()) {
            if (!checkUnaryExp())
                return false;
            addLine("<UnaryExp>");
            return true;
        }
        return false;
    }

    bool checkUnaryOp() {
        if (currentWord.checkType("PLUS") || currentWord.checkType("MINU") || currentWord.checkType("NOT")) {
            move();
            addLine("<UnaryOp>");
            return true;
        }
        return false;
    }

    bool checkFuncRParams() {
        if (!checkExp()) {
            return false;
        }
        int startIndex = wordIndex;
        while (startIndex < totalWord) {
            if (!currentWord.checkType("COMMA")) //','
                break;
            move();
            if (!checkExp()) {
                setIndex(startIndex);
                break;
            }
            startIndex = wordIndex;
        }
        addLine("<FuncRParams>");
        return true;
    }

    bool checkMulExp() {
        if (!checkUnaryExp())
            return false;
        int startIndex = wordIndex;
        while (startIndex < totalWord) {
            if (!currentWord.checkType("MULT") && !currentWord.checkType("DIV") && !currentWord.checkType("MOD")) //'*' or '/' or '%'
                break;
            addLine("<MulExp>");
            move();
            if (!checkUnaryExp()) {
                setIndex(startIndex);
                break;
            }
            startIndex = wordIndex;
        }
        addLine("<MulExp>");
        return true;
    }

    bool checkAddExp() {
        if (!checkMulExp())
            return false;
        int startIndex = wordIndex;
        while (startIndex < totalWord) {
            if (!currentWord.checkType("PLUS") && !currentWord.checkType("MINU") ) //'+' or '-'
                break;
            addLine("<AddExp>");
            move();
            if (!checkMulExp()) {
                setIndex(startIndex);
                break;
            }
            startIndex = wordIndex;
        }
        addLine("<AddExp>");
        return true;
    }

    bool checkRelExp() {
        if (!checkAddExp())
            return false;
        int startIndex = wordIndex;
        while (startIndex < totalWord) {
            if (!currentWord.checkType("LSS") && !currentWord.checkType("LEQ") && !currentWord.checkType("GRE") && !currentWord.checkType("GEQ")) //'<' or '<=' or '>' or '>='
                break;
            addLine("<RelExp>");
            move();
            if (!checkAddExp()) {
                setIndex(startIndex);
                break;
            }
            startIndex = wordIndex;
        }
        addLine("<RelExp>");
        return true;
    }

    bool checkEqExp() {
        if (!checkRelExp())
            return false;
        int startIndex = wordIndex;
        while (startIndex < totalWord) {
            if (!currentWord.checkType("EQL") && !currentWord.checkType("NEQ")) //'==' or '!='
                break;
            addLine("<EqExp>");
            move();
            if (!checkRelExp()) {
                setIndex(startIndex);
                break;
            }
            startIndex = wordIndex;
        }
        addLine("<EqExp>");
        return true;
    }

    bool checkLAndExp() {
        if (!checkEqExp())
            return false;
        int startIndex = wordIndex;
        while (startIndex < totalWord) {
            if (!currentWord.checkType("AND")) //'&&'
                break;
            addLine("<LAndExp>");
            move();
            if (!checkEqExp()) {
                setIndex(startIndex);
                break;
            }
            startIndex = wordIndex;
        }
        addLine("<LAndExp>");
        return true;
    }

    bool checkLOrExp() {
        if (!checkLAndExp())
            return false;
        int startIndex = wordIndex;
        while (startIndex < totalWord) {
            if (!currentWord.checkType("OR")) //'&&'
                break;
            addLine("<LOrExp>");
            move();
            if (!checkLAndExp()) {
                setIndex(startIndex);
                break;
            }
            startIndex = wordIndex;
        }
        addLine("<LOrExp>");
        return true;
    }

    bool checkConstExp() {
        if (checkAddExp()) {
            addLine("<ConstExp>");
            return true;
        }
        return false;
    }

    bool checkIdent() {
        if (currentWord.checkType("IDENFR")) {
            move();
            return true;
        }
        return false;
    }

    bool checkIntConst() {
        if (currentWord.checkType("INTCON")) {
            move();
            return true;
        }
        return false;
    }

public:
    Grammar() = default;
    Grammar(string article) {
        lexical = Lexical(article);
        totalWord = lexical.totalWordCount();
        currentWord = lexical.getWord(0);
        bool state = checkCompUnit();
        if (!state) {
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
