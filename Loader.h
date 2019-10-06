class Loader
{
   private:
      bool loaded;        //set to true if a file is successfully loaded into memory
      std::ifstream inf;  //input file handle
   public:
      Loader(int argc, char * argv[]);
      bool fileOpen(int argc, char * argv[]);
      bool isLoaded();

      //Make these work
      void loadline(std::string lineRead);
      int convert(std::string line, int a, int b);
};
