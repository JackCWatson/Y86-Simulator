class Loader
{
   private:
      bool loaded;        //set to true if a file is successfully loaded into memory
      std::ifstream inf;  //input file handle
   public:
      Loader(int argc, char * argv[]);
      bool fileOpen(int argc, char * argv[]);
      bool isLoaded();


      bool hasData(std::string lineRead);
      bool hasAddress(std::string lineRead);
      void loadline(std::string lineRead);
      int32_t convert(std::string line, int a, int b);
      bool hasErrors(std::string input);

      bool isComment(std::string input);
      bool isDigit(std::string input);
      bool multBytes(std::string input);
      bool wrongCharacters(std::string input);
      bool lastMem(std::string input);
      bool outsideArray(std::string input);



};
