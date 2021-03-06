//class to perform the combinational logic of
//the Fetch stage
class FetchStage: public Stage
{
   private:
      void setDInput(D * dreg, uint64_t stat, uint64_t icode, uint64_t ifun, 
                     uint64_t rA, uint64_t rB,
                     uint64_t valC, uint64_t valP);
      uint64_t selectPC(F * freg, M * mreg, W * wreg);
      bool needRegIds(uint64_t f_icode);
      bool needValC(uint64_t f_icode);
      uint64_t PCIncrement(uint64_t fpc, bool nRegID, bool nValC);
      uint64_t predictPC(uint64_t f_icode, uint64_t f_valC, uint64_t f_valP);
      void getRegIds(uint64_t f_code, uint64_t * rA, uint64_t * rB);
      uint64_t buildValC(uint64_t fpc, bool needRegIds);
      bool instr_valid(uint64_t icode);
      bool F_stall, D_stall;
      bool getF_stall(uint64_t E_icode, uint64_t E_dstM, uint64_t d_srcA, uint64_t d_srcB);
      bool getD_stall(uint64_t E_icode, uint64_t E_dstM, uint64_t d_srcA, uint64_t d_srcB);
      void calculateControlSignals(PipeReg ** pregs, uint64_t d_srcA, uint64_t d_srcB);


      uint64_t f_stat(bool error, uint64_t icode);
      uint64_t f_icode(bool error, uint64_t icode);
      uint64_t f_ifun(bool error, uint64_t ifun);

   public:
      bool doClockLow(PipeReg ** pregs, Stage ** stages);
      void doClockHigh(PipeReg ** pregs);

};
