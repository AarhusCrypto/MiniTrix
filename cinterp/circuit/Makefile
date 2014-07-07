INCL=-I../deps/headers 

LIBS=-L../deps -lcminimacs -lcarena -lencoding -lmath -lds -losal -lpthread -lrt

TARGET=interp

all:
	bison --defines=y.tab.h grammar.y
	flex grammar.lex
	cc lex.yy.c grammar.tab.c ast.c interp.c -lfl $(INCL) $(LIBS) -o $(TARGET)

clean:
	rm -rf grammar.tab.c lex.yy.c interp y.tab.h