//
// Created by chenjiyuan3 on 2021/10/6.
//
#include <utility>

#ifndef GRAMMAR_1005_WORD_H
#define GRAMMAR_1005_WORD_H

#include "output.h"

using namespace std;
extern Output output;

class Word{
private:
    string typeCode;
    string value;
    int line;
public:
    Word() = default;

    Word(string _typeCode, string _value, int _line) {
        typeCode = std::move(_typeCode);
        value = std::move(_value);
        line = _line;
        //output.addLine(to_string());
    }

    Word(string _typeCode, int _value, int _line) {
        typeCode = std::move(_typeCode);
        value = ::to_string(_value);
        line = _line;
        //output.addLine(to_string());
    }

    Word(string _typeCode, char _c, int _line) {
        typeCode = std::move(_typeCode);
        value = string(1, _c);
        line = _line;
        //output.addLine(to_string());
    }

    string getTypeCode() {
        return typeCode;
    }

    string getValue() {
        return value;
    }

    int getLine() {
        return line;
    }

    string to_string() {
        return typeCode + " " + value;
    }

    bool checkType(const string& _type) {
        return _type == typeCode;
    }

    void pushToOutput(int index) {
        output.addLine(to_string(), index);
    }
};

#endif //GRAMMAR_1005_WORD_H
