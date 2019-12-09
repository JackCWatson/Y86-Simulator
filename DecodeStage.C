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
#include "Instructions.h"
#include "ExecuteStage.h"
#include "MemoryStage.h"
#include "DecodeStage.h"
#include "Status.h"
#include "Debug.h"


/*
 * doClockLow:
 * Performs the Fetch stage combinational logic that is performed when
 * the clock edge is low.
 *
 * @param: pregs - array of the pipeline register sets (F, D, E, M, W instances)
 * @param: stages - array of stages (FetchStage, DecodeStage, ExecuteStage,
 *         MemoryStage, WritebackStage instances)
 */
bool DecodeStage::doClockLow(PipeReg ** pregs, Stage ** stages)
{
   ExecuteStage * eStage = (ExecuteStage*)stages[ESTAGE];
   MemoryStage * mStage = (MemoryStage*)stages[MSTAGE];
   D * dreg = (D *) pregs[DREG];
   E * ereg = (E *) pregs[EREG];
   W * wreg = (W *) pregs[WREG];
   M * mreg = (M *) pregs[MREG];
   //F * freg = (F *) pregs[FREG];
   uint64_t icode = dreg->geticode()->getOutput(), ifun = dreg->getifun()->getOutput(), 
   valC = dreg->getvalC()->getOutput();
   uint64_t stat = dreg->getstat()->getOutput();
   //Added ValP
   uint64_t valP = dreg->getvalP()->getOutput();
   srcA = d_srcA(icode, dreg->getrA());
   srcB = d_srcB(icode, dreg->getrB());
   uint64_t dstE = d_dstE(icode, dreg->getrB());
   uint64_t dstM = d_dstM(icode, dreg->getrA());
   uint64_t valA = d_valA(srcA, eStage, mStage, mreg, wreg, icode, valP); 
   uint64_t valB = d_valB(srcB, eStage, mStage, mreg, wreg);

   setEInput(ereg, stat, icode, ifun, valC, valA, valB, dstE, dstM, srcA, srcB);
   return false;
}

/* doClockHigh
 * applies the appropriate control signal to the F
 * and D register intances
 *
 * @param: pregs - array of the pipeline register (F, D, E, M, W instances)
 */
void DecodeStage::doClockHigh(PipeReg ** pregs)
{
   E * ereg = (E *) pregs[EREG];

   ereg->getstat()->normal();
   ereg->geticode()->normal();
   ereg->getifun()->normal();
   ereg->getvalC()->normal();
   ereg->getvalA()->normal();
   ereg->getvalB()->normal();
   ereg->getdstE()->normal();
   ereg->getdstM()->normal();
   ereg->getsrcA()->normal();
   ereg->getsrcB()->normal();
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
void DecodeStage::setEInput(E * ereg, uint64_t stat, uint64_t icode, 
                           uint64_t ifun, uint64_t valC, uint64_t valA,
                           uint64_t valB, uint64_t dstE, uint64_t dstM, uint64_t srcA, uint64_t srcB)
{
   ereg->getstat()->setInput(stat);
   ereg->geticode()->setInput(icode);
   ereg->getifun()->setInput(ifun);
   ereg->getsrcA()->setInput(srcA);
   ereg->getsrcB()->setInput(srcB);
   ereg->getdstE()->setInput(dstE);
   ereg->getdstM()->setInput(dstM);
   ereg->getvalC()->setInput(valC);
   ereg->getvalA()->setInput(valA);
   ereg->getvalB()->setInput(valB);
}
/*
 * @param icode to be comapred to values of actual icodes
 * @param rA to obtain the value of what is in rA
 */
uint64_t DecodeStage::d_srcA(uint64_t icode, PipeRegField * rA)
{
    if (icode == IRRMOVQ || icode == IRMMOVQ || icode == IOPQ || icode == IPUSHQ) return rA->getOutput();
    if (icode == IPOPQ || icode == IRET) return RSP;
    return RNONE;
}
/*
 * @param icode to be compared to values of actual icodes
 * @param rB to obtain the value of what is in rB
 */
uint64_t DecodeStage::d_srcB(uint64_t icode, PipeRegField * rB)
{
    if (icode == IOPQ || icode == IRMMOVQ || icode == IMRMOVQ) return rB->getOutput();
    if (icode == IPUSHQ || icode == IPOPQ || icode == ICALL || icode == IRET) return RSP;
    return RNONE;
}
/*
 * @param icode to be compared to actual icode values
 * @param rB to obtain what is in rB
 */
uint64_t DecodeStage::d_dstE(uint64_t icode, PipeRegField * rB)
{
    if (icode == IRRMOVQ || icode == IIRMOVQ || icode == IOPQ) return rB->getOutput();
    if (icode == IPUSHQ || icode == IPOPQ || icode == ICALL || icode == IRET) return RSP;
    return RNONE;
}
/*
 * @param icode to be compared to values of actual icodes
 * @param rA to obtain the value of what is in rA
 */
uint64_t DecodeStage::d_dstM(uint64_t icode, PipeRegField * rA)
{
    if (icode == IMRMOVQ || icode == IPOPQ) return rA->getOutput();
    return RNONE;
}
/*
 * @param d_srcA value to be compared to opcodes
 * @param eStage pointer to eStage to obtain its values
 * @param mStage pointer to mStage to obtain its values
 * @param mreg pointer to the memory register
 * @param wreg pointer to the writeback register
 * @param icode ot check to see if a jump should be taken
 * @param valP will return the next sequential instruction
 */
uint64_t DecodeStage::d_valA(uint64_t d_srcA, ExecuteStage* eStage, MemoryStage* mStage, M * mreg, W * wreg, uint64_t icode, uint64_t valP)
{  
    bool error = false;
    if (icode == ICALL || icode == IJXX) return valP; // Added this
    if (d_srcA == RNONE) return 0;
    if (d_srcA == eStage->getdstE()) return eStage->getValE();
    if (d_srcA == mreg->getdstE()->getOutput()) return mreg->getvalE()->getOutput();
    if (d_srcA == wreg->getdstE()->getOutput()) return wreg->getvalE()->getOutput();
     
    if (d_srcA == mreg->getdstM()->getOutput()) return mStage->getvalM();
    if (d_srcA == wreg->getdstM()->getOutput()) return wreg->getvalM()->getOutput();

    RegisterFile * reg = RegisterFile::getInstance();
    return reg->readRegister(d_srcA, error);
}

/*
 * @param d_srcB value to checked against other values of next stage to set valE
 * @param eStage to be used to grab values of other stage to compare to d_srcB
 * @param mreg to be used to grab values of other stage to compare to d_srcB
 * @param wreg to be used to grab values of other stage to compare to d_srcB
 */
uint64_t DecodeStage::d_valB(uint64_t d_srcB, ExecuteStage* eStage, MemoryStage* mStage, M * mreg, W * wreg)
{
    bool error = false;
    if (d_srcB == RNONE) return 0;
    if (d_srcB == eStage->getdstE()) return eStage->getValE();
    if (d_srcB == mreg->getdstE()->getOutput()) return mreg->getvalE()->getOutput();
    if (d_srcB == wreg->getdstE()->getOutput()) return wreg->getvalE()->getOutput();

    if (d_srcB == mreg->getdstM()->getOutput()) return mStage->getvalM();
    if (d_srcB == wreg->getdstM()->getOutput()) return wreg->getvalM()->getOutput();

    RegisterFile * reg = RegisterFile::getInstance();
    return reg->readRegister(d_srcB, error);
}
/*
 * @return srcA
 */
uint64_t DecodeStage::getd_srcA()
{
    return srcA;
}
/*
 * @return srcB
 */
uint64_t DecodeStage::getd_srcB()
{
    return srcB;
}
