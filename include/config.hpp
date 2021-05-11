#include <iostream>
#include <fstream>
#include <vector>
#include <list>
#include <string>
#include <map>

using namespace std;
class Config{
  public:
    map<string, string> possibleServerAddresses;

    Config(map<string, string> possibleServerAddresses);
    int generateIpsAndGatesLists();
  
};
