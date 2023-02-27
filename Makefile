SCANNER = scanner
PARSER  = parser
CC      = g++
CFLAGS  = -Iinclude -Wall --std=c++14 -g -DNDEBUG
LEX     = flex
YACC    = bison

EXEC    = compiler
SRCS    = $(PARSER) $(SCANNER)
OBJS    = $(OBJDIR)/$(SCANNER).c $(OBJDIR)/$(PARSER).c
OBJDIR  = obj
TESTDIR = testcases
ASM     = fibonacci_recursive.j qsort.j test1.j

all: $(OBJDIR) $(EXEC)

$(OBJDIR)/$(SCANNER).c: $(SCANNER).l
	$(LEX) -o $@ $<

$(OBJDIR)/$(PARSER).c: $(PARSER).y
	$(YACC) -o $@ --defines=$(OBJDIR)/parser.h -v $<

$(OBJDIR):
	mkdir -p $(OBJDIR)

$(EXEC): $(OBJS)
	$(CC) $(CFLAGS) -o $@ $^
	
gen: $(ASM)
	
%.j: $(TESTDIR)/%.p
	./$(EXEC) $< -o $@
