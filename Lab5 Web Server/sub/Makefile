CXX = g++ -fPIC -pthread
NETLIBS= -lnsl

CC = g++
CCFLAGS = -g -std=c++17

generics= defines.hh

all: tester myhttpd

tester : tester.o $(generics)
	$(CXX) -o $@ $@.o $(NETLIBS)

myhttpd : myhttpd.o
	$(CXX) -o $@ $@.o  $(NETLIBS)

use-dlopen: use-dlopen.o
	$(CXX) -o $@ $@.o $(NETLIBS) -ldl

%.o: %.cc
	@echo 'Building $@ from $<'
	$(CXX) -o $@ -c -I. $<

clean:
	rm -f *.o myhttpd

