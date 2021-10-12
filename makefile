NAME = hymn
TEST_NAME = tests

SOURCE = $(wildcard src/*.c)
HEADERS = $(wildcard src/*.h)
OBJECTS = $(patsubst src/%.c,objects/%.o,$(SOURCE))
DEPENDENCY = $(patsubst %.o,%.d,$(OBJECTS))

TEST_SOURCE = $(wildcard test/*.c)
TEST_HEADERS = $(wildcard test/*.h)
TEST_OBJECTS = $(patsubst test/%.c,testing/%.o,$(TEST_SOURCE))
TEST_DEPENDENCY = $(patsubst %.o,%.d,$(TEST_OBJECTS))

INCLUDE = -Isrc

COMPILER_FLAGS = -Wall -Wextra -Werror -pedantic -Wno-unused-function -std=c11 $(INCLUDE)
LINKER_FLAGS =
LIBS = -lm
PREFIX =
CC = gcc

ifneq ($(shell uname), Linux)
	CC = clang
	COMPILER_FLAGS += -Wno-nullability-extension -Wno-deprecated-declarations
endif

.PHONY: all test analysis address valgrind clean

all: $(NAME)

test: COMPILER_FLAGS += -DHYMN_TESTING -g
test: $(TEST_NAME)

-include $(DEPENDENCY)
-include $(TEST_DEPENDENCY)

analysis: PREFIX = scan-build
analysis: all

address: COMPILER_FLAGS += -fsanitize=address
address: all

valgrind: COMPILER_FLAGS += -g
valgrind: all

$(NAME): $(HEADERS) $(OBJECTS)
	$(PREFIX) $(CC) $(OBJECTS) $(COMPILER_FLAGS) $(LINKER_FLAGS) -o $(NAME) $(LIBS)

$(TEST_NAME): $(HEADERS) $(OBJECTS) $(TEST_HEADERS) $(TEST_OBJECTS)
	$(PREFIX) $(CC) $(OBJECTS) $(TEST_OBJECTS) $(COMPILER_FLAGS) $(LINKER_FLAGS) -o $(TEST_NAME) $(LIBS)

objects/%.o: src/%.c
	@mkdir -p $(dir $@)
	$(CC) -c $< $(COMPILER_FLAGS) -MMD -o $@

testing/%.o: test/%.c
	@mkdir -p $(dir $@)
	$(CC) -c $< $(COMPILER_FLAGS) -MMD -o $@

clean:
	rm -f ./$(NAME)
	rm -f ./$(TEST_NAME)
	rm -rf ./objects
	rm -rf ./testing
