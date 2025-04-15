#include <iostream>
#include <fstream>
#include <string>


// testing input orientations if only contains indices and minus sign
bool is_miller_index(const std::string& str) {
    std::string indices = "hkl" ; 
    for (char c : str) {
        if (!indices.find(c) && c != '-') {
            return false;
        }
    }
    return true;
}

int main(int argc, char* argv[]) {
    // Check if filename was provided
    if (argc < 4) {
        std::cerr << "Usage: " << argv[0] << " <filename.HKL> h k l \n or k h l (to flip h and k)\n or -h l k (to flip k and l, while inverting h)" << std::endl;
        return 1;
    }

    // Open file specified as first argument
    std::ifstream inputFile(argv[1]);
    
    if (!inputFile.is_open()) {
        std::cerr << "Error opening file: " << argv[1] << std::endl;
        return 1;
    }


    
    
    // understand the requested modification
    if ((!is_miller_index(argv[2])) || (!is_miller_index(argv[3])) || (!is_miller_index(argv[4])))
    {
        std::cerr << "Error: miller indices information, it should only be h k l, and a minus sign (if inverting needed)" << std::endl;
        std::cerr << "Usage: " << argv[0] << " <filename.HKL> h k l \n or k h l (to flip h and k)\n or -h l k (to flip k and l, while inverting h)" << std::endl;
        return 1;
    }
    else {
        // if argv[2]
    }
    


    // Read and print file contents line by line
    std::string line;
    while (std::getline(inputFile, line)) {

        if (line.find("!END_OF_HEADER") != std::string::npos) {
            // get_the_millers = True;
        }

        std::cout << line << std::endl;
    }

    inputFile.close();
    return 0;
}