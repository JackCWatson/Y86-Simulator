/**
 * Names: Jack Watson & Jake Wooten
 * Team:2
*/
#include <iostream>
#include <fstream>
#include <string.h>
#include <ctype.h>

#include <sstream>

#include "Loader.h"
#include "Memory.h"

using namespace std;

//first column in file is assumed to be 0
#define ADDRBEGIN 2   //starting column of 3 digit hex address 
#define ADDREND 4     //ending column of 3 digit hext address
#define DATABEGIN 7   //starting column of data bytes
#define COMMENT 28    //location of the '|' character 

/**
 * Loader constructor
 * Opens the .yo file named in the command line arguments, reads the contents of the file
 * line by line and loads the program into memory.  If no file is given or the file doesn't
 * exist or the file doesn't end with a .yo extension or the .yo file contains errors then
 * loaded is set to false.  Otherwise loaded is set to true.
 *
 * @param argc is the number of command line arguments passed to the main; should
 *        be 2
 * @param argv[0] is the name of the executable
 *        argv[1] is the name of the .yo file
 */
Loader::Loader(int argc, char * argv[])
{
   

   //Start by writing a method that opens the file (checks whether it ends 
   //with a .yo and whether the file successfully opens; if not, return without 
   //loading)
   string holder; 
   if (fileOpen(argc, argv) == false) 
   {
       loaded = false;
       return;
   }
   else
   {
       // Call to loadline method here
        while (std::getline(inf, holder))
        {
           //cout << holder << endl;
           if (hasData(holder) == true &&  hasAddress(holder) == true)
           {
               loadline(holder);
           }
        }
        loaded = true;
   }

   
   

   //the file handle is declared in Loader.h.  You should use that and
   //not declare another one in this file.
   
   //Next write a simple loop that reads the file line by line and prints it out
   
   //Next, add a method that will write the data in the line to memory 
   //(call that from within your loop)

   //Finally, add code to check for errors in the input line.
   //When your code finds an error, you need to print an error message and return.
   //Since your output has to be identical to your instructor's, use this cout to print the
   //error message.  Change the variable names if you use different ones.
   //  std::cout << "Error on line " << std::dec << lineNumber
   //       << ": " << line << std::endl;


   //If control reaches here then no error was found and the program
   //was loaded into memory.
  
}

bool Loader::fileOpen(int argc, char * argv[])
{
    // Reduce Return Statements
   
    if (argv[1] == NULL)
    {
        return false;
    }
    std::string file = argv[1];
    int size = strlen(argv[1]);
    if (size < 4)
    {
        return false;
    }
    if (file.compare(size - 3, size, ".yo") == 0)
    {
        inf.open(argv[1], std::ifstream::in);
        if (inf.is_open())
        {
            return true;
        }
        else
        {
            return false;
        }
    }
    else
    {
        return false;
    }


}

void Loader::loadline(string lineRead)
{
   uint8_t byteOneVal;
   //uint8_t byteTwoVal;
   bool error = false;
   int byteOne = DATABEGIN;
   //int byteTwo = DATABEGIN + 1;
   uint16_t address = convert(lineRead, ADDRBEGIN, ADDREND);
   Memory * mem = Memory::getInstance();

   while(lineRead.c_str()[byteOne] != ' ' && lineRead.c_str()[byteOne + 1] != ' ')
   {
       byteOneVal = convert(lineRead, byteOne, byteOne + 1);
       //byteTwoVal = convert(lineRead, byteTwo, byteTwo);
       std::cout << byteOneVal;
       byteOne += 2;
       //byteTwo += 2;
       mem->putByte(byteOneVal, address, error);
       //mem->putByte(byteTwoVal, address, error);
       address += 1;
   }
    
}




int32_t Loader::convert(string line, int begin, int end)
{
    // Want to convert string to a hec here
    string toDec = "";
    // for loop will iterate through to build the string
    // then convert to hex
    for (int i = begin; i <= end; i++)
    {
     toDec += line.c_str()[i];
    }
    // return the conversion
    std::cout << line << std::endl << strtol(toDec.c_str(), NULL, 16) << endl;
    return strtol(toDec.c_str(), NULL, 16);
}   


bool Loader::hasData(string line)
{
    if (line.c_str()[DATABEGIN] != ' ' )
    {
        return true;
    }
    else
    {
        return false;
    }
}

bool Loader::hasAddress(string line)
{
    if (line.c_str()[0] == '0')
    {
        return true;
    }
    else
    {
        return false;
    }
}


/**
 * isLoaded
 * returns the value of the loaded data member; loaded is set by the constructor
 *
 * @return value of loaded (true or false)
 */
bool Loader::isLoaded()
{
   return loaded;
}


//You'll need to add more helper methods to this file.  Don't put all of your code in the
//Loader constructor.  When you add a method here, add the prototype to Loader.h in the private
//section.
