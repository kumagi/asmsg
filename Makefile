CXX=g++
# -L/usr/local/lib
OPTS=-O0 -fexceptions -g
LD=-lboost_program_options
WARNS= -W -Wall -Wextra -Wformat=2 -Wstrict-aliasing=4 -Wcast-qual -Wcast-align \
	-Wwrite-strings -Wfloat-equal -Wpointer-arith -Wswitch-enum
NOTIFY=&& notify-send Test success! -i ~/themes/ok_icon.png || notify-send Test failed... -i ~/themes/ng_icon.png
SRCS=$(HEADS) $(BODYS)

target:asmsg
target:test_do

test_do:test
	./test $(NOTIFY)

asmsg:asmsg.o
	$(CXX) asmsg.o $(LD) $(OPTS) -o $@
#	./asmsg $(NOTIFY)

asmsg.o:asmsg.cc asmsg.hpp
	$(CXX) -c asmsg.cc $(LD) $(OPTS) -o $@

test:test.o gtest.o
	g++ -I$(GTEST_DIR)/include test.o gtest.o -o $@ $(OPTS) -pthread $(LD)

test.o:test.cc
	g++ -c test.cc -o -O0 $(LD) $(OPTS) -o $@
gtest.o:
	g++ -c -I$(GTEST_DIR)/include -I$(GTEST_DIR) $(GTEST_DIR)/src/gtest-all.cc -o $@

