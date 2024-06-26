#include "transport_catalogue.h"
#include "json_reader.h"
#include <iostream>
#include <fstream>

int main() {

    using namespace trans_cat;
    TransportCatalogue catalogue;
    
     std::ifstream in_file("../in-out-files/in_4.txt");
    // std::ifstream in_file("../in-out-files/s12_final_opentest_1.json");
    std::ofstream out_file("../in-out-files/out.txt");
    JsonReader input_json;
    input_json.LoadJSON(/*in_file*/std::cin);
    input_json.ParseRequest(catalogue);
    auto render_options = input_json.LoadRender();
   // PrintSVG(catalogue, render_options, std::cout);
    input_json.ParseStateRequest(catalogue, std::cout/* out_file*/);
}