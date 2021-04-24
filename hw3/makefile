CC       = gcc
CFLAGS   = -Wall -Wextra -pedantic -Werror  -Wfloat-equal -Wundef -Wshadow -Wpointer-arith -Wcast-align  -Wstrict-overflow=5 -Wwrite-strings -Waggregate-return
CFLAGS += -Wcast-qual -Wswitch-default -Wswitch-enum  -Wunreachable-code -O2 -g
LDFLAGS  =  -lpthread -lrt  
SRC := $(wildcard *.c)
OBJFILES:=$(patsubst %c,%o,$(SRC))
TARGET   = out/main
all: $(TARGET) clean print-
$(TARGET): $(OBJFILES)
	$(CC)  -o $(TARGET) $(OBJFILES) $(CFLAGS) $(LDFLAGS)

clean:
	rm -f $(OBJFILES)
print-:
	@echo 'build has finished'

