// lzw435.cpp
// Compresses a given filename using variable 16-bit LZW
// Or decompresses a file it has compressed to retrieve the original contents

#include <string>
#include <map>
#include <iostream>
#include <fstream>
#include <iterator>
#include <vector> 
#include <sys/stat.h>
#include <cmath>

/* This code is derived in parts from LZW@RosettaCode for UA CS435 
*/ 

// Constant to adjust max dictionary size
const int DICTMAXBITS = 16;
const int DICTSIZEMAX = std::pow(2, DICTMAXBITS);
 
// Compress a string to a list of output symbols.
// The result will be written to the output iterator
// starting at "result"; the final iterator is returned.
// Output: A container of integers representing dictionary elements
template <typename Iterator>
Iterator compress(const std::string &uncompressed, Iterator result) {
  // Build the dictionary, start with 256.
  int dictSize = 256;
  std::map<std::string,int> dictionary;
  for (int i = 0; i < dictSize; i++)
    dictionary[std::string(1, i)] = i;
 
  std::string previousWord;
  for (std::string::const_iterator iter = uncompressed.begin();
       iter != uncompressed.end(); ++iter) {
    char currentChar = *iter;
    std::string currentWord = previousWord + currentChar; // Expand the current word
    if (dictionary.count(currentWord)) // Word is already in code table
      previousWord = currentWord; // Continue expanding the word
    else {
      *result++ = dictionary[previousWord];
      // Add currentWord to the dictionary
      if (dictionary.size()<DICTSIZEMAX)
         dictionary[currentWord] = dictSize++;
      previousWord = std::string(1, currentChar);
    }
  }
 
  // Output the code for previousWord.
  if (!previousWord.empty())
    *result++ = dictionary[previousWord];
  return result;
}
 
// Decompress a list of output ks to a string.
// "begin" and "end" must form a valid range of ints
template <typename Iterator>
std::string decompress(Iterator begin, Iterator end) {
  // Build the dictionary.
  int dictSize = 256;
  std::map<int,std::string> dictionary;
  for (int i = 0; i < dictSize; i++)
    dictionary[i] = std::string(1, i);
 
  std::string w(1, *begin++);
  std::string result = w;
  std::string entry;
  for ( ; begin != end; begin++) {
    int k = *begin;
    if (dictionary.count(k))
      entry = dictionary[k];
    else if (k == dictSize)
      entry = w + w[0];
    else
      throw "Bad compressed k";

    result += entry;
 
    // Add w+entry[0] to the dictionary
    if (dictionary.size()<DICTSIZEMAX)
      dictionary[dictSize++] = w + entry[0];
 
    w = entry;
  }
  return result;
}

// Inputs: an integer and the desired length of the output string
// Ouputs: The integer, expressed as a binar string of the specified length
std::string int2BinaryString(int c, int cl) {
      std::string p = ""; //a binary code string with code length = cl
      int code = c;
      while (c>0) {         
		   if (c%2==0)
            p="0"+p;
         else
            p="1"+p;
         c=c>>1;   
      }
      int zeros = cl-p.size();
      if (zeros<0) {
         std::cout << "\nWarning: Overflow. code " << code <<" is too big to be coded by " << cl <<" bits!\n";
         p = p.substr(p.size()-cl);
      }
      else {
         for (int i=0; i<zeros; i++)  //pad 0s to left of the binary code if needed
            p = "0" + p;
      }
      return p;
}

// Inputs: A binary string
// Ouputs: An integer representation of the input string
int binaryString2Int(std::string p) {
   int code = 0;
   if (p.size()>0) {
      if (p.at(0)=='1') 
         code = 1;
      p = p.substr(1);
      while (p.size()>0) { 
         code = code << 1; 
		   if (p.at(0)=='1')
            code++;
         p = p.substr(1);
      }
   }
   return code;
}

int main(int argc, char *argv[]) {
   // Get input filename
   std::string fileName = argv[2];
   std::string zeros = "00000000";

   // If user chooses compress
   if(*argv[1] == 'c' || *argv[1] == 'C') {
      // Open and read in the specified file as a character array
      std::ifstream input;
      input.open(fileName.c_str(), std::ios::binary);
      struct stat filestatus;
      stat(fileName.c_str(), &filestatus);
      long fileSize = filestatus.st_size;
      char inputText[fileSize];
      input.read(inputText, fileSize);
      std::string inString(inputText);
      inString = inString.substr(0, fileSize);
      input.close();

      // Create a new file to store the compressed data
      std::ofstream output(fileName + ".lzw2", std::ios::trunc);

      // Compress the file into a container of dictionary references
      std::vector<int> compressed;
      compress(inString, std::back_inserter(compressed));

      // Convert the computed integer values into a binary string
      std::string binaryOutput = "";
      int bitCount = 0;
      int wordCounter = 0;
      int currentBits = 9;
      for(std::vector<int>::const_iterator iter = compressed.begin(); iter != compressed.end(); ++iter) {
         // When code table is full, increase number of bits per word, up to max size
         if(currentBits < DICTMAXBITS) {
            ++wordCounter;
            if(wordCounter == std::pow(2, currentBits - 1)) {
               ++currentBits;
               wordCounter = 0;
            }
         }
         binaryOutput += int2BinaryString(*iter, currentBits);
         bitCount = (bitCount + currentBits) % 8; // Track # of trailing bits
      }
      // Pad the end with zeroes
      for(int i = (8 - bitCount); i > 0; --i) { 
         binaryOutput += "0";
      }
      // Convert binary to characters and write to the file
      int byte; 
      for (int i = 0; i < (binaryOutput.size() - 1); i+=8) { 
         byte = 1;
         for (int j = 0; j < 8; j++) {
            byte = byte << 1;
            if (binaryOutput.at(i+j) == '1')
            byte += 1;
         }
         char currentChar = (char) (byte & 255); //save the string byte by byte
         output.write(&currentChar, 1);
      }
      output.close();
   }
   // If user chooses expand
   if(*argv[1] == 'e' || *argv[1] == 'E') {
      // Open and read in the specified file as a character array
      std::ifstream input;
      input.open(fileName.c_str(), std::ios::binary);
      struct stat filestatus;
      stat(fileName.c_str(), &filestatus);
      long fileSize = filestatus.st_size;
      char inputText[fileSize];
      input.read(inputText, fileSize);
      input.close();

      // Convert input text into a binary string
      std::string inputBinary = "";
      long count = 0;
      while(count < fileSize) {
         unsigned char currentChar = (unsigned char) inputText[count];
         std::string currentBinary = "";
         for(int bit = 0; bit < 8 && currentChar > 0; ++bit) {
            if (currentChar % 2 == 0) {
               currentBinary = "0" + currentBinary;
            }
            else {
               currentBinary = "1" + currentBinary;
            }
            currentChar = currentChar >> 1;
         }
         currentBinary = zeros.substr(0, (8 - currentBinary.size())) + currentBinary;
         inputBinary += currentBinary;
         ++count;
      }
      // Convert binary string into integers
      std::vector<int> compressed;
      int wordCounter = 0;
      int currentBits = 9;
      for(int start = 0; (inputBinary.size() - start) > currentBits; start += currentBits) {
         // When code table is full, increase number of bits per word, up to max size
         if(currentBits < DICTMAXBITS) {
            ++wordCounter;
            if(wordCounter == std::pow(2, currentBits - 1)) {
               ++currentBits;
               wordCounter = 0;
            }
         }
         compressed.push_back(binaryString2Int(inputBinary.substr(start, currentBits)));
      }

      // Create file to output decompressed data
      std::ofstream outputFile(fileName.substr(0, (fileName.size() - 5)) + ".2M", std::ios::trunc);

      // Decompress and write data to output
      outputFile << decompress(compressed.begin(), compressed.end());
      outputFile.close();
   }

  return 0;
}
