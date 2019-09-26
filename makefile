CC = g++
CFLAGS = -g -c -Wall -std=c++11 -Og
OBJ = lab5.o MemoryTester.o Memory.o Tools.o RegisterFile.o \
RegisterFileTester.o ConditionCodes.o ConditionCodesTester.o

.C.o:
	$(CC) $(CFLAGS) $< -o $@

lab6: $(OBJ)

lab6.o: Memory.h RegisterFile.h ConditionCodes.h loader.h

Memory.o: Memory.h Tools.h

RegisterFile.o: RegisterFile.h Tools.h

ConditionCodes.o: ConditionCodes.h Tools.h

Loader.o: Loader.h Memory.h

clean:
	rm $(OBJ) lab6

run:
	make clean
	make lab6
	./run.sh

