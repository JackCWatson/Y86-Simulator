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
#include "MemoryStage.h"
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
   W * wreg = (W *) pregs[WREG];
   MemoryStage * mStage = (MemoryStage*)stages[MSTAGE];
   uint64_t m_stat = mStage->getm_stat(); 
   M_bubble = calculateControlSignals(m_stat, wreg);

   uint64_t stat = ereg->getstat()->getOutput();
   uint64_t icode = ereg->geticode()->getOutput();
   uint64_t ifun = ereg->getifun()->getOutput();
   uint64_t valA = ereg->getvalA()->getOutput();
   uint64_t dstM = ereg->getdstM()->getOutput();
   dstE = ereg->getdstE()->getOutput();
    
   uint64_t e_aluFun = alufun(icode, ifun);
   uint64_t e_aluA = aluA(icode, ereg);
   uint64_t e_aluB = aluB(icode, ereg);
    
   valE = e_alu(e_aluFun, set_cc(icode, m_stat, wreg), e_aluA, e_aluB);
   //Just Added
   uint64_t e_cond = cond(ifun, icode);
   dstE = e_dstE(icode, e_cond, dstE);




   //provide the input values for the D register
   setMInput(mreg, stat, icode, e_cond, valE, valA, dstE, dstM);
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
   E * ereg = (E *) pregs[EREG];
   if (!M_bubble)
   {
       ereg->getstat()->normal();
       ereg->geticode()->normal();
       ereg->getifun()->normal();
       ereg->getvalB()->normal();
       ereg->getvalA()->normal();
       ereg->getvalC()->normal();
       ereg->getdstE()->normal();
       ereg->getdstM()->normal();
       ereg->getsrcA()->normal();
       ereg->getsrcB()->normal();
       
       mreg->getstat()->normal();
       mreg->geticode()->normal();
       mreg->getCnd()->normal();
       mreg->getvalE()->normal();
       mreg->getvalA()->normal();
       mreg->getdstE()->normal();
       mreg->getdstM()->normal();
   }
   else
   {
       mreg->getstat()->bubble(SAOK);
       mreg->geticode()->bubble(INOP);
       mreg->getCnd()->bubble();
       mreg->getvalE()->bubble();
       mreg->getvalA()->bubble();
       mreg->getdstE()->bubble(RNONE);
       mreg->getdstM()->bubble(RNONE);

   }

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

uint64_t ExecuteStage::alufun(uint64_t icode, uint64_t ifun)
{
    if (icode == IOPQ) return ifun;
    return ADDQ;
}

//come back and check on this method
bool ExecuteStage::set_cc(uint64_t icode, uint64_t m_stat, W * wreg)
{
    uint64_t w_stat = wreg->getstat()->getOutput();
    return (icode == IOPQ && !(m_stat == SADR || m_stat == SINS || m_stat == SHLT) && !(w_stat == SADR || w_stat == SINS || w_stat == SHLT));
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


uint64_t ExecuteStage::e_dstE(uint64_t icode, uint64_t e_Cnd, uint64_t e_dstE)
{
    //what id e_Cnd
    if (icode == IRRMOVQ && !e_Cnd) return RNONE;
    return e_dstE; //ereg->getdstE()->getOutput();
}

uint64_t ExecuteStage::getdstE()
{
    return dstE;
}
uint64_t ExecuteStage::getValE()
{
    return valE;
}
// Add to header file, and change DOCLOCKLOW
uint64_t ExecuteStage::cond(uint64_t ifun, uint64_t icode)
{
    bool error = false;
    ConditionCodes * CC = ConditionCodes::getInstance();
    uint64_t overFlow = CC->getConditionCode(OF, error);
    uint64_t signFlag = CC->getConditionCode(SF, error);
    uint64_t zeroFlag = CC->getConditionCode(ZF, error);
    //Could do a switch here
    if (icode == ICMOVXX || icode == IJXX)
    {
        if (ifun == UNCOND) return 1;
        if (ifun == LESSEQ) return ((signFlag ^ overFlow) || zeroFlag == 1);
        if (ifun == LESS) return (signFlag ^ overFlow);
        if (ifun == EQUAL) return (zeroFlag == 1);
        if (ifun == NOTEQUAL) return (zeroFlag == 0);
        if (ifun == GREATER) return ((signFlag ^ overFlow) == 0 && zeroFlag == 0);
        if (ifun == GREATEREQ) return ((signFlag ^ overFlow) == 0);
    }
    return 0;
}

bool ExecuteStage::calculateControlSignals(uint64_t m_stat, W * wreg) {
    uint64_t w_stat = wreg->getstat()->getOutput();
    return ((m_stat == SADR || m_stat == SINS || m_stat == SHLT) || (w_stat == SADR || w_stat == SINS || w_stat == SHLT));
}
