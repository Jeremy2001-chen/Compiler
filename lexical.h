//
// Created by chenjiyuan3 on 2021/10/5.
//

#ifndef GRAMMAR_1005_LEXICAL_H
#define GRAMMAR_1005_LEXICAL_H

#include <map>
#include <vector>
#include "word.h"
using namespace std;

class Lexical{
private:
    map <string, string> typeTable{};
    string article{};
    vector<Word> words{};
    int index = 0;
    int line = 1;
    static bool checkVar(char c) {
        return isdigit(c) || isalpha(c) || c == '_';
    }
    static bool checkBlank(char c) {
        return isblank(c) || c == '\n' || (int)c == 13;
    }
    bool getToken() {
        while(index < article.size() && checkBlank(article[index])) {
            if (article[index] == '\n') {
                line += 1;
            }
            index ++;
        }
        if (index >= article.size()) return false;
        if (index < article.size() - 1) {
            if(article[index] == '/' && article[index + 1] == '*') {
                index += 2;
                while(index < article.size() - 1) {
                    if (article[index] == '*' && article[index + 1] == '/') {
                        index += 2;
                        return true;
                    } else if (article[index] == '\n') {
                        line += 1;
                    }
                    index += 1;
                }
                //cout << "error there is no */ " << endl;
                exit(1);
                //output.emplace_back(1, article[index]);
                //fout << int(article[index]) << endl;
                return false;
            }
            else if (article[index] == '/' && article[index + 1] == '/') {
                index += 2;
                while (index < article.size()) {
                    if (article[index] == '\n') {
                        index += 1;
                        line += 1;
                        return true;
                    }
                    index += 1;
                }
                //cout << "error // can't end" << endl;
                exit(1);
                //output.emplace_back(1, article[index]);
                //fout << int(article[index]) << endl;
                return false;
            }
        }
        if (!checkVar(article[index])) {
            if (article[index] == '\"') {
                string temp;
                temp = article[index ++];
                do {
                    temp += article[index ++];
                } while(index <= article.size() && article[index - 1] != '\"');
                words.emplace_back("STRCON", temp, line);
                //fout << "STRCON " << temp << endl;
                return true;
            }
            if (index < article.size() - 1) {
                string temp;
                temp += article[index];
                temp += article[index + 1];
                auto it = typeTable.find(temp);
                if (it != typeTable.end()) {
                    words.emplace_back(string(it->second), string(it->first), line);
                    //fout << it -> second << " " << it -> first << endl;
                    index += 2;
                    return true;
                }
            }
            string temp;
            temp += article[index];
            auto it = typeTable.find(temp);
            if (it != typeTable.end()) {
                words.emplace_back(string(it->second), string(it->first), line);
                //fout << it -> second << " " << it -> first << endl;
                index ++;
                return true;
            }
            //cout << "error had find on " << index << " !" << endl;
            exit(1);
            //output.emplace_back(1, article[index]);
            //fout << int(article[index]) << endl;
            return false;
        }
        string temp;
        while(index < article.size() && checkVar(article[index])) {
            temp += article[index ++];
        }
        auto it = typeTable.find(temp);
        if (it != typeTable.end()) {
            words.emplace_back(string(it->second), string(it->first), line);
            //fout << it -> second << " " << it -> first << endl;
            return true;
        }
        bool alphaAppear = false;
        for (auto c: temp) {
            if (isalpha(c) || c == '_') {
                alphaAppear = true;
                break;
            }
        }
        if (alphaAppear) {
            words.emplace_back("IDENFR", temp, line);
            //fout << "IDENFR " << temp << endl;
        } else {
            words.emplace_back("INTCON", temp, line);
            //fout << "INTCON " << temp << endl;
        }
        return true;
    }
    void initTable() {
        typeTable["main"] = "MAINTK";
        typeTable["const"] = "CONSTTK";
        typeTable["int"] = "INTTK";
        typeTable["break"] = "BREAKTK";
        typeTable["continue"] = "CONTINUETK";
        typeTable["if"] = "IFTK";
        typeTable["else"] = "ELSETK";
        typeTable["!"] = "NOT";
        typeTable["&&"] = "AND";
        typeTable["||"] = "OR";
        typeTable["while"] = "WHILETK";
        typeTable["getint"] = "GETINTTK";
        typeTable["printf"] = "PRINTFTK";
        typeTable["return"] = "RETURNTK";
        typeTable["+"] = "PLUS";
        typeTable["-"] = "MINU";
        typeTable["void"] = "VOIDTK";
        typeTable["*"] = "MULT";
        typeTable["/"] = "DIV";
        typeTable["%"] = "MOD";
        typeTable["<"] = "LSS";
        typeTable["<="] = "LEQ";
        typeTable[">"] = "GRE";
        typeTable[">="] = "GEQ";
        typeTable["=="] = "EQL";
        typeTable["!="] = "NEQ";
        typeTable["="] = "ASSIGN";
        typeTable[";"] = "SEMICN";
        typeTable[","] = "COMMA";
        typeTable["("] = "LPARENT";
        typeTable[")"] = "RPARENT";
        typeTable["["] = "LBRACK";
        typeTable["]"] = "RBRACK";
        typeTable["{"] = "LBRACE";
        typeTable["}"] = "RBRACE";
    }

public:
    Lexical() = default;
    explicit Lexical(string article) {
        this->article = std::move(article);
        index = 0;
        initTable();
        while(getToken());
    }

    string to_string() {
        string ret;
        for(Word word: words)
            ret += word.getTypeCode() + " " + word.getValue() + "\n";
        return ret.substr(0, ret.size()-1);
    }

    Word getWord(int id) {
        if (id >= (int)words.size()) {
            return {};
        }
        return words[id];
    }

    int totalWordCount() {
        return (int)words.size();
    }
};

#endif //GRAMMAR_1005_LEXICAL_H
