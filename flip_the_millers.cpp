#include <iostream>
#include <fstream>
#include <string>
#include <algorithm>
#include <array>
#include <vector>
#include <chrono>
#include <sstream>
#include <unordered_map>

// testing input orientations if only contains indices and minus sign
bool is_miller_index(const std::string& str) {
    std::string indices = "hkl" ; 
    for (char c : str) {
        if (indices.find(c) != std::string::npos) {
            return true;
        }
        if (c == '-') {
            continue;
        }
    }
    return false;
}

// Helper function to add "_flipped" before the extension
std::string make_flipped_filename(const std::string& input_name) {
    size_t dot_pos = input_name.find_last_of('.');
    if (dot_pos == std::string::npos) {
        return input_name + "_flipped";  // no extension
    } else {
        return input_name.substr(0, dot_pos) + "_flipped" + input_name.substr(dot_pos);
    }
}

// strip argv from - sign and return strings to be compared
std::string strip_dash(const char* arg) {
    std::string cleaned = arg;
    cleaned.erase(std::remove(cleaned.begin(), cleaned.end(), '-'), cleaned.end());
    return cleaned;
}

auto strip_dash_return_sense(const char* arg) {
    std::string newindex = arg ;   // arg should be equal to       new_index[ "default_indices[i]" ]
    int multiplier = 1;
    std::string newindex_stripped;
    for (char c : newindex) {
        if (c == '-') {
            multiplier = multiplier * -1; //multiply the column by -1
            continue;
        } 
        else {
            multiplier = multiplier * 1;
            newindex_stripped = std::string(1,c);
        }
    }
    return (newindex_stripped, multiplier); //return these two and store them to be used
}

int main(int argc, char* argv[]) {
    // Check if filename was provided
    if (argc != 5)  {
        std::cerr << "Usage: " << argv[0] << " <filename.HKL> h k l \n or k h l (to flip h and k)\n or -h l k (to flip k and l, while inverting h)" << std::endl;
        return 1;
    }
    std::string default_indices = "hkl" ; 
    std::string value;
    std::string filename = argv[1];
    std::string marker = "!END_OF_HEADER";
    std::string ender = "!END_OF_DATA";
    std::unordered_map<std::string, std::string> new_index;
    
    // Open file specified as first argument
//     std::ifstream inputFile(argv[1]);
    std::ifstream file(filename, std::ios::binary);
    if (!file.is_open()) {
        std::cerr << "Error opening file: " << argv[1] << std::endl;
        return 1;
    }
    
    std::string output_filename = make_flipped_filename(filename);
    std::ofstream outputFile(output_filename);  
    
    // understand the requested modification
    if ((strip_dash(argv[2]) == strip_dash(argv[3])) || (strip_dash(argv[2]) == strip_dash(argv[4])) || (strip_dash(argv[4]) == strip_dash(argv[3])))
    {
        std::cout << "Error: some indices entered are the same" << std::endl;
        return 1;  
    }
    else if ((!is_miller_index(argv[2])) || (!is_miller_index(argv[3])) || (!is_miller_index(argv[4]))) {
        std::cerr << "Error: miller indices information, it should only be h k l, and a minus sign (if inverting is needed)" << std::endl;
        std::cerr << "Usage: " << argv[0] << " <filename.HKL> h k l \n or k h l (to flip h and k)\n or -h l k (to flip k and l, while inverting h)" << std::endl;
        return 1;
    }
    else {
      std::unordered_map<std::string, std::string> new_index;     
      std::array<char, 3> hkl = {'h', 'k', 'l'};
      for (int i=0; i<3; i++)
        {
          std::string inverting = "";
          for (int j=2; j<5; j++)
            {
              if (std::string(argv[j]).find(hkl[i]) != std::string::npos)
                {
                  if (std::string(argv[j]).find('-') != std::string::npos) {
                    std::string str = argv[j];
                    str.erase(std::remove(str.begin(), str.end(), '-'), str.end());
                    inverting = "and inverting " + str;
                  }
                  new_index[std::string(1, hkl[i])]=argv[i+2];
                //   std::cout << "Flipping "<<hkl[i]<<" with "<<hkl[j-2]<<" "<<inverting << std::endl;              
                }
            }
        }
//         for (const auto& pair : new_index) {
        std::cout << "h -> " << new_index["h"] << std::endl;
        std::cout << "k -> " << new_index["k"] << std::endl;
        std::cout << "l -> " << new_index["l"] << std::endl;
//         }     
    }
  
    
    // Read entire file into memory for fastest processing
    file.seekg(0, std::ios::end);
    size_t fileSize = file.tellg();
    file.seekg(0, std::ios::beg);
    
    std::vector<char> buffer(fileSize + 1);
    file.read(buffer.data(), fileSize);
    buffer[fileSize] = '\0';
    file.close();

    char* current = buffer.data();
    char* end = buffer.data() + fileSize;
    size_t linesAfterMarker = 0;
    bool markerFound = false;
    bool endingFound = false;

    while (current < end) {
        char* lineStart = current;
        
        // Find end of current line
        while (current < end && *current != '\n') current++;
        
        // check if end of file reached before incrementing nb of lines, and break the loop
        if (!endingFound) {
            size_t lineLength = current - lineStart;
            std::string line(lineStart, lineLength);
            if (line.find(ender) != std::string::npos) {
                endingFound = true;
                outputFile << line << "\n";
                break;
            }
        }

        // Check if line contains marker
        if (!markerFound) {
            size_t lineLength = current - lineStart;
            std::string line(lineStart, lineLength);
            if (line.find("!UNIT_CELL_CONSTANTS=") != std::string::npos) {
                std::istringstream rowStream(line);
                std::vector<std::string> row;
                std::string newindex;
                int multiplier = 1;
                while (std::getline(rowStream, value, '\t')) {
                    row.push_back(value);
                }
//                 for (int i=0; i< default_indices.length(); i++) {
//                     std::string newindex = new_index[ "default_indices[i]" ] ;
//                     int multiplier = 1;
//                     std::string newindex_stripped;
//                     for (char c : newindex) {
//                         if (c == '-') {
//                             multiplier = multiplier * -1; //multiply the column by -1
//                             continue;
//                         }
//                         else {
//                             multiplier = multiplier * 1;
//                             newindex_stripped = std::string(1,c);
//                         }
//                     }
//                     if (std::string(1, default_indices[i]) != newindex_stripped) {
//                         std::swap(row[i+1], row[ 1 + default_indices.find(newindex_stripped) ] );
//                     }
//                 }
                std::vector<std::string> new_row;
                new_row.push_back(row[0]);
                // use strip_dash_return_sense to get the multiplier and get the index without minus sign
                for (int i=0;i<3;i++) {
                    const char *cstr = new_index[ "default_indices[i]"].c_str();
                    tie(newindex , multiplier) = strip_dash_return_sense(cstr);
                    new_row.push_back(row[ 1 + default_indices.find(newindex)] * multiplier);
                }
                // new_row.push_back(row[ 1 + default_indices.find(new_index[ "default_indices[0]" ]) ]);
                // new_row.push_back(row[ 1 + default_indices.find(new_index[ "default_indices[1]" ]) ]);
                // new_row.push_back(row[ 1 + default_indices.find(new_index[ "default_indices[2]" ]) ]);
                new_row.insert(new_row.end(), row.begin() + 2, row.end());



                line = new_row;
                
                // std::swap(row[], row[]);    
            }
            
            outputFile << line << "\n";
            if (line.find(marker) != std::string::npos) {
                markerFound = true;
                current++; // move to next line
                continue;
            }
        } 
        else {
            // Count lines after marker
            std::vector<std::vector<std::string>> table;
            size_t lineLength = current - lineStart;
            std::string line(lineStart, lineLength);

            std::istringstream rowStream(line);
            std::vector<std::string> row;
            while (std::getline(rowStream, value, '\t')) {
                row.push_back(value);
            }
//             std::swap(row[], row[]);
            table.push_back(row);
            linesAfterMarker++;
        }
        
        if (current < end) current++; // skip newline character
    }
    
    std::cout << "number of reflections is: " << linesAfterMarker << std::endl;
    

    


    file.close();
    return 0;
}
