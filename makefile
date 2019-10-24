CC = g++
CFLAGS = -g -c -Wall -std=c++11 -Og
OBJ = yess.o Memory.o Tools.o RegisterFile.o ConditionCodes.o Loader.o D.o E.o F.o M.o W.o\
 FetchStage.o DecodeStage.o ExecuteStage.o MemoryStage.o WritebackStage.o PipeReg.o PipeRegField.o Simulate.o 

.C.o:
	$(CC) $(CFLAGS) $< -o $@

yess: $(OBJ)

yess.o: Memory.h RegisterFile.h ConditionCodes.h Loader.h

Memory.o: Memory.h Tools.h

RegisterFile.o: RegisterFile.h Tools.h

ConditionCodes.o: ConditionCodes.h Tools.h

Loader.o: Loader.h Memory.h

D.o: Instructions.h RegisterFile.h PipeReg.h PipeRegField.h D.h Status.h
E.o: RegisterFile.h Instructions.h PipeRegField.h PipeReg.h E.h Status.h
F.o: PipeRegField.h PipeReg.h F.h
M.o: RegisterFile.h Instructions.h PipeRegField.h PipeReg.h M.h Status.h
W.o: RegisterFile.h Instructions.h PipeRegField.h PipeReg.h W.h Status.h

FetchStage.o: RegisterFile.h PipeRegField.h PipeReg.h F.h D.h M.h W.h Stage.h FetchStage.h Status.h Debug.h
DecodeStage.o: RegisterFile.h PipeRegField.h PipeReg.h F.h E.h D.h M.h W.h Stage.h DecodeStage.h Status.h Debug.h Instructions.h
ExecuteStage.o: RegisterFile.h PipeRegField.h PipeReg.h F.h D.h M.h W.h Stage.h ExecuteStage.h Status.h Debug.h Instructions.h
MemoryStage.o: RegisterFile.h PipeRegField.h PipeReg.h F.h D.h M.h W.h Stage.h MemoryStage.h Status.h Debug.h Instructions.h
WritebackStage.o: RegisterFile.h PipeRegField.h PipeReg.h F.h D.h M.h W.h Stage.h WritebackStage.h Status.h Debug.h Instructions.h
PipeReg.o: PipeReg.h
PipeRegField.o: PipeRegField.h
Simulate.o: PipeRegField.h PipeReg.h F.h D.h E.h M.h W.h Stage.h ExecuteStage.h MemoryStage.h\
 DecodeStage.h FetchStage.h WritebackStage.h Simulate.h Memory.h RegisterFile.h ConditionCodes.h




clean:
	rm $(OBJ) yess

run:
	make clean
	make yess
	./run.sh

