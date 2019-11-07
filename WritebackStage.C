#include <string>
#include <cstdint>
#include "RegisterFile.h"
#include "PipeRegField.h"
#include "PipeReg.h"
#include "F.h"
#include "D.h"
#include "M.h"
#include "W.h"
#include "Stage.h"
#include "WritebackStage.h"
#include "MemoryStage.h"
#include "Status.h"
#include "Debug.h"
#include "Instructions.h"

/*
 * doClockLow:
 * Performs the Fetch stage combinational logic that is performed when
 * the clock edge is low.
 *
 * @param: pregs - array of the pipeline register sets (F, D, E, M, W instances)
 * @param: stages - array of stages (FetchStage, DecodeStage, ExecuteStage,
 *         MemoryStage, WritebackStage instances)
 */
bool WritebackStage::doClockLow(PipeReg ** pregs, Stage ** stages)
{

   W * wreg = (W *) pregs[WREG];
   uint64_t icode = wreg->geticode()->getOutput();

   uint64_t valM = wreg->getvalM()->getOutput();
   uint64_t dstM = wreg->getdstM()->getOutput();
   bool error = false;

   if (icode == IHALT) return true;
   
   //MemoryStage * mem = (MemoryStage*) stages[MSTAGE];
   RegisterFile * regFile = RegisterFile::getInstance();
   //Error here?
   regFile->writeRegister(valM, dstM, error);

   return false;
}


/* doClockHigh
 * applies the appropriate control signal to the F
 * and D register intances
 *
 * @param: pregs - array of the pipeline register (F, D, E, M, W instances)
 */
void WritebackStage::doClockHigh(PipeReg ** pregs)
{
    W * wreg = (W *) pregs[WREG];
    bool error;
    RegisterFile * regfile = RegisterFile::getInstance();
    regfile->writeRegister(wreg->getvalE()->getOutput(), wreg->getdstE()->getOutput(), error);
}
