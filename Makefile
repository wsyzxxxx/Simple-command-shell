SOURCES=Interpreter.cpp ffosh.cpp Executor.cpp
OBJS=$(patsubst $.cpp, %.o, $(SOURCES))
CPPFLAGS= -Wall -Werror -pedantic -std=gnu++98 -ggdb3

all: ffosh printAllInputs printAllArguments segmentfault normalSuccess normalFailure

ffosh: $(OBJS)
	g++ $(CPPFLAGS) -o ffosh $(OBJS)

%.o: %.cpp Interpreter.h Executor.h Command.h
	g++ $(CPPFLAGS) -c $<

clean:
	rm -f ffosh printAllInputs printAllArguments testcases/segmentfault testcases/normalSuccess testcases/normalFailure *~ *.o

printAllArguments: testcases/printAllArguments.cpp
	g++ $(CPPFLAGS) testcases/printAllArguments.cpp -o $@

segmentfault: testcases/segmentfault.cpp
	g++ $(CPPFLAGS) testcases/segmentfault.cpp -o testcases/segmentfault

normalSuccess: testcases/normalSuccess.cpp
	g++ $(CPPFLAGS) testcases/normalSuccess.cpp -o testcases/normalSuccess

normalFailure: testcases/normalFailure.cpp
	g++ $(CPPFLAGS) testcases/normalFailure.cpp -o testcases/normalFailure

printAllInputs: testcases/printAllInputs.cpp
	g++ $(CPPFLAGS) testcases/printAllInputs.cpp -o $@
