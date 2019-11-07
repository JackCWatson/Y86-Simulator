class DecodeStage: public Stage
{
   private:
      void setEInput(E * ereg, uint64_t stat, uint64_t icode, uint64_t ifun, 
                     uint64_t valC, uint64_t valA, uint64_t valB,
                     uint64_t dstE, uint64_t dstM, uint64_t srcA, uint64_t srcB);
      uint64_t d_srcA(uint64_t icode, PipeRegField * rA);
      uint64_t d_srcB(uint64_t icode, PipeRegField * rB);
      uint64_t d_dstE(uint64_t icode, PipeRegField * rB);
      uint64_t d_dstM(uint64_t icode, PipeRegField * rA);
      uint64_t d_valA(uint64_t d_srcA, ExecuteStage* eStage, MemoryStage* mStage, M * mreg, W * wreg, uint64_t icode, uint64_t valP);
      uint64_t d_valB(uint64_t d_srcB, ExecuteStage* eStage, MemoryStage* mStage, M * mreg, W * wreg);

   public:
      bool doClockLow(PipeReg ** pregs, Stage ** stages);
      void doClockHigh(PipeReg ** pregs);

};
