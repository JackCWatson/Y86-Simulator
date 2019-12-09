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
#include "MemoryStage.h"
#include "Status.h"
#include "Debug.h"
#include "Memory.h"
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
bool MemoryStage::doClockLow(PipeReg ** pregs, Stage ** stages)
{
   M * mreg = (M *) pregs[MREG];
   W * wreg = (W *) pregs[WREG];
   bool error = false;
   uint64_t valE = mreg->getvalE()->getOutput();
   uint64_t dstE = mreg->getdstE()->getOutput();
   uint64_t dstM = mreg->getdstM()->getOutput();
   uint64_t icode = mreg->geticode()->getOutput(); 
   uint64_t valA = mreg->getvalA()->getOutput();
   
   valM = 0;
   uint64_t m_addr = mem_addr(icode, valA, valE);
   
   if (mem_read(icode))
   {
       Memory * memory = Memory::getInstance();
       valM = memory->getLong(m_addr, error);
   }
   
   if (mem_write(icode))
   {
       Memory * memory = Memory::getInstance();
       memory->putLong(valA, m_addr, error);
   }
   stat = m_stat(error, mreg->getstat()->getOutput()); 




  setWInput(wreg, stat, icode, valE, valM, dstE, dstM);
   return false;
}

/* doClockHigh
 * applies the appropriate control signal to the F
 * and D register intances
 *
 * @param: pregs - array of the pipeline register (F, D, E, M, W instances)
 */
void MemoryStage::doClockHigh(PipeReg ** pregs)
{
   W * wreg = (W *) pregs[WREG];

   wreg->getstat()->normal();
   wreg->geticode()->normal();
   wreg->getvalE()->normal();
   wreg->getvalM()->normal();
   wreg->getdstE()->normal();
   wreg->getdstM()->normal();
}

/* setDInput
 * provides the input to potentially be stored in the D register
 * during doClockHigh
 *
 * @param: dreg - pointer to the D register instance
 * @param: stat - value to be stored in the stat pipeline register within D
 * @param: icode - value to be stored in the icode pipeline register within D
 * @param: ifun - value to be stored in the ifun pipeline register within D
 * @param: rA - value to be stored in the rA pipeline register within D
 * @param: rB - value to be stored in the rB pipeline register within D
 * @param: valC - value to be stored in the valC pipeline register within D
 * @param: valP - value to be stored in the valP pipeline register within D
*/
void MemoryStage::setWInput(W * wreg, uint64_t stat, uint64_t icode, 
                           uint64_t valE, uint64_t valM, uint64_t dstE,
                           uint64_t dstM)
{
   wreg->getstat()->setInput(stat);
   wreg->geticode()->setInput(icode);
   wreg->getvalE()->setInput(valE);
   wreg->getvalM()->setInput(valM);
   wreg->getdstE()->setInput(dstE);
   wreg->getdstM()->setInput(dstM);
}
/*
 * @param icode checks to see if the instruction uses valE or valA
 * @param m_valA is returned if the instruction is a popq or ret
 * @param m_valE is returned if the instruction is a rmmovq pushq call or mrmovq
 */
uint64_t MemoryStage::mem_addr(uint64_t icode, uint64_t m_valA, uint64_t m_valE)
{
    if (icode == IRMMOVQ || icode == IPUSHQ || icode == ICALL || icode == IMRMOVQ) return m_valE;
    if (icode == IPOPQ || icode == IRET) return m_valA;
    return 0;
}
/*
 * @param icode if the instruction reads from memory then return true
 */
bool MemoryStage::mem_read(uint64_t icode)
{
    return (icode == IMRMOVQ || icode == IPOPQ || icode == IRET);
}
/*
 * @param icode if the instruction writes to memory then return truee
 */
bool MemoryStage::mem_write(uint64_t icode)
{
    return (icode == IRMMOVQ || icode == IPUSHQ || icode == ICALL);
}

/*
 * @return valM
 */
uint64_t MemoryStage::getvalM()
{
    return valM;
}
/*
 * @param mem_error controls what is returned determined on its value
 * @param M_stat value of stat in memeory stage and is returned if mem_error is false
 */
uint64_t MemoryStage::m_stat(bool mem_error, uint64_t M_stat) {
    if (mem_error) return SADR;
    return M_stat;
}
/*
 * @return stat
 */
uint64_t MemoryStage::getm_stat() {
    return stat;
}
