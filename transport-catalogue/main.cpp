#include <iostream>
#include <string>
#include <fstream>
#include <sstream>
#include "input_reader.h"
#include "stat_reader.h"


using namespace std;


int main() {
    using namespace trans_cat;
    TransportCatalogue catalogue;
   
    std::ifstream in("in.txt");
    {
        InputReader reader;
        reader.ParseRequest(std::cin);
        reader.ApplyCommands(catalogue);
    }
    ParseAndPrintStat(catalogue, std::cin, std::cout);
   
}