//
// Created by chenjiyuan3 on 2021/10/22.
//

#ifndef COMPILER_TYPE_H
#define COMPILER_TYPE_H

enum ClassType{
    NodeType,
    BinaryExpType,
    MulType,
    AddExpType,
    RelExpType,
    EqExpType,
    LAndExpType,
    LOrExpType,
    AssignExpType,
    BlockType,
    IfStmtType,
    WhileStmtType,
    BreakStmtType,
    ContinueStmtType,
    ReturnStmtType,
    PrintfStmtType,
    CompUnitType,
    FunRParamType,
    FunFParamType,
    FunFType,
    FunRType,
    SingleExpType,
    UnaryExpType,
    NumberType,
    VariableType,
    VariableDeclType,
    ReadValueType,
    DeclStmtType,
    NullStmtType
};

#endif //COMPILER_TYPE_H
