//
// Created by chenjiyuan3 on 2021/10/6.
//

#ifndef GRAMMAR_1005_OUTPUT_H
#define GRAMMAR_1005_OUTPUT_H

#include <stack>
#include "error.h"
#include <algorithm>
using namespace std;

typedef pair<int, string> pis;
class Output{
private:
    vector<string> line;
    vector<Error*> errors;
    stack<pis> s;
public:
    Output() = default;
    void addLine(const string& _line) {
        line.push_back(_line);
        //cout << _line << endl;
    }
    void addError(Error* error) {
        errors.push_back(error);
    }
    void addLine(const string& _line, int index) {
        //cout << index << " " << _line << endl;
        s.push(make_pair(index, _line));
    }

    static bool cmp(Error* error1, Error* error2) {
        return error1->getErrorLine() < error2->getErrorLine();
    }

    string to_string() {
        // this is for grammar
        /*while (!s.empty()) {
            line.push_back(s.top().second);
            s.pop();
        }
        string ret;
        for (auto it = line.rbegin(); it != line.rend(); ++it)
            ret += (*it) + "\n";
        return ret.substr(0, ret.size()-1);*/
        // this is for error
        sort(errors.begin(), errors.end(), cmp);
        string ret;
        for (Error *error: errors) {
            ret = ret + error->display();
        }
        return ret;
    }

    void setIndex(int index) {
        //cout << "back to " << index << endl;
        while (!s.empty()) {
            pis top = s.top();
            if (top.first >= index)
                s.pop();
            else
                break;
        }
    }

    bool sameError(int line) {
        if ((int)errors.size() == 0) return false;
        Error* error = errors[(int)errors.size()-1];
        return error -> getErrorLine() == line;
    }
};

#endif //GRAMMAR_1005_OUTPUT_H
