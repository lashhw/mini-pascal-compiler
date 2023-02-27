#ifndef INFO_H
#define INFO_H

/* stderr */
#define REDEF_FUN "%d:%d: redefined function %s\n"
#define REDEF_ARG "%d:%d: redefined argument %s\n"
#define REDEF_VAR "%d:%d: redefined variable %s\n"
#define UNDEC_VAR "%d:%d: undeclared variable %s\n"
#define UNDEC_FUN "%d:%d: undeclared function %s\n"
#define ARITH_TYPE "%d:%d: type errors in arithmetic expressions %s\n"
#define ASSIG_TYPE "%d:%d: type errors on assignment statement\n"
#define INDEX_TYPE "%d:%d: array indexing must be integer\n"
#define INDEX_MANY "%d:%d: too many subscripten on %s\n"
#define WRONG_ARGS "%d:%d: arguments' types and numbers of %s are wrong\n"
#define RETURN_VAL "%d:%d: missing return value of function %s\n"

/* stdout */
#define SHOW_NEWSYM(sym) printf("add new symbol %s\n", sym)
#define SHOW_NEWSCP() puts("create a scope")
#define SHOW_CLSSCP() puts("close a scope")

#define SHOW_SYMTAB_HEAD() \
  fputs( \
      "-----------------------------------------------------------------------\n" \
      "|    Name    | Scope |                   Type                         |\n" \
      "-----------------------------------------------------------------------\n", stdout)

#define SYMTAB_ENTRY_FMT "|%-12s|   %-4d|%-48s|\n"

#define SHOW_SYMTAB_TAIL() \
  fputs( \
      "-----------------------------------------------------------------------\n", stdout)

#endif
