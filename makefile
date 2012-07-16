NAME = ngxsbs
OBJS = error.o bindings.o scanner.o parser.o generate.o
TESTS = debug.o

ifeq ($(target),candidate)
   FLAGS = -Wall -O2 -g
   DEST=build-candidate/
else
ifeq ($(target),release)
   FLAGS = -Wall -O2
   DEST=build-release/
else
#ifeq ($(target),debug)
   FLAGS = -Wall -O0 -g
   DEST=build-debug/
endif
endif

ifeq ($(profile),yes)
   XFLAGS = -pg 
else
   XFLAGS =
endif

CC = gcc
CFLAGS = -pedantic -std=c99 $(FLAGS) $(XFLAGS)
LFLAGS = $(XFLAGS) 

PATHOBJS = $(foreach var,$(OBJS), $(DEST)$(var))
PATHTESTS = $(foreach var,$(TESTS), $(DEST)$(var))

all: build test

build: $(PATHOBJS)

test: $(PATHOBJS) $(PATHTESTS)
	@echo Compiled program: test-$(NAME)
	@mkdir -p $(DEST)
	@$(CC) $(LFLAGS) -o test-$(NAME) $^

clean: 
	@echo Removing temporary build files
	@rm -f $(OBJS) $(TESTS)

cleanall: 
	@echo Removing ALL temporary build files
	@rm -R build-debug/ 2>/dev/null || :
	@rm -R build-candidate/ 2>/dev/null || :
	@rm -R build-release/ 2>/dev/null || :

$(DEST)%.o: %.c
	@echo Compiling library: $< "->" $@
	@mkdir -p $(DEST)
	@$(CC) $(CFLAGS) -o $@ -c $<

