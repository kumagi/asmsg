CXX=g++
# -L/usr/local/lib
OPTS=-O0 -fexceptions -g
LD=-lboost_program_options
TEST_LD= -lgtest_main $(LD)
GTEST_INC= -I$(GTEST_DIR)/include -I$(GTEST_DIR)
GTEST_DIR=/opt/google/gtest-1.5.0
WARNS= -W -Wall -Wextra -Wformat=2 -Wstrict-aliasing=4 -Wcast-qual -Wcast-align \
	-Wwrite-strings -Wfloat-equal -Wpointer-arith -Wswitch-enum
NOTIFY=&& notify-send Test success! -i ~/themes/ok_icon.png || notify-send Test failed... -i ~/themes/ng_icon.png
SRCS=$(HEADS) $(BODYS)

target:asmsg
#target:test_do

test_do:test
	./test $(NOTIFY)

asmsg:asmsg.o
	$(CXX) asmsg.o $(LD) $(OPTS) -o $@
	./asmsg $(NOTIFY)

asmsg.o:asmsg.cc asmsg.hpp
	$(CXX) -c asmsg.cc $(OPTS) -o $@

test:test.o gtest_main.a
	$(CXX) $(GTEST_INC) $^ -o $@ $(OPTS) -lpthread

test.o:test.cc
	$(CXX) -c test.cc -o $(OPTS) -o $@

# gtest
gtest_main.o:
	$(CXX) $(GTEST_INC) -c $(OPTS) $(GTEST_DIR)/src/gtest_main.cc -o $@
gtest-all.o:
	$(CXX) $(GTEST_INC) -c $(OPTS) $(GTEST_DIR)/src/gtest-all.cc -o $@
gtest_main.a:gtest-all.o gtest_main.o
	ar -r $@ $^

clean:
	rm -f *.o
	rm -f *~