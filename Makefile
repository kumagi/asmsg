CXX=g++
# -L/usr/local/lib
OPTS=-O4 -fexceptions -g
LD=-lboost_program_options
TEST_LD= -lgtest_main -lpthread $(LD)
GTEST_INC= -I$(GTEST_DIR)/include -I$(GTEST_DIR)
GTEST_DIR=/opt/google/gtest-1.5.0
WARNS= -W -Wall -Wextra -Wformat=2 -Wstrict-aliasing=4 -Wcast-qual -Wcast-align \
	-Wwrite-strings -Wfloat-equal -Wpointer-arith -Wswitch-enum
NOTIFY=&& notify-send Test success! -i ~/themes/ok_icon.png || notify-send Test failed... -i ~/themes/ng_icon.png
SRCS=$(HEADS) $(BODYS)

#target:asmsg
target:test_do

test_do:test
	./test $(NOTIFY)

asmsg:asmsg.o asmsg_impl.o
	$(CXX) asmsg.o asmsg_impl.o $(LD) $(OPTS) -o $@
	./asmsg $(NOTIFY)

asmsg.o:asmsg.cc asmsg.hpp
	$(CXX) -c asmsg.cc $(OPTS) -o $@ $(WARNS) 
asmsg_impl.o:asmsg_impl.cc
	$(CXX) -c asmsg_impl.cc $(OPTS) -o $@ $(WARNS) 


test:test.o gtest_main.a asmsg_impl.o
	$(CXX) $(GTEST_INC) $^ -o $@ $(WARNS)  $(OPTS) $(TEST_LD)

test.o:test.cc
	$(CXX) -c test.cc -o $(OPTS) $(WARNS) -o $@

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