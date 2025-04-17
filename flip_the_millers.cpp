#include <iostream>
#include <fstream>
#include <string>
#include <algorithm>
#include <array>
#include <vector>
#include <chrono>
#include <sstream>
#include <unordered_map>
#include <utility>
#include <cmath>
#include <iomanip>


// sorting
bool is_data_line(const std::string& line) {
    return !line.empty() && line[0] != '!';
}

bool is_numeric(const std::string& s) {
    if (s.empty()) return false;
    size_t start = (s[0] == '-') ? 1 : 0;
    bool has_dot = false;
    for (size_t i = start; i < s.size(); ++i) {
        if (s[i] == '.') {
            if (has_dot) return false;
            has_dot = true;
        } else if (!isdigit(s[i])) {
            return false;
        }
    }
    return true;
}

void write_table(std::ofstream& file, const std::vector<std::vector<std::string>>& table) {
    for (const auto& row : table) {
        file << "\t";
        for (size_t i = 0; i < row.size(); ++i) {
            file << row[i];
            if (i != row.size() - 1) file << "\t";
        }
        file << "\n";
    }
}

bool sort_and_overwrite_table_file(const std::string& filename) {
    std::ifstream infile(filename);
    if (!infile) {
        std::cerr << "Error opening input file: " << filename << "\n";
        return false;
    }

    std::vector<std::string> header_lines;
    std::vector<std::string> footer_lines;
    std::vector<std::vector<std::string>> table;

    std::string line;
    bool in_table = false;

    while (std::getline(infile, line)) {
        if (!in_table && is_data_line(line)) {
            in_table = true;
            header_lines.push_back(line);
            continue;
        }

        if (in_table && !is_data_line(line)) {
            footer_lines.push_back(line);
            continue;
        }

        if (in_table) {
            std::istringstream rowStream(line);
            std::string cell;
            std::vector<std::string> row;

            while (rowStream >> cell) {
                row.push_back(cell);
            }

            if (!row.empty()) {
                table.push_back(row);
            }
        } else {
            header_lines.push_back(line);
        }
    }
    infile.close();

    std::string status;
    std::stable_sort(table.begin(), table.end(), [&status](const std::vector<std::string>& a, const std::vector<std::string>& b) {
        if (is_numeric(a[0]) && is_numeric(b[0])) {
            status = "success";
            return std::abs(std::stoi(a[0])) < std::abs(std::stoi(b[0]));
        } else {
            status = "failed";
            return false;
        }
    });

    std::ofstream outfile(filename);
    if (!outfile) {
        std::cerr << "Error opening output file: " << filename << "\n";
        return false;
    }

    for (const auto& h : header_lines) outfile << h << '\n';
    write_table(outfile, table);
    for (const auto& f : footer_lines) outfile << f << '\n';

//     std::cout << status << std::endl;
    return true;
}


std::string vector_to_tab_delimited_string(const std::vector<std::string>& row) {
    std::ostringstream oss;
//     oss << '\t';  // Add the initial tab
    for (size_t i = 0; i < row.size(); ++i) {
        oss << row[i];
        if (i != row.size() - 1) {
            oss << '\t';  // Add tab between items
        }
    }
    return oss.str();
}

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

std::pair<std::string, int> strip_dash_return_sense(const char* arg) {
    std::string newindex = arg ;   // arg should be equal to       new_index[ "default_indices[i]" ]
    int multiplier = 1;
    std::string newindex_stripped;
    for (char c : newindex) {
//         std::cout << "testing this if it is a miller index: " << c << std::endl;
        if (c == '-') {
            multiplier = multiplier * -1; //multiply the column by -1
            continue;
        } 
        else {
            multiplier = multiplier * 1;
            newindex_stripped = std::string(1,c);
        }
    }
    return std::make_pair(newindex_stripped, multiplier); //return these two and store them to be used
}

int main(int argc, char* argv[]) {
    // Check if filename was provided
    if (argc != 5)  {
        std::cerr << "Usage: " << argv[0] << " <filename.HKL> h k l \n or k h l (to flip h and k)\n or -h l k (to flip k and l, while inverting h)" << std::endl;
        return 1;
    }
//     std::string default_indices = "hkl" ; 
    std::array<char, 3> default_indices = {'h', 'k', 'l'};
    std::unordered_map<char, int> index_map;
    // Precompute the map for fast lookup
    for (size_t i = 0; i < default_indices.size(); ++i) {
        index_map[default_indices[i]] = i;
    }

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
//       std::unordered_map<std::string, std::string> new_index;     
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
//                   std::cout << "Key: '" << std::string(1, hkl[i]) << "' for argv[" << i + 2 << "] = " << argv[i+2] << std::endl;
                  new_index[std::string(1, hkl[i])]=argv[i+2];
                //   std::cout << "Flipping "<<hkl[i]<<" with "<<hkl[j-2]<<" "<<inverting << std::endl;              
                }
            }
        }
//         for (const auto& pair : new_index) {
//         std::cout << "h -> " << new_index["h"] << std::endl;
//         std::cout << "k -> " << new_index["k"] << std::endl;
//         std::cout << "l -> " << new_index["l"] << std::endl;
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
        
        size_t lineLength = current - lineStart;
        std::string line(lineStart, lineLength);
        
        // check if end of file reached before incrementing nb of lines, and break the loop
        if (!endingFound) {
//             size_t lineLength = current - lineStart;
//             std::string line(lineStart, lineLength);
            if (line.find(ender) != std::string::npos) {
                endingFound = true;
                outputFile << line << "\n";
                break;
            }
        }

        // Check if line contains marker
        if (!markerFound) {
//             size_t lineLength = current - lineStart;
//             std::string line(lineStart, lineLength);
            if (line.find("!UNIT_CELL_CONSTANTS=") != std::string::npos) {
                std::istringstream rowStream(line);
                std::vector<std::string> row;
                std::string newindex;
                int multiplier = 1;
                while (rowStream >> value) {
                    row.push_back(value);
                }
                std::vector<std::string> new_row;
                new_row.push_back(row[0]);
                for (int i=0;i<3;i++) {
                    std::string key = std::string(1, default_indices[i]);
                    const char *cstr = new_index[ key ].c_str();
                    auto index_info = strip_dash_return_sense(cstr);
                    newindex = index_info.first;
                    multiplier = index_info.second;
                    auto itt = index_map.find(newindex[0]); // get index from index_map
                    try {
                        float val = std::stof(row[1 + itt->second]);
//                         val = val * multiplier;  
                        std::ostringstream val_stream;
                        val_stream << std::fixed << std::setprecision(3) << val ;
                        std::string val_str = val_stream.str();
                        new_row.push_back( val_str );
                    } catch (const std::invalid_argument& e) {
                        std::cerr << "Invalid input for stof: '" << row[1 + itt->second] << "'" << std::endl;
                        new_row.push_back("0.0");  // fallback
                    }
                }
                new_row.insert(new_row.end(), row.begin() + 4, row.end());
                line = vector_to_tab_delimited_string(new_row);
                std::cout << line << std::endl;
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
//             std::vector<std::vector<std::string>> table;
//             size_t lineLength = current - lineStart;
//             std::string line(lineStart, lineLength);

            std::istringstream rowStream(line);
            std::vector<std::string> row;
            while (rowStream >> value) {
                row.push_back(value);
            }

            
            linesAfterMarker++;

            // Spinner setup
            const char spinner[] = "|/-\\";
            int i = 0;  // Spinner index
            
            
            std::string newindex;
            int multiplier = 1;
            while (rowStream >> value) {
                row.push_back(value);
            }
            std::vector<std::string> new_row;
//             new_row.push_back(row[0]);
            for (int i=0;i<3;i++) {
                std::string key = std::string(1, default_indices[i]);
                const char *cstr = new_index[ key ].c_str();
                auto index_info = strip_dash_return_sense(cstr);
                newindex = index_info.first;
                multiplier = index_info.second;
//                 std::cout << "Checking key: " << newindex[0] <<"." << std::endl;
                auto itt = index_map.find(newindex[0]); // get index from index_map
                if (itt != index_map.end()) {
                    try {
                        int val = std::stoi(row[itt->second]);
                        val = val * multiplier;  
                        std::string val_str = std::to_string(val);
                        new_row.push_back( val_str );
                    } catch (const std::invalid_argument& e) {
                        std::cerr << "Invalid input for stoi: '" << row[itt->second] << "'" << std::endl;
                        new_row.push_back("0");  // fallback
                    }
                } else {
                    std::cerr << "Index not found for key: " << newindex[0] << std::endl;
                    new_row.push_back("0");
                }
            }
            new_row.insert(new_row.end(), row.begin() + 3, row.end());
            line = "";
            for (size_t i = 0; i < new_row.size(); ++i) {
                line += new_row[i];
                if (i != new_row.size() - 1) line += '\t'; // tab delimiter
            }
            // Spinner animation
            std::cout << "\r" << "Loading ... " << "\r" << spinner[i % 2] << std::flush;
            i++;
//             std::cout << line << std::endl;
            std::string tabbed_line = vector_to_tab_delimited_string(new_row);
            outputFile << "\t" << line << "\n";

        }
        
        
        
        if (current < end) current++; // skip newline character
    }
    
    std::cout << "number of reflections is: " << linesAfterMarker << std::endl;
    

    

    // Complete writing and stop spinner
    std::cout << "\rDone !" << std::endl;
    file.close();
    sort_and_overwrite_table_file(output_filename);
    std::cout << "Output file: " << output_filename << std::endl;    
    return 0;
}
