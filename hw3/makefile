CC       = gcc
CFLAGS   = -Wall -Wextra -pedantic -Werror 
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

