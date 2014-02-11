INCL=-I../deps/osal/headers -I../deps/cminimacs/headers \
-I../deps/carena/headers -I../deps/ds/headers -I../deps/encoding/headers \
-I../deps/math/headers

LIBS=-L../deps/osal -L../deps/cminimacs \
-L../deps/carena -L../deps/ds -L../deps/encoding \
-L../deps/math -lcminimacs -lcarena -lencoding -lmath -lds -losal -lpthread

TARGET=interp

all:
	bison --defines=y.tab.h grammar.y
	flex grammar.lex
	cc lex.yy.c grammar.tab.c ast.c -lfl $(INCL) $(LIBS) -o $(TARGET)

clean:
	rm -rf grammar.tab.c lex.yy.c interp y.tab.h