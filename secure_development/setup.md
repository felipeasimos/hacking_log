# Setup

While programming, it is important to be aware of good pratices in order
to avoid security issues down the line. To make this process easier,
we will setup an environment for a C/C++ project with:

* __Valgrind__ - to notify us of memory-related errors.

* __CppCheck__ - to notify us of any security guidelines we are not following.

* __GoogleTest__ - to write and run tests.

* __Gcov__ - to ensure our tests cover the entire code.

## Project

### Structure

The first thing we have to set is the project's folder structure. The 
following is a good structure for a medium-sized C/C++ project:

```
|-- Makefile
|-- README.md
|-- build
|   |-- app
|   |-- objects
|   `-- tests
|-- include
|   |-- header_with_access_to_all_others.h
|   |-- module1/
|   |-- module2/
|   `-- module3/
|-- src
|   |-- module1/
|   |-- module2
|   `-- module3/
`-- tests
    |-- module1/
    |-- module2/
    `-- module3/
```

* __`src/`__ - where we store `.c`/`.cpp` files for each module.

* __`include/`__ - where we store `.h`/`.hpp` files for each module.

* __`tests/`__ - where we store tests files for each module (will always
be `.cpp` files, since googletest is written in C++).

* __`src/`__ - where we store `.c`/`.cpp` files for each module.

* __`build/`__ - where our objects and executables will be stored after
compilation.

* __`build/app/`__ - where our executable/library will be stored.

### Content

As an example, this project could produce a single-linked list. It would
have two modules: a `data/` module, where we will deal with storing data in a
`void*` pointer, and a `list/` folder, where will implement the list
using the `data/` module.

Then the structure would look something like this:

```
|-- Makefile
|-- README.md
|-- build
|   |-- app
|   |   `-- libds.so
|   |-- objects
|   |   |-- data
|   |   |   `-- data.o
|   |   `-- list_simple
|   |       `-- list_simple.o
|   `-- tests
|       |-- data
|       |   `-- data.o
|       `-- list_simple
|           `-- list_simple.o
|-- include
|   |-- data
|   |   `-- data.h
|   |-- ds.h
|   `-- list_simple
|       `-- list_simple.h
|-- runner
|-- src
|   |-- data
|   |   `-- data.c
|   `-- list_simple
|       `-- list_simple.c
`-- tests
    |-- data
    |   `-- data.cpp
    `-- list_simple
        `-- list_simple.cpp
```

The `structs` and function declarations are in the header files, this
allow any dependency between the modules to be dealt with by the
compiler.


## Makefile

First, we will write an `Makefile` that compile our modules into objects
that later are compiled together to produce the desired executable/library.

We can then add the relevant tools to our `Makefile`, integrating it to
our development cycle.

We will build upon [this `Makefile`](https://www.partow.net/programming/makefile/index.html)

If you have some doubts about how this works, check
[here for how Makefiles in general work](https://makefiletutorial.com/) and
[here for how some of the tricks inside `$()` used here work](https://www.gnu.org/software/make/manual/html_node/Substitution-Refs.html)

```
# folders
ROOT_DIR :=.
SRC_DIR := $(ROOT_DIR)/src
INCLUDE_DIR := $(ROOT_DIR)/include
BUILD_DIR := $(ROOT_DIR)/build
OBJ_DIR := $(BUILD_DIR)/objects
APP_DIR := $(BUILD_DIR)/app
TEST_OBJ_DIR := $(BUILD_DIR)/tests

# targets and pre-requisites
# change *.c to *.cpp for C++ projects
SRC := $(wildcard $(SRC_DIR)/*/*.c )
# change %.c to %.cpp for C++ projects
OBJECTS := $(SRC:$(SRC_DIR)/%.c=$(OBJ_DIR)/%.o)

TARGET := libds.so

# libraries (without -l)
LIBS :=

# compiler settings
CXX := g++ -std=c++17
CXXFLAGS := -pedantic-errors -Wall -Wextra -Werror -fPIC
LDFLAGS := $(addprefix -l,$(LIBS))
INCLUDE := -I$(INCLUDE_DIR)

# final preparations (don't change this)
TARGET_FINAL := $(APP_DIR)/$(TARGET)

.PHONY: all folders clean debug release test

release: CXXFLAGS += -O3
release: all 

all: folders $(TARGET_FINAL)

debug: CXXFLAGS += -DDEBUG -g
debug: | test all

folders:
	@mkdir -p $(APP_DIR)
	@mkdir -p $(OBJ_DIR)
	@mkdir -p $(TEST_OBJ_DIR)

clean:
	-@rm -rvf $(OBJ_DIR)/*
	-@rm -rvf $(APP_DIR)/*
	-@rm -rvf $(TEST_OBJ_DIR)/*

$(TARGET_FINAL): $(OBJECTS)
	@mkdir -p $(@D)
	$(CXX) $(CXXFLAGS) -o $(TARGET_FINAL) $^ $(LDFLAGS) -shared -fPIC

# change $(SRC_DIR)/%.c to $(SRC_DIR)/%.cpp for C++ projects
$(OBJECTS): $(OBJ_DIR)/%.o: $(SRC_DIR)/%.c
	@mkdir -p $(@D)
	$(CXX) $(CXXFLAGS) $(INCLUDE) -c $< -o $@
```

Some important things to note here: we are calling `g++` instead of `gcc`.

Even though we are going to write C code, googletest, which we will add
shortly, is written in C++. To compile C code as you would with `gcc` in
a C++ compiler, just put it inside a
[`extern "C"` block](https://stackoverflow.com/questions/1041866/what-is-the-effect-of-extern-c-in-c).

## Adding tools

### Googletest

The first tool we will add is googletest. With it we can write tests to
make sure our code is behaving as expected. You can follow instructions
on how to install and use googletest from their
[README.md in github](https://github.com/google/googletest) or get a
quick explanation and installation procedure from
[one of my notes](googletest.md).

We will add some lines to our `Makefile`, in order to compile the tests
and run them if `make debug` is called.

The idea of this `Makefile` is that if someone clones a repository with it,
the only step to compile everything is to run `make`, which will only
produce the final binary, with no tests and thus no need to have
googletest installed:

```
# folders
ROOT_DIR :=.
SRC_DIR := $(ROOT_DIR)/src
INCLUDE_DIR := $(ROOT_DIR)/include
BUILD_DIR := $(ROOT_DIR)/build
OBJ_DIR := $(BUILD_DIR)/objects
APP_DIR := $(BUILD_DIR)/app
TEST_OBJ_DIR := $(BUILD_DIR)/tests
TEST_SRC_DIR := $(ROOT_DIR)/tests

# targets and pre-requisites
# change *.c to *.cpp for C++ projects
SRC := $(wildcard $(SRC_DIR)/*/*.c )
# change %.c to %.cpp for C++ projects
OBJECTS := $(SRC:$(SRC_DIR)/%.c=$(OBJ_DIR)/%.o)

TESTS_SRC := $(wildcard $(TEST_SRC_DIR)/*/*.cpp)
TESTS_OBJ := $(TESTS_SRC:$(TEST_SRC_DIR)/%.cpp=$(TEST_OBJ_DIR)/%.o)

TARGET := libds.so
TEST_TARGET := runner

# libraries (without -l)
LIBS :=
TEST_LIBS := gtest gtest_main pthread

# compiler settings
CXX := g++ -std=c++17
CXXFLAGS := -pedantic-errors -Wall -Wextra -Werror -fPIC
LDFLAGS := $(addprefix -l,$(LIBS))
INCLUDE := -I$(INCLUDE_DIR)

TEST_LIBS := $(addprefix -l,$(TEST_LIBS))

# final preparations (don't change this)
TARGET_FINAL := $(APP_DIR)/$(TARGET)
TEST_TARGET_FINAL := $(ROOT_DIR)/$(TEST_TARGET)

.PHONY: all folders clean debug release test

release: CXXFLAGS += -O3
release: all 

all: folders $(TARGET_FINAL)

debug: CXXFLAGS += -DDEBUG -g
debug: | $(TESTS_OBJ) test all

test: LDFLAGS += $(TEST_LIBS)
test: $(TESTS_OBJ) $(OBJECTS)
	$(CXX) $(CXXFLAGS) $(INCLUDE) -o $(TEST_TARGET_FINAL) $^ $(LDFLAGS)
	$(TEST_TARGET_FINAL)

folders:
	@mkdir -p $(APP_DIR)
	@mkdir -p $(OBJ_DIR)
	@mkdir -p $(TEST_OBJ_DIR)

clean:
	-@rm -rvf $(OBJ_DIR)/*
	-@rm -rvf $(APP_DIR)/*
	-@rm -rvf $(TEST_OBJ_DIR)/*
	-@rm -vf $(TEST_TARGET_FINAL)

$(TARGET_FINAL): $(OBJECTS)
	@mkdir -p $(@D)
	$(CXX) $(CXXFLAGS) -o $(TARGET_FINAL) $^ $(LDFLAGS) -shared -fPIC

# change $(SRC_DIR)/%.c to $(SRC_DIR)/%.cpp for C++ projects
$(OBJECTS): $(OBJ_DIR)/%.o: $(SRC_DIR)/%.c
	@mkdir -p $(@D)
	$(CXX) $(CXXFLAGS) $(INCLUDE) -c $< -o $@

$(TESTS_OBJ): $(OBJECTS)
$(TESTS_OBJ): $(TEST_OBJ_DIR)/%.o: $(TEST_SRC_DIR)/%.cpp
	@mkdir -p $(@D)
	$(CXX) $(CXXFLAGS) $(INCLUDE) -c $< -o $@
```

Remember: googletest enable us to write and check our tests, but
how confident we can be that our tests are good depend on us.

### Valgrind

Before installing valgrind, mind that using `sudo apt install valgrind`
may install an old version which produce false positives (at least in
debian, other distributions may install more up-to-date versions).

I personally recommend avoiding the trouble and downloding and compiling
the source from [their site](https://www.valgrind.org/downloads/current.html).

After that, adding the tool is pretty simple, we just have to run our
tests using it. However, we give it some arguments to make sure
`valgrind` give us a lot of details and also stops at the first error
(`--exit-on-first-error=yes`).

We set an non-zero error exit code (`--error-exitcode=1`) so the
`Makefile` stops executing when `valgrind` finds an error.

The idea is to compile with `make debug`, read the only error
that may appear, fix it and do it all again.

The cherry on top is the `-q` option. This means that valgrind
will output nothing if no error occured:

```
# folders
ROOT_DIR :=.
SRC_DIR := $(ROOT_DIR)/src
INCLUDE_DIR := $(ROOT_DIR)/include
BUILD_DIR := $(ROOT_DIR)/build
OBJ_DIR := $(BUILD_DIR)/objects
APP_DIR := $(BUILD_DIR)/app
TEST_OBJ_DIR := $(BUILD_DIR)/tests
TEST_SRC_DIR := $(ROOT_DIR)/tests

# targets and pre-requisites
# change *.c to *.cpp for C++ projects
SRC := $(wildcard $(SRC_DIR)/*/*.c )
# change %.c to %.cpp for C++ projects
OBJECTS := $(SRC:$(SRC_DIR)/%.c=$(OBJ_DIR)/%.o)

TESTS_SRC := $(wildcard $(TEST_SRC_DIR)/*/*.cpp)
TESTS_OBJ := $(TESTS_SRC:$(TEST_SRC_DIR)/%.cpp=$(TEST_OBJ_DIR)/%.o)

TARGET := libds.so
TEST_TARGET := runner

# libraries (without -l)
LIBS :=
TEST_LIBS := gtest gtest_main pthread

# compiler settings
CXX := g++ -std=c++17
CXXFLAGS := -pedantic-errors -Wall -Wextra -Werror -fPIC
LDFLAGS := $(addprefix -l,$(LIBS))
INCLUDE := -I$(INCLUDE_DIR)

TEST_LIBS := $(addprefix -l,$(TEST_LIBS))

# final preparations (don't change this)
TARGET_FINAL := $(APP_DIR)/$(TARGET)
TEST_TARGET_FINAL := $(ROOT_DIR)/$(TEST_TARGET)

.PHONY: all folders clean debug release test

release: CXXFLAGS += -O3
release: all 

all: folders $(TARGET_FINAL)

debug: CXXFLAGS += -DDEBUG -g
debug: | $(TESTS_OBJ) test all

test: LDFLAGS += $(TEST_LIBS)
test: $(TESTS_OBJ) $(OBJECTS)
	$(CXX) $(CXXFLAGS) $(INCLUDE) -o $(TEST_TARGET_FINAL) $^ $(LDFLAGS)
	@valgrind -q --exit-on-first-error=yes --error-exitcode=1\
	--tool=memcheck --show-reachable=yes --leak-check=yes --track-origins=yes $(TEST_TARGET_FINAL)

folders:
	@mkdir -p $(APP_DIR)
	@mkdir -p $(OBJ_DIR)
	@mkdir -p $(TEST_OBJ_DIR)

clean:
	-@rm -rvf $(OBJ_DIR)/*
	-@rm -rvf $(APP_DIR)/*
	-@rm -rvf $(TEST_OBJ_DIR)/*
	-@rm -vf $(TEST_TARGET_FINAL)

$(TARGET_FINAL): $(OBJECTS)
	@mkdir -p $(@D)
	$(CXX) $(CXXFLAGS) -o $(TARGET_FINAL) $^ $(LDFLAGS) -shared -fPIC

# change $(SRC_DIR)/%.c to $(SRC_DIR)/%.cpp for C++ projects
$(OBJECTS): $(OBJ_DIR)/%.o: $(SRC_DIR)/%.c
	@mkdir -p $(@D)
	$(CXX) $(CXXFLAGS) $(INCLUDE) -c $< -o $@

$(TESTS_OBJ): $(OBJECTS)
$(TESTS_OBJ): $(TEST_OBJ_DIR)/%.o: $(TEST_SRC_DIR)/%.cpp
	@mkdir -p $(@D)
	$(CXX) $(CXXFLAGS) $(INCLUDE) -c $< -o $@
```

In the end, we only change one line of code from the previous `Makefile`.

### CppCheck

`cppcheck` is a static analysis tool. This means that unlike `valgrind`
that runs your program to analyse it, `cppcheck` only reads your source.

We give `cppcheck` all the source files: test `.c` files, `.c` files and
headers.

Since we won't give the system header files to it (like `stdio.h`) we add
`--suppress=missingIncludeSystem` 

We add `--quiet` to make `cppcheck` output nothing if no errors were found
(just like in `valgrind`).

The `--addon` arguments tell `cppcheck` to do extra checks. Mind the
`--addon=cert`, it will check our code against the CERT C guidelines.

The rest of the arguments just tell `cppcheck` to be thorough and take
longer in the analysis if necessary:

```
# folders
ROOT_DIR :=.
SRC_DIR := $(ROOT_DIR)/src
INCLUDE_DIR := $(ROOT_DIR)/include
BUILD_DIR := $(ROOT_DIR)/build
OBJ_DIR := $(BUILD_DIR)/objects
APP_DIR := $(BUILD_DIR)/app
TEST_OBJ_DIR := $(BUILD_DIR)/tests
TEST_SRC_DIR := $(ROOT_DIR)/tests

# targets and pre-requisites
SRC := $(wildcard $(SRC_DIR)/*/*.c )
OBJECTS := $(SRC:$(SRC_DIR)/%.c=$(OBJ_DIR)/%.o)

TESTS_SRC := $(wildcard $(TEST_SRC_DIR)/*/*.cpp)
TESTS_OBJ := $(TESTS_SRC:$(TEST_SRC_DIR)/%.cpp=$(TEST_OBJ_DIR)/%.o)

TARGET := libds.so
TEST_TARGET := runner

# libraries (without -l)
LIBS :=
TEST_LIBS := gtest gtest_main pthread

# compiler settings
CXX := g++ -std=c++17
CXXFLAGS := -pedantic-errors -Wall -Wextra -Werror -fPIC
LDFLAGS := $(addprefix -l,$(LIBS))
INCLUDE := -I$(INCLUDE_DIR)

TEST_LIBS := $(addprefix -l,$(TEST_LIBS))

# final preparations (don't change this)
TARGET_FINAL := $(APP_DIR)/$(TARGET)
TEST_TARGET_FINAL := $(ROOT_DIR)/$(TEST_TARGET)

# tests, static analysis and code coverage
RUN_TESTS_COMMAND:=@valgrind -q --exit-on-first-error=yes --error-exitcode=1 --tool=memcheck\
		--show-reachable=yes --leak-check=yes --track-origins=yes $(TEST_TARGET_FINAL)
STATIC_ANALYSIS_COMMAND:=@cppcheck --addon=cert --addon=y2038 --addon=threadsafety --addon=naming \
	$(INCLUDE) --suppress=missingIncludeSystem --bug-hunting --quiet --enable=all $(SRCS) $(TESTS_SRC)


.PHONY: all folders clean debug release test

release: CXXFLAGS += -O3
release: all 

all: folders $(TARGET_FINAL)

debug: CXXFLAGS += -DDEBUG -g
debug: | $(TESTS_OBJ) test all

test: LDFLAGS += $(TEST_LIBS)
test: $(TESTS_OBJ) $(OBJECTS)
	$(CXX) $(CXXFLAGS) $(INCLUDE) -o $(TEST_TARGET_FINAL) $^ $(LDFLAGS)	
	$(RUN_TESTS_COMMAND)
	$(STATIC_ANALYSIS_COMMAND)

folders:
	@mkdir -p $(APP_DIR)
	@mkdir -p $(OBJ_DIR)
	@mkdir -p $(TEST_OBJ_DIR)

clean:
	-@rm -rvf $(OBJ_DIR)/*
	-@rm -rvf $(APP_DIR)/*
	-@rm -rvf $(TEST_OBJ_DIR)/*
	-@rm -vf $(TEST_TARGET_FINAL)

$(TARGET_FINAL): $(OBJECTS)
	@mkdir -p $(@D)
	$(CXX) $(CXXFLAGS) -o $(TARGET_FINAL) $^ $(LDFLAGS) -shared -fPIC

$(OBJECTS): $(OBJ_DIR)/%.o: $(SRC_DIR)/%.c
	@mkdir -p $(@D)
	$(CXX) $(CXXFLAGS) $(INCLUDE) -c $< -o $@

$(TESTS_OBJ): $(OBJECTS)
$(TESTS_OBJ): $(TEST_OBJ_DIR)/%.o: $(TEST_SRC_DIR)/%.cpp
	@mkdir -p $(@D)
	$(CXX) $(CXXFLAGS) $(INCLUDE) -c $< -o $@

```

### Gcov

If you are testing your software properly, you should be testing all of it.
Although the percentage of lines tested is not a good metric to say if the
code is well tested, it is certain that untested code is not good. With `gcov`
we can check if our tests cover all the code.

Mind that a 100% code coverage does not mean your code is good, secure and 
ready for production. In fact, if your tests only pass through each line
of code once, it probably means they are not good enough, since you should
test many input/output scenarios.

To use `gcov`, we need to compile our code with `--coverage` and then
run `gcov <object files>`

The other arguments given tells `gcov` to don't output to a file (`-n`),
only show results for the files given as argument only (`-r`) and demangle
C++ names (`-m`, we are not going to use C++ though).

The process will output some files related to `gcov` in the same directory
as our objects files. We can just ignore them.

In the following file, we also use `grep`, `paste` to ignore any output from
`gcov` that shows a file with 100% code coverage, since we are looking
for files with untested code:

```
# folders
ROOT_DIR :=.
SRC_DIR := $(ROOT_DIR)/src
INCLUDE_DIR := $(ROOT_DIR)/include
BUILD_DIR := $(ROOT_DIR)/build
OBJ_DIR := $(BUILD_DIR)/objects
APP_DIR := $(BUILD_DIR)/app
TEST_OBJ_DIR := $(BUILD_DIR)/tests
TEST_SRC_DIR := $(ROOT_DIR)/tests

# targets and pre-requisites
SRC := $(wildcard $(SRC_DIR)/*/*.c )
OBJECTS := $(SRC:$(SRC_DIR)/%.c=$(OBJ_DIR)/%.o)

TESTS_SRC := $(wildcard $(TEST_SRC_DIR)/*/*.cpp)
TESTS_OBJ := $(TESTS_SRC:$(TEST_SRC_DIR)/%.cpp=$(TEST_OBJ_DIR)/%.o)

TARGET := libds.so
TEST_TARGET := runner

# libraries (without -l)
LIBS :=
TEST_LIBS := gtest gtest_main pthread

# compiler settings
CXX := g++ -std=c++17
CXXFLAGS := -pedantic-errors -Wall -Wextra -Werror -fPIC
LDFLAGS := $(addprefix -l,$(LIBS))
INCLUDE := -I$(INCLUDE_DIR)

TEST_LIBS := $(addprefix -l,$(TEST_LIBS))

# final preparations (don't change this)
TARGET_FINAL := $(APP_DIR)/$(TARGET)
TEST_TARGET_FINAL := $(ROOT_DIR)/$(TEST_TARGET)

# tests, static analysis and code coverage
GCOV_COMMAND:=gcov -rnm $(TESTS_OBJ) $(OBJECTS) 2>/dev/null | grep -Eo "('.*'|[[:digit:]]{1,3}.[[:digit:]]{2}%)" | paste -d " " - - | sort -k2 -nr
UNTESTED_DETECTOR_COMMAND:=$(GCOV_COMMAND) | grep -v "100.00%" | awk '{ print "\x1b[38;2;255;25;25;1m" $$1 " \x1b[0m\x1b[38;2;255;100;100;1m" $$2 "\x1b[0m" }'
COVERAGE_COMMAND:=@$(UNTESTED_DETECTOR_COMMAND); $(GCOV_COMMAND) | awk '{ sum += $$2; count[NR] = $$2 } END { if(NR%2) { median=count[(NR+1)/2]; } else { median=count[NR/2]; } if( NR==0 ) { NR=1; } print "\x1b[32mCode Coverage\x1b[0m:\n\t\x1b[33mAverage\x1b[0m: " sum/NR "%\n\x1b[35m\tMedian\x1b[0m: " median  }'
RUN_TESTS_COMMAND:=@valgrind -q --exit-on-first-error=yes --error-exitcode=1 --tool=memcheck\
		--show-reachable=yes --leak-check=yes --track-origins=yes $(TEST_TARGET_FINAL)
STATIC_ANALYSIS_COMMAND:=@cppcheck --addon=cert --addon=y2038 --addon=threadsafety --addon=naming \
	$(INCLUDE) --suppress=missingIncludeSystem --bug-hunting --quiet --enable=all $(SRC) $(TESTS_SRC)

.PHONY: all folders clean debug release test

release: CXXFLAGS += -O3
release: all 

all: folders $(TARGET_FINAL)

debug: CXXFLAGS += -DDEBUG -g
debug: COVERAGE = --coverage
debug: | $(TESTS_OBJ) test all

test: LDFLAGS += $(TEST_LIBS)
test: $(TESTS_OBJ) $(OBJECTS)
	$(CXX) $(CXXFLAGS) $(INCLUDE) $(COVERAGE) -o $(TEST_TARGET_FINAL) $^ $(LDFLAGS)
	$(STATIC_ANALYSIS_COMMAND)
	$(RUN_TESTS_COMMAND)
	$(COVERAGE_COMMAND)

folders:
	@mkdir -p $(APP_DIR)
	@mkdir -p $(OBJ_DIR)
	@mkdir -p $(TEST_OBJ_DIR)

clean:
	-@rm -rvf $(OBJ_DIR)/*
	-@rm -rvf $(APP_DIR)/*
	-@rm -rvf $(TEST_OBJ_DIR)/*
	-@rm -vf $(TEST_TARGET_FINAL)

$(TARGET_FINAL): $(OBJECTS)
	@mkdir -p $(@D)
	$(CXX) $(CXXFLAGS) -o $(TARGET_FINAL) $^ $(LDFLAGS) -shared -fPIC

$(OBJECTS): $(OBJ_DIR)/%.o: $(SRC_DIR)/%.c
	@mkdir -p $(@D)
	$(CXX) $(CXXFLAGS) $(INCLUDE) $(COVERAGE) -c $< -o $@

$(TESTS_OBJ): $(OBJECTS)
$(TESTS_OBJ): $(TEST_OBJ_DIR)/%.o: $(TEST_SRC_DIR)/%.cpp
	@mkdir -p $(@D)
	$(CXX) $(CXXFLAGS) $(INCLUDE) -c $< -o $@
```

## Conclusion

You can also check a version of the final `Makefile` [here](https://gist.github.com/asimos-bot/dd3ac015efe8dcf2f08f8719e41153da).

I hope this gives you an idea on how to integrate tools to
help you write secure C code. If you intend on also doing
a project in another language, looking for similar tools
that work for them can really pay off.

Happy Learning!
