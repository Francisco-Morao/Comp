#---------------------------------------------------------------
#             CONFIGURE THESE VARIABLES IF NEEDED
#---------------------------------------------------------------

ROOT = 
CDK_INC_DIR = libcdk20-202408310000/src
CDK_LIB_DIR = libcdk20-202408310000/
CDK_BIN_DIR = libcdk20-202408310000/bin

LANGUAGE=udf

#---------------------------------------------------------------
# PROBABLY, THERE'S NO NEED TO CHANGE ANYTHING BEYOND THIS POINT
#---------------------------------------------------------------

L_NAME=$(LANGUAGE)_scanner
Y_NAME=$(LANGUAGE)_parser

LFLAGS   = 
YFLAGS   = -dtv
CXXFLAGS = -std=c++23 -DYYDEBUG=1 -pedantic -Wall -Wextra -ggdb -I. -I$(CDK_INC_DIR) -Wno-unused-parameter
LDFLAGS  = -L$(CDK_LIB_DIR) -lcdk 
COMPILER = $(LANGUAGE)

CDK  = $(CDK_BIN_DIR)/cdk
LEX  = flex
YACC = bison

SRC_CPP = $(shell find ast -name \*.cpp) $(wildcard targets/*.cpp) $(wildcard ./*.cpp)
OFILES  = $(SRC_CPP:%.cpp=%.o)

#---------------------------------------------------------------
#                DO NOT CHANGE AFTER THIS LINE
#---------------------------------------------------------------

all: .auto/all_nodes.h .auto/visitor_decls.h $(COMPILER)

%.tab.o:: %.tab.c
	$(CXX) $(CXXFLAGS) -c $< -o $@ -Wno-class-memaccess

%.o:: %.c
	$(CXX) $(CXXFLAGS) -c $< -o $@

%.o:: %.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@

%.tab.c:: %.y
	$(YACC) $(YFLAGS) -b $* $<

%.tab.h:: %.y
	$(YACC) $(YFLAGS) -b $* $<

$(L_NAME).cpp: $(L_NAME).l
	$(LEX) $(LFLAGS) $<

$(Y_NAME).tab.c: $(Y_NAME).y
$(Y_NAME).tab.h: $(Y_NAME).y

# this is needed to force byacc to run
$(L_NAME).o: $(L_NAME).cpp $(Y_NAME).tab.h

.auto/all_nodes.h: 
	$(CDK) ast --decls ast --language $(LANGUAGE) > $@

.auto/visitor_decls.h: 
	$(CDK) ast --decls target --language $(LANGUAGE) > $@

$(COMPILER): $(L_NAME).o $(Y_NAME).tab.o $(OFILES)
	$(CXX) -o $@ $^ $(LDFLAGS)

clean:
	$(RM) .auto/all_nodes.h .auto/visitor_decls.h *.tab.[ch] *.o $(OFILES) $(L_NAME).cpp $(Y_NAME).output $(COMPILER)

depend: .auto/all_nodes.h
	$(CXX) $(CXXFLAGS) -MM $(SRC_CPP) > .makedeps

run:
	valgrind ./udf --target asm co-25-fork/auto-tests/W-02-125-N-ok.udf
	yasm -gdwarf2 -felf32 co-25-fork/auto-tests/W-02-125-N-ok.asm
	ld -m elf_i386 -o W-02-125-N-ok W-02-125-N-ok.o -L/home/francisco/091/librts6-20250414131211 -lrts
	valgrind -s ./W-02-125-N-ok

-include .makedeps

#---------------------------------------------------------------
#                           THE END
#---------------------------------------------------------------
