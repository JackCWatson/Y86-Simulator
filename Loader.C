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
        int counter = 0;
        while (std::getline(inf, holder))
        {
           counter += 1;
           if (trickyErrors(holder))
           {
               if (hasErrors(holder))
               {
                   std::cout << "Error on line " << std::dec << counter
                             << ": " << holder << std::endl;
                   return;
               }
               else
               {
                   loadline(holder);
               }
           }
        }
        inf.close();
   }
   loaded = true;
   
   

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
   bool error = false;
   int byteOne = DATABEGIN;
   uint16_t address = convert(lineRead, ADDRBEGIN, ADDREND);
   Memory * mem = Memory::getInstance();

   while(lineRead.c_str()[byteOne] != ' ' && lineRead.c_str()[byteOne + 1] != ' ')
   {
       byteOneVal = convert(lineRead, byteOne, byteOne + 1);
       byteOne += 2;
       mem->putByte(byteOneVal, address, error);
       //Attempt to store last used address
       lastAddr = address;
       address += 1;
       
   }

}




int32_t Loader::convert(string line, int begin, int end)
{
    std::string str = line.substr(begin, (end - begin) + 1);
    return stoul(str, NULL, 16);
}   


bool Loader::trickyErrors(string line)
{
    if (align(line)) return true;
    if (line[COMMENT] != '|') return true;
    //Blank lines
    if (line.c_str()[DATABEGIN] != ' ') return true;
    //Improper formating
    if (line[DATABEGIN] != ' ' && line[DATABEGIN + 1] != ' ') return true;
    if (line[0] == '0' && line[1] != 'x') return true;
    else return false;
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

bool Loader::hasErrors(string input)
{
     // When restructuring code, think: 
     // Multiple cases: data with lines, data without lines, partial lines, etc
     // This should help with removal of trickyErrors
     // need to redue alignment case, missing comment bar, and bites cases Then will work

     //pre checks
     if (validChar(input)) return true;
     if(comment(input)) return true;
     
     //complete lines
     if(input[0] != ' ' && input[DATABEGIN] != ' ')
     {

        if (input[DATABEGIN] != ' ' && input[DATABEGIN + 1] != ' ' && input[DATABEGIN + 2] == ' ' && input[DATABEGIN + 6] != ' ') return true;
        if (memAddress(input)) return true;
        if (colon(input)) return true;
        if (validChar(input)) return true;
        if (byteTwo(input)) return true;
        if (boundsCheck(input)) return true;

        if (greaterMem(input)) return true;
     }
     if (comment(input)) return true;
     //lines without data
     else if(input[0] != ' ' && input[DATABEGIN] == ' ')
     {
        if (align(input)) return true;
        if (memAddress(input)) return true;
     }
     else
     {
        if (dataNoAdd(input)) return true;
        if (memAddress(input)) return true;
     }

     return false;
     
}

bool Loader::align(string input)
{
    if (input[DATABEGIN + 1] != ' ')
    {
        return true;
    }
    return false;
}
bool Loader::memAddress(string input)
{
    if (input[0] != '0' || input[1] != 'x')
    {
        return true;
    }
    return false;
}
bool Loader::colon(string input)
{
    if (input[5] != ':' || input[6] != ' ')
    {
        return true;
    }
    return false;
}
bool Loader::validChar(string input)
{
    for (int i = DATABEGIN; input[i] != ' '; i++)
    {
        if(input[i] < '0' || input[i] > 'f')
        {
            return true;
        }
    }
    return false;
}
bool Loader::comment(string input)
{
    if (input[COMMENT] != '|')
    {
        return true;
    }
    return false;
}
bool Loader::byteTwo(string input)
{
    int j = DATABEGIN;
    int bitCount = 0;
    while (input[j] != ' ')
    {
        bitCount++;
        j++;
    }
    if (bitCount % 2 != 0)
    {
        return true;
    }
    return false;
}
bool Loader::dataNoAdd(string input)
{
    if (input[DATABEGIN]!= ' ' && input[0] == ' ')
    {
        return true;
    }
    return false;
}

bool Loader::boundsCheck(string input)
{
    int count = 0;
    int i = DATABEGIN;
    while (input[i] != ' ')
    {
        i++;
        count++;
    }
    count = count / 2;
    if (convert(input, ADDRBEGIN, ADDREND) + count > MEMSIZE) return true;
    return false;


}

bool Loader::greaterMem(string input)
{   
    /*
    uint16_t currAddr = convert(input, ADDRBEGIN, ADDREND);
    if (lastAddr >= currAddr) return true;
    else
    {
        lastAddr = currAddr;
        return false;
    }
    */

    if (convert(input, ADDRBEGIN, ADDREND) < lastAddr) return true;
    int currAddr = 0;
    for (int i = DATABEGIN; input[i] != '|'; i++)
    {
        if (input[i] == ' ') currAddr = 1;
        if (currAddr == 1 && input[i] != ' ') return true;
    }
    return false;



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
