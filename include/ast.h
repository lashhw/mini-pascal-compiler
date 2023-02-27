#ifndef AST_H
#define AST_H

#include <array>
#include "loc.h"

enum class NodeType { 
  PROG, 
  ID_LIST,
  DECL_LIST,
  TYPE,
  SUBPROG_DECL_LIST,
  SUBPROG_HEAD, 
  PARAM_LIST,
  STMT_LIST,
  ASSIGN,
  IF,
  WHILE,
  VAR,
  EXPR_LIST,
  PROCEDURE,
  OP,
  NOT,
  NEGATE,
  LITERAL_INT,
  LITERAL_DBL,
  LITERAL_STR
};

enum class IDType { INT, REAL, STRING, VOID, ARRAY, SUBPROG };

enum class OpType {
  AND, OR,
  LT, GT, EQ, LET, GET, NEQ,
  ADD, SUB, MUL, DIV
};

struct Node {
  NodeType node_type;
  LocType loc;
  std::array<Node*, 4> child;
  Node *next;
  union {
    int ival;
    double dval;
    char *sval;
    IDType tval;
    OpType oval;
  } metadata;
};

#endif
