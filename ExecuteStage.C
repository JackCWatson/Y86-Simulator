#include <string>
#include <cstdint>
#include "RegisterFile.h"
#include "PipeRegField.h"
#include "PipeReg.h"
#include "F.h"
#include "E.h"
#include "D.h"
#include "M.h"
#include "W.h"
#include "Stage.h"
#include "ExecuteStage.h"
#include "Status.h"
#include "Debug.h"
#include "Instructions.h"
#include "ConditionCodes.h"
#include "Tools.h"


/*
 * doClockLow:
 * Performs the Fetch stage combinational logic that is performed when
 * the clock edge is low.
 *
 * @param: pregs - array of the pipeline register sets (F, D, E, M, W instances)
 * @param: stages - array of stages (FetchStage, DecodeStage, ExecuteStage,
 *         MemoryStage, WritebackStage instances)
 */
bool ExecuteStage::doClockLow(PipeReg ** pregs, Stage ** stages)
{
   M * mreg = (M *) pregs[MREG];
   E * ereg = (E *) pregs[EREG];
   
   uint64_t Cnd = 0;


   uint64_t stat = ereg->getstat()->getOutput();
   uint64_t icode = ereg->geticode()->getOutput();
   //uint64_t ifun = ereg->getifun()->getOutput();
   uint64_t valA = ereg->getvalA()->getOutput();
   uint64_t dstM = ereg->getdstM()->getOutput();
   dstE = ereg->getdstE()->getOutput();
    
   uint64_t e_aluFun = alufun(icode, ereg);
   uint64_t e_aluA = aluA(icode, ereg);
   uint64_t e_aluB = aluB(icode, ereg);
    
   valE = e_alu(e_aluFun, set_cc(icode), e_aluA, e_aluB);
   //uint64_t e_cond = cond(icode, ifun);
   //dstE = e_dstE(icode, e_cond, ereg);




   //provide the input values for the D register
   setMInput(mreg, stat, icode, Cnd, valE, valA, dstE, dstM);
   return false;
}

/* doClockHigh
 * applies the appropriate control signal to the F
 * and D register intances
 *
 * @param: pregs - array of the pipeline register (F, D, E, M, W instances)
 */
void ExecuteStage::doClockHigh(PipeReg ** pregs)
{
   M * mreg = (M *) pregs[MREG];

   mreg->getstat()->normal();
   mreg->geticode()->normal();
   mreg->getCnd()->normal();
   mreg->getvalE()->normal();
   mreg->getvalA()->normal();
   mreg->getdstE()->normal();
   mreg->getdstM()->normal();
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
void ExecuteStage::setMInput(M * mreg, uint64_t stat, uint64_t icode, 
                           uint64_t Cnd, uint64_t valE, uint64_t valA,
                           uint64_t dstE, uint64_t dstM)
{
   mreg->getstat()->setInput(stat);
   mreg->geticode()->setInput(icode);
   mreg->getvalE()->setInput(valE);
   mreg->getCnd()->setInput(Cnd);
   mreg->getvalA()->setInput(valA);
   mreg->getdstE()->setInput(dstE);
   mreg->getdstM()->setInput(dstM);
}

uint64_t ExecuteStage::aluA(uint64_t icode, E * ereg)
{
    if (icode == IRRMOVQ || icode == IOPQ) return ereg->getvalA()->getOutput();
    if (icode == IIRMOVQ || icode == IRMMOVQ || icode == IMRMOVQ) return ereg->getvalC()->getOutput();
    if (icode == ICALL || icode == IPUSHQ) return -8;
    if (icode == IRET || icode == IPOPQ) return 8;
    return 0;
}

uint64_t ExecuteStage::aluB(uint64_t icode, E * ereg)
{
    if (icode == IRMMOVQ || icode == IMRMOVQ || icode == IOPQ || icode == ICALL || icode == IPUSHQ ||
        icode == IRET || icode == IPOPQ) return ereg->getvalB()->getOutput();
    if (icode == IRRMOVQ || icode == IIRMOVQ) return 0;
    return 0;
}

uint64_t ExecuteStage::alufun(uint64_t icode, E * ereg)
{
    if (icode == IOPQ) return ereg->getifun()->getOutput();
    return ADDQ;
}

//come back and check on this method
bool ExecuteStage::set_cc(uint64_t icode)
{
    return (icode == IOPQ);
}

uint64_t ExecuteStage::e_alu(uint64_t alufun, bool setCC, uint64_t aluA, uint64_t aluB)
{
     bool checkOverflow = false;
     bool error = false;
     uint64_t result = 0;
     //Switch Statement instead of a million ifs, to check ALU
     switch(alufun)
     {
         case ADDQ:
            checkOverflow = Tools::addOverflow(aluA, aluB);
            result = aluA + aluB;
            break;
         case ANDQ:
            result = aluA & aluB;
            break;
         case XORQ:
            result = aluA ^ aluB;
            break;
            //check
         case SUBQ:
            result = aluB - aluA;
            checkOverflow = Tools::subOverflow(aluB, aluA);
            break;
     }
     if (setCC)
     {
         ConditionCodes * codes = ConditionCodes::getInstance();
         codes->setConditionCode((result == 0), ZF, error);
         codes->setConditionCode(Tools::sign(result), SF, error);
         codes->setConditionCode(checkOverflow, OF, error);
     }
     return result;
}


uint64_t ExecuteStage::e_dstE(uint64_t icode, uint64_t e_Cnd, E * ereg)
{
    //what id e_Cnd
    if (icode == IRRMOVQ && !e_Cnd) return RNONE;
    return ereg->getdstE()->getOutput();
}

uint64_t ExecuteStage::getdstE()
{
    return dstE;
}
uint64_t ExecuteStage::getValE()
{
    return valE;
}
