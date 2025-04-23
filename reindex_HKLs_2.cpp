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
#include <numeric>  // for std::accumulate



using namespace std;
using namespace std::chrono;


string script_header=   "\n#######################################################\n"
                        "Source code: https://github.com/m92rizk/reindex_hkl.git\n"
                        "Version: 2.0.0\n"
                        "#######################################################\n";

string get_os_endline() {
    #ifdef _WIN32
        return "\r\n";  // Windows
    #else
        return "\n";    // Unix/Linux/macOS
    #endif
}
    
string endline = get_os_endline();

void print_progress_bar(float progress, steady_clock::time_point start_time) {
    const int barWidth = 20;
    cout << "\r[";
    int pos = static_cast<int>(barWidth * progress);
    for (int i = 0; i < barWidth; ++i) {
        cout << (i < pos ? '=' : (i == pos ? '>' : ' '));
    }
    cout << "] " << fixed << setprecision(0) << (progress * 100.0f) << "%";

    auto now = steady_clock::now();
    auto elapsed = duration_cast<seconds>(now - start_time).count();
    cout << " | Elapsed: " << elapsed << "s";

    // if (progress >= 1.0f)
    //     cout << endl;

    cout.flush();
}


string toStringWithPrecision(float value, int precision) {
    ostringstream oss;
    oss << fixed << setprecision(precision) << value;
    return oss.str();
}

void push_back_padded(vector<string>& strings, const string& item, int width, int space_after) {
    ostringstream val_stream;
    val_stream << setw (width) << item ;
    string val_str = val_stream.str();
    strings.push_back( val_str );
    for (int i=0; i<space_after; i++) {
        strings.push_back(" ");
    }
}

std::string vector_to_string(const std::vector<std::string>& row) {
    std::ostringstream oss;
    for (size_t i = 0; i < row.size(); ++i) {
        oss << row[i];
        // if (i != row.size() - 1) {
        //     oss << '\t';  // Add tab between items
        // }
    }
    return oss.str();
}

// sorting
bool is_data_line(const std::string& line) {
    return !line.empty() && line[0] != '!';
}

bool is_numeric(const std::string& s) {
    // Trim leading and trailing spaces
    size_t first = s.find_first_not_of(" \t");
    size_t last = s.find_last_not_of(" \t");
    std::string trimmed_s = (first == std::string::npos || last == std::string::npos) 
                            ? "" 
                            : s.substr(first, (last - first + 1));

    
    if (trimmed_s.empty()) return false;
    size_t start = (trimmed_s[0] == '-') ? 1 : 0;
    bool has_dot = false;
    for (size_t i = start; i < trimmed_s.size(); ++i) {
        if (trimmed_s[i] == '.') {
            if (has_dot) return false;
            has_dot = true;
        } else if (!isdigit(trimmed_s[i])) {
            return false;
        }
    }
    return true;
}

// Helper function to add "_reIDX" before the extension
string make_reindexed_filename(const std::string& input_name, string& suffix) {
    size_t dot_pos = input_name.find_last_of('.');
    if (dot_pos == std::string::npos) {
        return input_name + suffix;  // no extension
    } else {
        return input_name.substr(0, dot_pos) + suffix + input_name.substr(dot_pos);
    }
}

vector<string> parse_fixed_width_row(const string& line, const vector<int>& fieldWidths) {
    vector<string> fields;
    size_t pos = 0;
    int expected_length = accumulate(fieldWidths.begin(), fieldWidths.end(), 0);
    
    if (line.length() < expected_length) {
        cerr << "\n\nLine too short! Expected " << expected_length << ", got " << line.length() << "\n";
        // cerr << line << endl;
    }

    for (int width : fieldWidths) {
        if (pos >= line.length()) break;
        string field = line.substr(pos, width);
        // Trim leading/trailing spaces
        // field.erase(0, field.find_first_not_of(" \t"));
        // field.erase(field.find_last_not_of(" \t") + 1);
        fields.push_back(field);
        pos += width;
    }

    return fields;
}

void write_table(ofstream& file, const vector<vector<string>>& table) {
    size_t total_rows = table.size();
    float last_progress = -1.0f;  // to force the first update
    int row_index = 0;
    cout<<"\n|Exporting reindexed file|"<<endl;
    auto start_time = steady_clock::now();
    int last_percent = -1;  // So it triggers on first 0%
    for (const auto& row : table) {
        float progress = static_cast<float>(row_index) / total_rows;
        int current_percent = static_cast<int>(progress * 100);
        
        if (current_percent != last_percent) {
            print_progress_bar(progress+0.01, start_time);
            last_percent = current_percent;
        }
        
        // // Force final update at 100%
        // if (row_index + 1 == total_rows) {
        //     progress = 1.0f;
        //     print_progress_bar(progress+0.01, start_time);
        // } else if (progress - last_progress >= 0.01f) {
        //     print_progress_bar(progress+0.01, start_time);
        //     last_progress = progress;
        // }

        if (row.size() < 12) {
            cerr << "Skipping row " << row_index << " : not enough columns (" << row.size() << "): ";
            for (const auto& cell : row) {
                cerr << "[" << cell << "] ";
            }
            cerr << "\n";
            ++row_index;
            continue;
        }
        vector<string> new_row; // 6 6 6 11 11 8 8 9 10 4 4 8
//         new_row.push_back(" ");
        push_back_padded(new_row, row[0],6,0);
        push_back_padded(new_row, row[1],6,0);
        push_back_padded(new_row, row[2],6,0);
        push_back_padded(new_row, row[3],11,0);
        push_back_padded(new_row, row[4],11,0);
        push_back_padded(new_row, row[5],8,0);
        push_back_padded(new_row, row[6],8,0);
        push_back_padded(new_row, row[7],9,0);
        push_back_padded(new_row, row[8],10,0);
        push_back_padded(new_row, row[9],4,0);
        push_back_padded(new_row, row[10],4,0);
        push_back_padded(new_row, row[11],8,0);
        string new_row_str = vector_to_string(new_row);
        file << new_row_str;
        file << endline;
        ++row_index;
    }
    cout<<endl;
}

// Read file in binary mode
vector<char> read_file_binary(const string& filename) {
    ifstream file(filename, ios::binary);
    if (!file) {
        cerr << "Error opening file: " << filename << "\n";
        return {};
    }

    // Move to end to get size
    file.seekg(0, ios::end);
    size_t fileSize = static_cast<size_t>(file.tellg());
    file.seekg(0, ios::beg);

    vector<char> buffer(fileSize);
    file.read(buffer.data(), fileSize);

    size_t actualRead = file.gcount();  // ← this is the truth
    buffer.resize(actualRead);          // Trim to the actual size

    if (actualRead != fileSize) {
        cerr << "⚠️ Warning: Expected to read " << fileSize 
             << " bytes, but got only " << actualRead << " bytes.\n";
    }

    return buffer;
}

void print_last_line(const string& filename){
    ifstream fin;
    fin.open(filename);
    if(fin.is_open()) {
        fin.seekg(-1,ios_base::end);                // go to one spot before the EOF

        bool keepLooping = true;
        while(keepLooping) {
            char ch;
            fin.get(ch);                            // Get current byte's data

            if((int)fin.tellg() <= 1) {             // If the data was at or before the 0th byte
                fin.seekg(0);                       // The first line is the last line
                keepLooping = false;                // So stop there
            }
            else if(ch == '\n') {                   // If the data was a newline
                keepLooping = false;                // Stop at the current position.
            }
            else {                                  // If the data was neither a newline nor at the 0 byte
                fin.seekg(-2,ios_base::cur);        // Move to the front of that data, then to the front of the data before it
            }
        }

        string lastLine;            
        getline(fin,lastLine);                      // Read the current line
        cout << "Result: " << lastLine << '\n';     // Display it

        fin.close();
    }
}

bool sort_and_overwrite_table_file(const string& filename) {
    // cout<<"last line of " << filename << " is : ";
    // print_last_line(filename);
    string sorted_suffix = "_sorted";
    // string table_suffix = "_table";
    string filename_sorted = make_reindexed_filename(filename, sorted_suffix);
    // string table_file = make_reindexed_filename(filename, table_suffix);
    
    ifstream infile(filename);
    // Get total size
    infile.seekg(0, ios::end);
    size_t fileSize = infile.tellg();
    infile.seekg(0, ios::beg);

    string line;
    float last_progress = -1.0f;

    // ifstream fin;
    // fin.open(filename);
    // if(fin.is_open()) {
    //     fin.seekg(-1,ios_base::end);                // go to one spot before the EOF
    //     bool keepLooping = true;
    //     while(keepLooping) {
    //         char ch;
    //         fin.get(ch);                            // Get current byte's data
    //         if((int)fin.tellg() <= 1) {             // If the data was at or before the 0th byte
    //             fin.seekg(0);                       // The first line is the last line
    //             keepLooping = false;                // So stop there
    //         }
    //         else if(ch == '\n') {                   // If the data was a newline
    //             keepLooping = false;                // Stop at the current position.
    //         }
    //         else {                                  // If the data was neither a newline nor at the 0 byte
    //             fin.seekg(-2,ios_base::cur);        // Move to the front of that data, then to the front of the data before it
    //         }
    //     }
    //     string lastLine;            
    //     getline(fin,lastLine);                      // Read the current line
    //     cout << "Result: " << lastLine << '\n';     // Display it
    //     fin.close();
    //     return 1;
    // }
    // for( string line; getline( infile, line ); )
    // {
    //     cout<< line << endl;        
    // }
    // if (!infile) {
    //     cerr << "Error opening input file: " << filename << "\n";
    //     return false;
    // while (getline(infile, line)) {
    //     cout<< line << endl;
    // }


    bool in_table = false;
    vector<string> header_lines;
    vector<string> footer_lines;
    vector<vector<string>> table;
    int last_percent = -1;


    // ofstream outtable(table_file);

/*  uncomment this to try binary mode
    std::vector<char> buffer = read_file_binary(filename);

    const char* current = buffer.data();
    const char* end = buffer.data() + buffer.size();
    // cout << "buffer end is : " << end << endl;
    // return 1;
    // cout<<"#################################\nBINARY FILE \n" << current << "\n#################################" <<endl;
    while (current < end) {
        const char* lineStart = current;

        // Find end of line
        while (current < end && *current != '\n') current++;

        size_t lineLength = current - lineStart;

        // // Remove trailing CR if present
        // if (lineLength > 0 && lineStart[lineLength - 1] == '\r') {
        //     lineLength--;
        // }

        std::string line(lineStart, lineLength);
        // std::cout << "Line: [" << line << "]\n";

        if (!in_table && is_data_line(line)) {
            in_table = true;
            header_lines.push_back(line);
            continue;
        }

        if (in_table && !is_data_line(line)) {
            cerr << "Stopped parsing table at line:\n" << line << "\n";
            footer_lines.push_back(line);
            current++;
            continue;
        }

        if (in_table) {
            // istringstream rowStream(line);
            // string cell;
            vector<string> row;
            vector<int> widths = {6, 6, 6, 11, 11, 8, 8, 9, 10, 4, 4, 8}; 
            // while (rowStream >> cell) {
            //     row.push_back(cell);
            // }

            row = parse_fixed_width_row(line, widths);


            if (!row.empty()) {
                table.push_back(row);
                outtable << vector_to_string(row) << '\n';
            } else {
                cerr << "❌ Skipped malformed row\n";
            }
        } else {
            header_lines.push_back(line);
        }





        // Skip over newline chars
        if (current < end && *current == '\r') current++;
        if (current < end && *current == '\n') current++;
    }
*/
    auto start_time = steady_clock::now();
    cout<< "|Sorting|"<<endl;
    while (getline(infile, line)) {
        // istringstream rowStream(line);
        // string word;
        // while (rowStream >> word) {
        //     cout << word << " ";
        // }
        // cout<<endl;
        // cerr << "### Line read: [" << line << "] length=" << line.length() << '\n';
        if (!in_table && is_data_line(line)) {
            in_table = true;
            header_lines.push_back(line);
            continue;
        }

        if (in_table && !is_data_line(line)) {
            // cerr << "Stopped parsing table at line:\n" << line << "\n";
            footer_lines.push_back(line);
            auto current_pos = infile.tellg();
            if (current_pos != -1) {
                float progress = static_cast<float>(current_pos) / fileSize;
                int current_percent = static_cast<int>(progress * 100);
                if (current_percent != last_percent) {
                    print_progress_bar(progress, start_time);
                    last_percent = current_percent;
                }
            }
            continue;
        }

        if (in_table) {

            // string cell;
            vector<string> row;
            vector<int> widths = {6, 6, 6, 11, 11, 8, 8, 9, 10, 4, 4, 8}; 
            // while (rowStream >> cell) {
            //     row.push_back(cell);
            // }

            row = parse_fixed_width_row(line, widths);


            if (!row.empty()) {
                table.push_back(row);
                // outtable << vector_to_string(row) << endline;
            } else {
                cerr << "❌ Skipped malformed row\n";
            }
        } else {
            header_lines.push_back(line);
        }
        auto current_pos = infile.tellg();
        if (current_pos != -1) {
            float progress = static_cast<float>(current_pos) / fileSize;
            int current_percent = static_cast<int>(progress * 100);
            if (current_percent != last_percent) {
                print_progress_bar(progress, start_time);
                last_percent = current_percent;
            }
        }
    }
    cout<<endl;
    infile.close();

    string status;
    stable_sort(table.begin(), table.end(), [&status](const vector<string>& a, const vector<string>& b) {
//         try {
//         cout<<"sorting these: "<< a[0] << " and " << b[0] <<endl;
        if (is_numeric(a[0]) && is_numeric(b[0])) {
            status = "SUCCESS";
            return abs(stoi(a[0])) < abs(stoi(b[0]));
        }
//         } catch (const std::exception& e) {
//             cerr << "Sort error: " << e.what() << "\n";
//         }
        status = "failed";
        return false;
    });

    ofstream outfile(filename_sorted);
    if (!outfile) {
        cerr << "Error opening output file: " << filename_sorted << "\n";
        return false;
    }

    for (const auto& h : header_lines) outfile << h << endline;
    write_table(outfile, table);
    for (const auto& f : footer_lines) outfile << f << endline;
    cout << "\nSorted output file: " << filename_sorted << "\n" << endl;
    cout << status << endl;
    return true;
}
//end of sorting

// testing input orientations if only contains indices and minus sign
bool is_new_orientation(const string& str) {
    int i=0;
    for (char c : str) {
        if ((c != '0') && (c != '1') && (c != '-')) {
            return false;
        }
        else if ((c == '0') || (c == '1')) {
            i++;
        }
    }
    if (i==3) return true;
    else {
        cout << "\nIncorrect orientation matrices!\n"<<endl;
        return false;
    }
}



vector<float> parse_string(const string& s) {
    vector<float> row;
    for (int i=0; i < s.size(); i++) {
        if (s[i] == '-') {
            if (i+1 < s.size()){
                string str; 
                str += s[i];
                if (isdigit(s[i+1])) {
                    str += s[i+1];
                    row.push_back(stof(str)); 
                    ++i;
                }
            } 
        } 
        else if (isdigit(s[i])) { 
            string str; 
            str += s[i];
            row.push_back(stof(str)); 
        }
    }
    return row;
}

vector<vector<float>> concatenate_to_matrix(char*& new_a, char*& new_b, char*& new_c) {
    // Create 3x3 matrix by concatenating vectors as rows
    vector<vector<float>> matrix;
    matrix.push_back(parse_string(new_a));
    matrix.push_back(parse_string(new_b));
    matrix.push_back(parse_string(new_c));
    return matrix;
}

void printVector(const vector<float>& Vec) {
    if (Vec.empty()) return;

    // Determine column width (for alignment)
    size_t col_width = 4;
    for (float val : Vec) {
        col_width = max(col_width, to_string(val).length());
    }

    // Top border
    cout << "+";
    for (size_t i = 0; i < Vec.size(); ++i) {
        cout << string(col_width + 2, '-') << "+";
    }
    cout << "\n";

    // Content row
    cout << "| ";
    for (float val : Vec) {
        cout << fixed << setprecision(3) << setw(col_width) << val << " | ";
    }
    cout << "\n";

    // Bottom border
    cout << "+";
    for (size_t i = 0; i < Vec.size(); ++i) {
        cout << string(col_width + 2, '-') << "+";
    }
    cout << "\n";
}

string normalize_line_endings(const string& input) {
    string output;
    for (size_t i = 0; i < input.size(); ++i) {
        if (input[i] == '\r') {
            if (i + 1 < input.size() && input[i+1] == '\n') ++i; // skip \n in \r\n
            output += '\n';
        } else {
            output += input[i];
        }
    }
    return output;
}

void printLine_details(string& line) {
    std::cout << "Line length: " << line.length() << "\n";
    for (char c : line) {
        if (c == '\0') std::cout << "[NULL]";
        else if (c == '\n') std::cout << "[LF]";
        else if (c == '\r') std::cout << "[CR]";
        else std::cout << c;
    }
    std::cout << std::endl;
}

void printMatrix(const vector<vector<float>>& matrix) {
    if (matrix.empty()) return;

    // Calculate column widths
    size_t col_width = 3; // Minimum width for single-digit numbers
    for (const auto& row : matrix) {
        for (float val : row) {
            col_width = max(col_width, to_string(val).length());
        }
    }

    // Top border
    cout << "+";
    for (size_t i = 0; i < matrix[0].size(); ++i) {
        cout << string(col_width + 2, '-') << "+";
    }
    cout << "\n";

    // Matrix content
    for (const auto& row : matrix) {
        cout << "| ";
        for (float val : row) {
            cout << fixed << setprecision(0) << setw(col_width) << val << " | ";
        }
        cout << "\n";

        // Middle border (except after last row)
        if (&row != &matrix.back()) {
            cout << "+";
            for (size_t i = 0; i < row.size(); ++i) {
                cout << string(col_width + 2, '-') << "+";
            }
            cout << "\n";
        }
    }

    // Bottom border
    cout << "+";
    for (size_t i = 0; i < matrix[0].size(); ++i) {
        cout << string(col_width + 2, '-') << "+";
    }
    cout << "\n";
}

float multiply_matrices(vector<float>& rowMatrix, vector<float>& columnMatrix) {
    
    // cout<< "row matrix size : " << rowMatrix.size() << endl;
    // cout<< "row matrix [0] : " << rowMatrix[0].size() << endl;
    // cout<< "column matrix [0] : " << columnMatrix[0].size() << endl;
    // cout<< "column matrix  : " << columnMatrix.size() << endl;
    if (rowMatrix.size() == 0 || columnMatrix.size() == 0 || rowMatrix.size() != columnMatrix.size()) {
        throw invalid_argument("Invalid matrix dimensions for multiplication");
    }

    double result = 0.0;
    for (size_t i = 0; i < rowMatrix.size(); ++i) {
        result += rowMatrix[i] * columnMatrix[i];
    }
    return result;
}

bool has_line_endings_or_null(const std::string& line) {
    for (char c : line) {
        if (c == '\n' || c == '\r' || c == '\0') {
            return true;
        }
    }
    return false;
}

bool reindex(string& filename,vector<vector<float>>& matrix) {

    // Open file specified as first argument
    ifstream file(filename, ios::binary);
    if (!file.is_open()) {
        cerr << "Error opening file: " << filename << endl;
        return 1;
    }
    string reIDX_suffix = "_reIDX";
    string output_filename = make_reindexed_filename(filename, reIDX_suffix);
    ofstream outputFile(output_filename);  

    // Read entire file into memory for fastest processing
    file.seekg(0, ios::end);
    size_t fileSize = file.tellg();
    file.seekg(0, ios::beg);
    
    vector<char> buffer(fileSize + 1);
    file.read(buffer.data(), fileSize);
    buffer[fileSize] = '\0';
    file.close();

    char* current = buffer.data();
    char* end = buffer.data() + fileSize;
    size_t linesAfterMarker = 0;
    bool markerFound = false;
    bool endingFound = false;
    string marker = "!END_OF_HEADER";
    string ender = "!END_OF_DATA";
    // string value;
    int last_percent = -1;
    // float last_progress = -1.0f;
    auto start_time = steady_clock::now();
    // cout << "current of " << filename<< " is |" << current << "|"<< endl;
    while (current < end) {

        char* lineStart = current;
        

        // Find end of current line
        while (current < end && *current != '\n') current++;
        // Handle Windows CRLF (\r\n)
        // if (current < end && *current == '\r') {
        //     current++;
        //     if (current < end && *current == '\n') {
        //         current++; // Skip the \n
        //     }
        // } else if (current < end && *current == '\n') {
        //     current++; // Unix-style
        // }
        
        size_t lineLength = current - lineStart;
        string line(lineStart, lineLength);
        
        // line = normalize_line_endings(line);
        // printLine_details(line);
        // if (has_line_endings_or_null(line)) {
        //     std::cout << "Line contains line endings or null byte:" << line << std::endl;
        // }
    

        // check if end of file reached before incrementing nb of lines, and break the loop
        if (!endingFound) {
            if (line.find(ender) != string::npos) {
                endingFound = true;
                outputFile << line << endline;
                outputFile.close();
                // cout<<"\nreached the end of file and reindexing"<<endl;
                break;
            }
        }

        // Check if line contains marker
        if (!markerFound) {
            if (line.find("!UNIT_CELL_CONSTANTS=") != string::npos) {
/*
!UNIT_CELL_CONSTANTS=    59.887    58.753    62.683 112.221  89.770 121.124|
000000000000000000000   aaaaaaa   bbbbbbb   ccccccc alphaal betabet gammaga|   
       0 : 21        ---  1:7  ---   2:7 ---  3:7  -  4:7  -  5:7  -  6:7  | */
//                 cout <<"Found unit cells"<<endl;
                istringstream rowStream(line);
                vector<string> row;
                string value;
                while (rowStream >> value) {
                    row.push_back(value);
                }
                vector<string> new_row;
                // Read the line as a single string
                string fullRow = line;
                getline(rowStream, fullRow);
                // Extract first a,b,c
                float a = stof(fullRow.substr(23, min(8, (int)fullRow.length())));
                float b = stof(fullRow.substr(33, min(8, (int)fullRow.length())));
                float c = stof(fullRow.substr(43, min(8, (int)fullRow.length())));

                vector<float> old_abc = {{a,b,c}};
                cout<< "\nOld indexing:" << endl;
                printVector(old_abc);
                cout<< "\nMultiplied by"<<endl;
                printMatrix(matrix);
                vector<float> new_abc;
                for (int i =0 ; i<3 ; i++) {
                    vector<float> myvector;
                    for (int j : matrix[i]) {
                        myvector.push_back(j);
                    }
                    new_abc.push_back(multiply_matrices(old_abc, myvector));
                }
                cout<<"\nNew orientation: "<<endl;
                printVector(new_abc);
                float new_a = abs(new_abc[0]);
                float new_b = abs(new_abc[1]);
                float new_c = abs(new_abc[2]);


                push_back_padded(new_row, row[0],21,3);
                push_back_padded(new_row, toStringWithPrecision(new_a,3),7,3);
                push_back_padded(new_row, toStringWithPrecision(new_b,3),7,3);
                push_back_padded(new_row, toStringWithPrecision(new_c,3),7,1);
                push_back_padded(new_row, row[4],7,1);
                push_back_padded(new_row, row[5],7,1);
                push_back_padded(new_row, row[6],7,0);
//                 new_row.push_back(endline);
                line = vector_to_string(new_row);
                // printLine_details(line);
                // std::cout << fullRow << std::endl;
                // std::cout << line << std::endl;
                // line = normalize_line_endings(line);
                // return true;
            }

            if (line.find(marker) != string::npos) {
                markerFound = true;
                // line = normalize_line_endings(line);
                outputFile << line <<endline ;
                current++; // move to next line
                cout<<"\n|Reindexing|"<<endl;
                continue;
            }
            
            outputFile << line << endline ;
            // current++;

        } 

        else {
/*
example for padding
   0      1   2       3           4           5       6       7       8       9   10     11
 hhhhh kkkkk lllll IOBSIOBSIO SIGMASIGMA  XDXDXD  YDYDYD   ZDZDZD   RLPRLPR PEA  CC  PSIPSI
     0    -2     0  8.247E+01  2.804E+01  1488.9  1697.4     62.4   0.01946 100  29  -74.94
   5  -  5  -  5  -    10    -    10    --  6   --  6   ---  6   ---   7   - 3 -- 2--  6         */
            istringstream rowStream(line);
            vector<string> row;
            string value;
            while (rowStream >> value) {
                row.push_back(value);
            }
            // Read the line as a single string
            string fullRow = line;
            getline(rowStream, fullRow);
            // Extract first a,b,c
            float a = stof(fullRow.substr(1, min(5, (int)fullRow.length())));
            float b = stof(fullRow.substr(7, min(5, (int)fullRow.length())));
            float c = stof(fullRow.substr(13, min(5, (int)fullRow.length())));

            linesAfterMarker++;

            // // Spinner setup
            // const char spinner[] = "|/-\\";
            // int i = 0;  // Spinner index
            
            vector<float> old_abc = {{a,b,c}};

            vector<float> new_abc;
            for (int i =0 ; i<3 ; i++) {
                vector<float> myvector;
                for (int j : matrix[i]) {
                    myvector.push_back(j);
                }
                new_abc.push_back(multiply_matrices(old_abc, myvector));
            }
            float new_a = new_abc[0];
            float new_b = new_abc[1];
            float new_c = new_abc[2];

            vector<string> new_row;
            new_row.push_back(" "); 
            push_back_padded(new_row, toStringWithPrecision(new_a,0),5,1);
            push_back_padded(new_row, toStringWithPrecision(new_b,0),5,1);
            push_back_padded(new_row, toStringWithPrecision(new_c,0),5,1);
            push_back_padded(new_row, row[3],10,1);
            push_back_padded(new_row, row[4],10,2);
            push_back_padded(new_row, row[5],6,2);
            push_back_padded(new_row, row[6],6,3);
            push_back_padded(new_row, row[7],6,3);
            push_back_padded(new_row, row[8],7,1);
            push_back_padded(new_row, row[9],3,1);
            push_back_padded(new_row, row[10],3,2);
            push_back_padded(new_row, row[11],6,0);
            new_row.push_back(endline); 
            // line = normalize_line_endings(line);

            line = "";

            // // Spinner animation
            // cout << "\r" << "Loading ... " << "\r" << spinner[i % 2] << flush;
            // i++;
        
            float progress = static_cast<float>(current  - buffer.data()) / (end - buffer.data());
            int current_percent = static_cast<int>(progress * 100);
            if (current_percent != last_percent) {
                print_progress_bar(progress+0.01, start_time);
                last_percent = current_percent;
            }
            

            string line_str = vector_to_string(new_row);
            // printLine_details(line_str);
            if (outputFile) {
                outputFile << line_str;
            } else {
                cout << output_filename << " seems to be closed!!!" << endl;
            }
        }                
        
        if (current < end) current++; // skip newline character
    }
    // cout<<endl;
    

    // Complete writing and stop spinner
    cout << "\n\nNumber of reflections is: " << linesAfterMarker << "\n" << endl;
    // cout << "\rDone !" << endl;
    
    // cout<<"last line of " << output_filename << " is : ";
    // print_last_line(output_filename);
    cout << "Output file: " << output_filename << "\n" << endl;    
    sort_and_overwrite_table_file(output_filename);

    
    return true;
}

int main(int argc, char* argv[]) {
    // string test = argv[1];
    // print_last_line(test);
    // return 0 ;
    // Check if filename was provided
    cout<<script_header;
    if ((argc != 5) || (! is_new_orientation(argv[2])) || (! is_new_orientation(argv[3])) || (! is_new_orientation(argv[4]))) {
        std::cerr << "Usage: " << argv[0] << " <filename.HKL> 100 010 001 \n or 010 100 001 (to flip h and k)\n or -100 001 010 (to flip k and l, while inverting h)" << std::endl;
        return 1;
    }
    
    std::string filename = argv[1];
    
    vector<vector<float>> orientation_matrix = concatenate_to_matrix(argv[2],argv[3],argv[4]);

    reindex(filename, orientation_matrix);

    return 0;
}

