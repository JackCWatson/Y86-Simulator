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
#include "FetchStage.h"
#include "ExecuteStage.h"
#include "MemoryStage.h"
#include "DecodeStage.h"
#include "Status.h"
#include "Debug.h"
#include "Instructions.h"
#include "Tools.h"
#include "Memory.h"

/*
 * doClockLow:
 * Performs the Fetch stage combinational logic that is performed when
 * the clock edge is low.
 *
 * @param: pregs - array of the pipeline register sets (F, D, E, M, W instances)
 * @param: stages - array of stages (FetchStage, DecodeStage, ExecuteStage,
 *         MemoryStage, WritebackStage instances)
 */
bool FetchStage::doClockLow(PipeReg ** pregs, Stage ** stages)
{
   F * freg = (F *) pregs[FREG];
   D * dreg = (D *) pregs[DREG];
   M * mreg = (M *) pregs[MREG];
   W * wreg = (W *) pregs[WREG];
   DecodeStage * dStage = (DecodeStage*)stages[DSTAGE];
   uint64_t d_srcA = dStage->getd_srcA(); 
   uint64_t d_srcB = dStage->getd_srcB();
   bool error = false;
   uint64_t valC = 0;
   uint64_t rA = RNONE, rB = RNONE;
   calculateControlSignals(pregs, d_srcA, d_srcB);


   uint64_t f_pc = selectPC(freg, mreg, wreg);
   //bits 4-7 icode and 0-3 are ifun
   uint64_t instByte = Memory::getInstance()->getByte(f_pc, error);
   uint64_t icode = f_icode(error, Tools::getBits(instByte, 4, 7));
   uint64_t ifun = f_ifun(error, Tools::getBits(instByte, 0, 3));
   uint64_t stat = f_stat(error, icode); 

   uint64_t valP = PCIncrement(f_pc, needRegIds(icode), needValC(icode));
   uint64_t prediction = predictPC(icode, valC, valP);
       
   //New code that checks needsRegIDs and needsValC
   if (needValC(icode)) valC = buildValC(f_pc, needRegIds(icode));
   if (needRegIds(icode)) getRegIds(f_pc, &rA, &rB);
   
   
   
   
      
   //The value passed to setInput below will need to be changed
   freg->getpredPC()->setInput(prediction);

   //provide the input values for the D register
   setDInput(dreg, stat, icode, ifun, rA, rB, valC, valP);
   return false;
}

/* doClockHigh
 * applies the appropriate control signal to the F
 * and D register intances
 *
 * @param: pregs - array of the pipeline register (F, D, E, M, W instances)
 */
void FetchStage::doClockHigh(PipeReg ** pregs)
{
   F * freg = (F *) pregs[FREG];
   D * dreg = (D *) pregs[DREG];
   
   freg->getpredPC()->normal();
   dreg->getstat()->normal();
   dreg->geticode()->normal();
   dreg->getifun()->normal();
   dreg->getrA()->normal();
   dreg->getrB()->normal();
   dreg->getvalC()->normal();
   dreg->getvalP()->normal();
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
void FetchStage::setDInput(D * dreg, uint64_t stat, uint64_t icode, 
                           uint64_t ifun, uint64_t rA, uint64_t rB,
                           uint64_t valC, uint64_t valP)
{
   dreg->getstat()->setInput(stat);
   dreg->geticode()->setInput(icode);
   dreg->getifun()->setInput(ifun);
   dreg->getrA()->setInput(rA);
   dreg->getrB()->setInput(rB);
   dreg->getvalC()->setInput(valC);
   dreg->getvalP()->setInput(valP);
}
uint64_t FetchStage::f_icode(bool mem_error, uint64_t icode)
{
    if (mem_error) return INOP;
    return icode;
}
uint64_t FetchStage::f_ifun(bool mem_error, uint64_t ifun)
{
    if (mem_error) return FNONE;
    return ifun;
}
uint64_t FetchStage::f_stat(bool mem_error, uint64_t icode)
{
    if (mem_error == true) return SADR;
    if (instr_valid(icode) == false) return SINS;
    if (icode == IHALT) return SHLT;
    return SAOK;
}

bool FetchStage::instr_valid(uint64_t icode)
{
    switch(icode)
    {
        case INOP:
            return true;
        case IHALT:
            return true;
        case IRRMOVQ:
            return true;
        case IIRMOVQ:
            return true;
        case IRMMOVQ:
            return true;
        case IMRMOVQ:
            return true;
        case IOPQ:
            return true;
        case IJXX:
            return true;
        case ICALL:
            return true;
        case IRET:
            return true;
        case IPUSHQ:
            return true;
        case IPOPQ:
            return true;
        default:
            return false;
    }
}


uint64_t FetchStage::selectPC(F * freg, M * mreg, W * wreg)
{
    uint64_t M_icode = mreg->geticode()->getOutput(), M_Cnd = mreg->getCnd()->getOutput(),
    W_icode = wreg->geticode()->getOutput();

    if (M_icode == IJXX && !M_Cnd) return mreg->getvalA()->getOutput();
    if (W_icode == IRET) return wreg->getvalM()->getOutput();
    return freg->getpredPC()->getOutput();

}

bool FetchStage::needRegIds(uint64_t f_icode)
{
    return (f_icode == IRRMOVQ || f_icode == IOPQ || f_icode == IPUSHQ ||
        f_icode == IPOPQ || f_icode == IIRMOVQ || f_icode == IRMMOVQ || f_icode == IMRMOVQ);
}

bool FetchStage::needValC(uint64_t f_icode)
{
    return (f_icode == IIRMOVQ || f_icode == IRMMOVQ || f_icode == IMRMOVQ ||
        f_icode == IJXX || f_icode == ICALL);
}
uint64_t FetchStage::PCIncrement(uint64_t fpc, bool nRegID, bool nValC)
{
    if (nRegID) fpc += 1;
    if (nValC) fpc += 8;
    fpc += 1;
    return fpc;
}

uint64_t FetchStage::predictPC(uint64_t f_icode, uint64_t f_valC, uint64_t f_valP)
{
    if (f_icode == IJXX || f_icode == ICALL) return f_valC;
    return f_valP;
}

void FetchStage::getRegIds(uint64_t fpc, uint64_t * rA, uint64_t * rB)
{
    bool errorInput;
    Memory * mem = Memory::getInstance();
    
    uint64_t rVal = mem->getByte(fpc + 1, errorInput);
    uint8_t valrA = Tools::getBits(rVal, 4, 7);
    uint8_t valrB = Tools::getBits(rVal, 0, 3);

    *rA = valrA;
    *rB = valrB;
}

uint64_t FetchStage::buildValC(uint64_t fpc, bool needRegIds)
{
    int offSetCalc = fpc + 1;
    bool errorInput;
    Memory * mem = Memory::getInstance();
    uint8_t valC[8];
    if (needRegIds) offSetCalc += 1;
    for (int i = 0; i < 8; i++)
    {
        valC[i] = mem->getByte(offSetCalc + i, errorInput);
    }
    return Tools::buildLong(valC);

}

bool FetchStage::getF_stall(uint64_t E_icode, uint64_t E_dstM, uint64_t d_srcA, uint64_t d_srcB)
{
    return ((E_icode == IMRMOVQ || E_icode == IPOPQ) && (E_dstM == d_srcA || E_dstM == d_srcB)); 
}

bool FetchStage::getD_stall(uint64_t E_icode, uint64_t E_dstM, uint64_t d_srcA, uint64_t d_srcB)
{
    return ((E_icode == IMRMOVQ || E_icode == IPOPQ) && (E_dstM == d_srcA || E_dstM == d_srcB));
}

bool FetchStage::calculateControlSignals(PipeReg ** pregs, uint64_t d_srcA, uint64_t d_srcB)
{
    E * ereg = (E*) pregs[EREG];
    uint64_t E_icode = ereg->geticode()->getOutput();
    uint64_t E_dstM = ereg->getdstM()->getOutput();
    
    return (getF_stall(E_icode, E_dstM, d_srcA, d_srcB) || getD_stall(E_icode, E_dstM, d_srcA, d_srcB));

}





