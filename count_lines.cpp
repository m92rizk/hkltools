#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <algorithm>
#include <chrono>


size_t countLinesAfterMarker(const std::string& filename, const std::string& marker, const std::string& ender) {
    std::ifstream file(filename, std::ios::binary);
    if (!file) {
        std::cerr << "Error opening file: " << filename << std::endl;
        return -1;
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
                break;
            }
        }

        // Check if line contains marker
        if (!markerFound) {
            size_t lineLength = current - lineStart;
            std::string line(lineStart, lineLength);
            if (line.find(marker) != std::string::npos) {
                markerFound = true;
                current++; // move to next line
                continue;
            }
        } 
        else {
            // Count lines after marker
            linesAfterMarker++;
        }
        
        if (current < end) current++; // skip newline character
    }

    if (!markerFound) {
        std::cerr << "Marker '" << marker << "' not found in file" << std::endl;
        return -1;
    }

    return linesAfterMarker;
}

int main(int argc, char* argv[]) {
    std::string filename = argv[1];
    std::string marker = "!END_OF_HEADER";
    std::string ender = "!END_OF_DATA";
    
    auto start = std::chrono::high_resolution_clock::now();
    size_t count = countLinesAfterMarker(filename, marker, ender);
    auto end = std::chrono::high_resolution_clock::now();
    
    if (count != static_cast<size_t>(-1)) {
        std::cout << "Lines after marker: " << count << std::endl;
        std::cout << "Processing time: " 
                  << std::chrono::duration_cast<std::chrono::milliseconds>(end-start).count()
                  << " ms" << std::endl;
    }
    
    return 0;
}