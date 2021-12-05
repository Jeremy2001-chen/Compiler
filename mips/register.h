#ifndef COMPILER_REGISTER_H
#define COMPILER_REGISTER_H

#include <map>

map <string, int> RMap;

#define REGISTER_COUNT 32

#define NOT_CHANGE (-1)
#define GLOBAL_VAR 0
#define TEMPORARY_VAR 1

string RegisterName[32] = {"$0", "$at", "$v0", "$v1", "$a0", "$a1", "$a2", "$a3", "$t0",
                           "$t1", "$t2", "$t3", "$t4", "$t5", "$t6", "$t7", "$s0", "$s1",
                           "$s2", "$s3", "$s4", "$s5", "$s6", "$s7", "$t8", "$t9", "$k0",
                           "$k1", "$gp", "$sp", "$fp", "$ra"};
int RegisterType[REGISTER_COUNT];
bool RegisterUsed[REGISTER_COUNT];
int RegisterTime[REGISTER_COUNT], TICK = 0;

class Register {
public:
    Register() {
        for (int i = 0; i < REGISTER_COUNT; ++ i) {
            RMap[RegisterName[i]] = i;
            RegisterType[i] = GLOBAL_VAR;
        }
        RegisterType[RMap["$t0"]] = TEMPORARY_VAR;
        RegisterType[RMap["$t1"]] = TEMPORARY_VAR;
        RegisterType[RMap["$t9"]] = TEMPORARY_VAR;
        RegisterType[RMap["$t8"]] = TEMPORARY_VAR;
        RegisterType[RMap["$sp"]] = NOT_CHANGE;
        RegisterType[RMap["$0"]] = NOT_CHANGE;
        RegisterType[RMap["$at"]] = NOT_CHANGE;
        RegisterType[RMap["$ra"]] = NOT_CHANGE;
        RegisterType[RMap["$v0"]] = NOT_CHANGE;
        RegisterType[RMap["$a0"]] = NOT_CHANGE;
        RegisterType[RMap["$a1"]] = NOT_CHANGE;
        RegisterType[RMap["$a2"]] = NOT_CHANGE;
        RegisterType[RMap["$a3"]] = NOT_CHANGE;
        RegisterType[RMap["$gp"]] = NOT_CHANGE;
    }

    static void clear() {
        for (bool & i : RegisterUsed)
            i = false;
        TICK = 0;
        for (int & i : RegisterTime)
            i = 0;
    }

    bool* getRegisterUsed() {
        return RegisterUsed;
    }

    int* getRegisterTime() {
        return RegisterTime;
    }

    void setUsed(bool* use) {
        for (int i = 0; i < 32; ++ i)
            RegisterUsed[i] = use[i];
    }

    void setTime(int* time) {
        for (int i = 0; i < 32; ++ i)
            RegisterTime[i] = time[i];
    }

    void release(const string& num) {
        int index = RMap[num];
        RegisterUsed[index] = false;
    }

    string alloc() {
        int pos = -1;
        for (int i = 0; i < REGISTER_COUNT; ++ i)
            if (RegisterType[i] == GLOBAL_VAR && !RegisterUsed[i]) {
                RegisterUsed[i] = true;
                RegisterTime[i] = ++ TICK;
                return RegisterName[i];
            } else if (RegisterType[i] == GLOBAL_VAR) {
                if (pos == -1 || RegisterTime[pos] > RegisterTime[i])
                    pos = i;
            }
        RegisterUsed[pos] = true;
        RegisterTime[pos] = ++ TICK;
        return RegisterName[pos];
    }
};

#endif //COMPILER_REGISTER_H
