CC = g++
CFLAGS = -g -c -Wall -std=c++11 -Og
OBJ = lab5.o MemoryTester.o Memory.o Tools.o RegisterFile.o \
RegisterFileTester.o ConditionCodes.o ConditionCodesTester.o

.C.o:
	$(CC) $(CFLAGS) $< -o $@

lab5: $(OBJ)

lab5.o: Memory.h RegisterFile.h MemoryTester.h RegisterFileTester.h ConditionCodes.h ConditionCodesTester.h

MemoryTester.o: Memory.h MemoryTester.h

Memory.o: Memory.h Tools.h

RegisterFile.o: RegisterFile.h Tools.h

RegisterFileTester.o: RegisterFile.h RegisterFileTester.h

ConditionCodes.o: ConditionCodes.h Tools.h

ConditionCodesTester.o: ConditionCodes.h ConditionCodesTester.h




clean:
	rm $(OBJ) lab5

run:
	make clean
	make lab5
	./run.sh

