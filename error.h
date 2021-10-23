#include <utility>

//
// Created by chenjiyuan3 on 2021/10/19.
//

#ifndef GRAMMAR_1005_ERROR_H
#define GRAMMAR_1005_ERROR_H

using namespace std;
class Error{
protected:
    int error_line;
public:
    virtual string display() = 0;
    virtual string debug() = 0;
    int getErrorLine() const {
        return error_line;
    }
};

class IllegalCharacterError : public Error {
public:
    explicit IllegalCharacterError(int line) {
        this->error_line = line;
    }
    string display() override {
        return (to_string(this->error_line) + " a\n");
    }
    string debug() override {
        return "[Debug]Error! At line: " + to_string(this->error_line) + " , the format string has illegal character!\n";
    }
};

class NameRedefineError : public Error {
private:
    string str;
public:
    NameRedefineError(int line, string _str) {
        this->error_line = line;
        this->str = std::move(_str);
    }
    string display() override {
        return to_string(this->error_line) + " b\n";
    }
    string debug() override {
        return "[Debug]Error! At line: " + to_string(this->error_line) + " , the name " +
                str + " define again!\n";
    }
};

class UndefineNameError: public Error {
private:
    string str;
public:
    UndefineNameError(int line, string _str) {
        this->error_line = line;
        this->str = _str;
    }
    string display() override {
        return to_string(this->error_line) + " c\n";
    }
    string debug() override {
        return "[Debug]Error! At line: " + to_string(this->error_line) + " , the name " +
                str + " has not defined!\n";
    }
};

class NotMatchParameterNumError: public Error {
private:
    string funName;
    int expectNum, realNum;
public:
    NotMatchParameterNumError(int line, string _funName, int _expectNum, int _realNum) {
        this->error_line = line;
        this->funName = _funName;
        expectNum = _expectNum;
        realNum = _realNum;
    }
    string display() override {
        return to_string(this->error_line) + " d\n";
    }
    string debug() override {
        return "[Debug]Error! At line: " + to_string(this->error_line) + " , the function " +
        funName + " expect " + to_string(expectNum) +
        " parameter, but you have " + to_string(realNum) + " parameter!\n";
    }
};

class NotMatchParameterTypeError: public Error {
private:
    string funName;
    string expectType, realType;
public:
    NotMatchParameterTypeError(int line, string _funName, string _expectType, string _readType) {
        this->error_line = line;
        this->expectType = _expectType;
        this->realType = _readType;
        this->funName = _funName;
    }
    string display() override {
        return to_string(this->error_line) + " e\n";
    }
    string debug() override {
        return "[Debug]Error! At line: " + to_string(this->error_line) + " , the function " +
        funName + " expect " + expectType + " type parameter, but you have " +
        realType + " parameter!\n";
    }
};

class NotMatchReturnError: public Error {
private:
    string funName;
public:
    NotMatchReturnError(int line, string _funName) {
        this->error_line = line;
        this->funName = _funName;
    }
    string display() override {
        return to_string(this->error_line) + " f\n";
    }
    string debug() override {
        return "[Debug]Error! At line: " + to_string(this->error_line) + " , the function " +
        funName + " has the incorrect return statement!\n";
    }
};

class NoReturnError: public Error {
private:
    string funName;
public:
    NoReturnError(int line, string _funName) {
        this->error_line = line;
        this->funName = _funName;
    }
    string display() override {
        return to_string(this->error_line) + " " + "g\n";
    }
    string debug() override {
        return "[Debug]Error! At line: " + to_string(this->error_line) + " , the function " +
        funName + " has no return statement!\n";
    }
};

class ConstVariableChangeError: public Error {
private:
    string varName;
public:
    ConstVariableChangeError(int line, string _varName) {
        this->error_line = line;
        this->varName = _varName;
    }
    string display() override {
        return to_string(this->error_line) + " " + "h\n";
    }
    string debug() override {
        return "[Debug]Error! At line: " + to_string(this->error_line) + " , the const variable " +
        varName + "can't change!\n";
    }
};

class NoSemicolonError: public Error {
public:
    NoSemicolonError(int line) {
        this->error_line = line;
    }
    string display() override {
        return to_string(this->error_line) + " " + "i\n";
    }
    string debug() override {
        return "[Debug]Error! At line: " + to_string(this->error_line) + " , need ';' at end!\n";
    }
};

class NoRightParenthesesError: public Error {
public:
    NoRightParenthesesError(int line) {
        this->error_line = line;
    }
    string display() override {
        return to_string(this->error_line) + " " + "j\n";
    }
    string debug() override {
        return "[Debug]Error! At line: " + to_string(this->error_line) + " , need ')' at end!\n";
    }
};

class NoRightBracketsError: public Error {
public:
    NoRightBracketsError(int line) {
        this->error_line = line;
    }
    string display() override {
        return to_string(this->error_line) + " " + "k\n";
    }
    string debug() override {
        return "[Debug]Error! At line: " + to_string(this->error_line) + " , need ']' at end!\n";
    }
};

class PrintParameterNumError: public Error {
private:
    int expectNum, realNum;
public:
    PrintParameterNumError(int line, int _expectNum, int _realNum) {
        this->error_line = line;
        expectNum = _expectNum;
        realNum = _realNum;
    }
    string display() override {
        return to_string(this->error_line) + " " + "l\n";
    }
    string debug() override {
        return "[Debug]Error! At line: " + to_string(this->error_line) + " , printf need " + to_string(expectNum)
        + " parameter but you have " + to_string(realNum) + " parameter!\n";
    }
};

class BreakContinueError: public Error {
private:
    string errorType;
public:
    BreakContinueError(int line, string _errorType) {
        this->error_line = line;
        this->errorType = _errorType;
    }
    string display() override {
        return to_string(this->error_line) + " " + "m\n";
    }
    string debug() override {
        return "[Debug]Error! At line: " + to_string(this->error_line) + " , " +
        errorType + " should not appear outside the while block!\n";
    }
};
#endif //GRAMMAR_1005_ERROR_H
