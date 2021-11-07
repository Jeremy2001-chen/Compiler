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

enum IrType{
    IrBinaryOpType,
    IrUnaryOpType,
    IrLabelLineType,
    IrFunDefineType,
    IrParaDefineType,
    IrPushVariableType,
    IrPushArrayType,
    IrCallFunctionType,
    IrReturnStmtType,
    IrReturnValStmtType,
    IrVarDefineWithAssignType,
    IrVarDefineWithOutAssignType,
    IrCmpStmtType,
    IrBranchStmtType,
    IrGotoStmtType,
    IrArrayDefineWithOutAssignType,
    IrArrayDefineWithAssignType,
    IrArrayAssignType,
    IrArrayGetType,
    IrReadIntegerType,
    IrPrintIntegerType,
    IrPrintStringType,
    IrNumberAssignType,
    IrExitType
};
#endif //COMPILER_TYPE_H
