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
    virtual void display();
    virtual void debug();
};



class IllegalCharacterError : public Error {
public:
    IllegalCharacterError(int line) {
        this->error_line = line;
    }
    void display() override {
        cout << this->error_line << " " << "a" << endl;
    }
    void debug() override {
        cout << "[Debug]Error! At line: " << this->error_line << " , the format string has illegal character!" << endl;
    }
};

class NameRedefineError : public Error {
private:
    string str;
public:
    NameRedefineError(int line, string _str) {
        this->error_line = line;
        this->str = _str;
    }
    void display() override {
        cout << this->error_line << " " << "b" << endl;
    }
    void debug() override {
        cout << "[Debug]Error! At line: " << this->error_line << " , the name " << str << " define again!" << endl;
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
    void display() override {
        cout << this->error_line << " " << "c" << endl;
    }
    void debug() override {
        cout << "[Debug]Error! At line: " << this->error_line << " , the name " << str << " has not defined!" << endl;
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
    void display() override {
        cout << this->error_line << " " << "d" << endl;
    }
    void debug() override {
        cout << "[Debug]Error! At line: " << this->error_line << " , the function " <<
        funName << " expect " << expectNum <<
        " parameter, but you have " << realNum << " parameter!" << endl;
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
    }
    void display() override {
        cout << this->error_line << " " << "e" << endl;
    }
    void debug() override {
        cout << "[Debug]Error! At line: " << this->error_line << " , the function " <<
        funName << " expect " << expectType << " type parameter, but you have " <<
        realType << " parameter!" << endl;
    }
};

class HasReturnError: public Error {
private:
    string funName;
public:
    HasReturnError(int line, string _funName) {
        this->error_line = line;
        this->funName = _funName;
    }
    void display() override {
        cout << this->error_line << " " << "f" << endl;
    }
    void debug() override {
        cout << "[Debug]Error! At line: " << this->error_line << " , the function " <<
        funName << " has return statement!" << endl;
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
    void display() override {
        cout << this->error_line << " " << "g" << endl;
    }
    void debug() override {
        cout << "[Debug]Error! At line: " << this->error_line << " , the function " <<
        funName << " no return statement!" << endl;
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
    void display() override {
        cout << this->error_line << " " << "h" << endl;
    }
    void debug() override {
        cout << "[Debug]Error! At line: " << this->error_line << " , the const variable " <<
        varName << "can't change!" << endl;
    }
};

class NoSemicolonError: public Error {
public:
    NoSemicolonError(int line) {
        this->error_line = line;
    }
    void display() override {
        cout << this->error_line << " " << "i" << endl;
    }
    void debug() override {
        cout << "[Debug]Error! At line: " << this->error_line << " , need ';' at end!" << endl;
    }
};

class NoRightParenthesesError: public Error {
public:
    NoRightParenthesesError(int line) {
        this->error_line = line;
    }
    void display() override {
        cout << this->error_line << " " << "j" << endl;
    }
    void debug() override {
        cout << "[Debug]Error! At line: " << this->error_line << " , need ')' at end!" << endl;
    }
};

class NoRightBracketsError: public Error {
public:
    NoRightBracketsError(int line) {
        this->error_line = line;
    }
    void display() override {
        cout << this->error_line << " " << "k" << endl;
    }
    void debug() override {
        cout << "[Debug]Error! At line: " << this->error_line << " , need ']' at end!" << endl;
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
    void display() override {
        cout << this->error_line << " " << "l" << endl;
    }
    void debug() override {
        cout << "[Debug]Error! At line: " << this->error_line << " , printf need " << expectNum
        << " parameter but you have " << realNum << " parameter!" << endl;
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
    void display() override {
        cout << this->error_line << " " << "m" << endl;
    }
    void debug() override {
        cout << "[Debug]Error! At line: " << this->error_line << " , " <<
        errorType << " should not appear outside the while block!" << endl;
    }
};
#endif //GRAMMAR_1005_ERROR_H
