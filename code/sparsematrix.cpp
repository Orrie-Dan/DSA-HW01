#include <iostream>
#include <fstream>
#include <sstream>
#include <map>
#include <stdexcept>
#include <string>
#include <utility>
#include <tuple>
#include <filesystem>

class SparseMatrix {
private:
    int rows;
    int cols;
    std::map<std::pair<int, int>, int> elements; //stores non-zero elements

public:
    // Constructor to load matrix from file
    SparseMatrix(const std::string& filepath) {
        loadFromFile(filepath);
    }

    // Constructor for an empty matrix
    SparseMatrix(int rows, int cols) : rows(rows), cols(cols) {}

    // Loading matrix from file
    void loadFromFile(const std::string& filepath) {
        std::ifstream file(filepath);
        if (!file.is_open()) {
            throw std::invalid_argument("Could not open file: " + filepath);
        }

        std::string line;
        // Reading number of rows
        if (!std::getline(file, line)) {
            throw std::invalid_argument("Input file is empty " + filepath);
        }
        if (line.substr(0, 5) != "rows=") {
            throw std::invalid_argument("Input files has wrong format" + line);
        }
        rows = std::stoi(line.substr(5));

        // Reading number of columns
        if (!std::getline(file, line)) {
            throw std::invalid_argument("Input file is missing column information: " + filepath);
        }
        if (line.substr(0, 5) != "cols=") {
            throw std::invalid_argument("Input file has wrong format" + line);
        }
        cols = std::stoi(line.substr(5)) + 1;  // Added 1 to cols to handle error i was getting

        std::cout << "Loading matrix from " << filepath << " with dimensions: " << rows << "x" << cols << std::endl;

        // Reading matrix entries
        int lineCount = 2; // Already read 2 lines
        while (std::getline(file, line)) {
            lineCount++;
            if (line.empty()) continue; // Ignore empty lines
            try {
                auto entry = parseEntry(line);
                int r = std::get<0>(entry);
                int c = std::get<1>(entry);
                int v = std::get<2>(entry);
                
                if (r < 0 || r >= rows || c < 0 || c >= cols) {
                    throw std::out_of_range("Index (" + std::to_string(r) + "," + std::to_string(c) + 
                                           ") out of bounds for matrix of size " + std::to_string(rows) + 
                                           "x" + std::to_string(cols) + " at line " + std::to_string(lineCount));
                }
                
                setElement(r, c, v);
            } catch (const std::exception& e) {
                throw std::invalid_argument(std::string("Error parsing line ") + std::to_string(lineCount) + ": " + line +  "\nError: " + e.what());
            }
        }
        file.close();
    }

    // Parsing a matrix entry
    std::tuple<int, int, int> parseEntry(const std::string& line) {
        if (line.front() != '(' || line.back() != ')') {
            throw std::invalid_argument("Entry must be in format (row, col, value) but got: " + line);
        }
        std::string content = line.substr(1, line.size() - 2);
        std::stringstream ss(content);
        int r, c, v;
        char comma1, comma2;
        ss >> r >> comma1 >> c >> comma2 >> v;
        if (ss.fail() || ss.bad() || comma1 != ',' || comma2 != ',') {
            throw std::invalid_argument("Entry must have three integrs separated by commas");
        }
        return std::make_tuple(r, c, v);
    }

    // Set element at specified position
    void setElement(int row, int col, int value) {
        if (row < 0 || row >= rows || col < 0 || col >= cols) {
            throw std::out_of_range("Invalid index (" + std::to_string(row) + "," + 
                                   std::to_string(col) + ") for matrix of size " + 
                                   std::to_string(rows) + "x" + std::to_string(cols));
        }
        if (value != 0) {
            elements[{row, col}] = value;
        } else {
            elements.erase({row, col}); // Remove zero entries
        }
    }

    // Get element at specifed position
    int getElement(int row, int col) const {
        if (row < 0 || row >= rows || col < 0 || col >= cols) {
            throw std::out_of_range("Invalid index (" + std::to_string(row) + "," + 
                                   std::to_string(col) + ") for matrix of size " + 
                                   std::to_string(rows) + "x" + std::to_string(cols));
        }
        auto it = elements.find({row, col});
        return (it != elements.end()) ? it->second : 0;
    }

    // Get number of columns
    int getCols() const {
        return cols;
    }

    // Get number of rows
    int getRows() const {
        return rows;
    }

    // Matrix addition
    SparseMatrix operator+(const SparseMatrix& other) const {
        if (rows != other.rows || cols != other.cols) {
            throw std::invalid_argument("Matrix dimensions do not match for adddition: " + 
            std::to_string(rows) + "x" + std::to_string(cols) + 
                                       " vs " + std::to_string(other.rows) + "x" + 
                                       std::to_string(other.cols));
        }
        
        std::cout << "Performing matrix addition of size " << rows << "x" << cols << std::endl;
        
        SparseMatrix result(rows, cols);
        // Add elements from first matrix
        for (const auto& entry : elements) {
            int row = entry.first.first;
            int col = entry.first.second;
            int value = entry.second;
            int otherValue = other.getElement(row, col);
            result.setElement(row, col, value + otherValue);
        }
        
        // Add elements from second matrix that don't exist in first matrix
        for (const auto& entry : other.elements) {
            int row = entry.first.first;
            int col = entry.first.second;
            if (elements.find(entry.first) == elements.end()) {
                result.setElement(row, col, entry.second);
            }
        }
        return result;
    }

    // Matrix subtraction
    SparseMatrix operator-(const SparseMatrix& other) const {
        if (rows != other.rows || cols != other.cols) {
            throw std::invalid_argument("Matrix dimensions do not match for subtraction: " + 
                                       std::to_string(rows) + "x" + std::to_string(cols) + 
             " vs " + std::to_string(other.rows) + "x" + 
                                       std::to_string(other.cols));
        }
        
        std::cout << "Performing matrix subtraction of size " << rows << "x" << cols << std::endl;
        
        SparseMatrix result(rows, cols);
        // Process elements from 1st matrix
        for (const auto& entry : elements) {
            int row = entry.first.first;
            int col = entry.first.second;
            int value = entry.second;
            int otherValue = other.getElement(row, col);
            result.setElement(row, col, value - otherValue);
        }
        
        // Process elements from second matrix that don't exist in first matrix
        for (const auto& entry : other.elements) {
            int row = entry.first.first;
            int col = entry.first.second;
            if (elements.find(entry.first) == elements.end()) {
                result.setElement(row, col, -entry.second);
            }
        }
        return result;
    }

    // Matrix multiplication
    SparseMatrix operator*(const SparseMatrix& other) const {
        if (cols != other.rows) {
            throw std::invalid_argument("Matrix dimensions do not match for multiplication: " + 
                                       std::to_string(rows) + "x" + std::to_string(cols) + 
                                       " vs " + std::to_string(other.rows) + "x" + 
                                       std::to_string(other.cols));
        }
        
        std::cout << "Performing matrix multiplication " << rows << "x" << cols << 
                 " with " << other.rows << "x" << other.cols << std::endl;
        
        SparseMatrix result(rows, other.cols);
        for (const auto& entryA : elements) {
            int rowA = entryA.first.first;
            int colA = entryA.first.second;
            int valueA = entryA.second;
            
            for (const auto& entryB : other.elements) {
                int rowB = entryB.first.first;
                int colB = entryB.first.second;
                int valueB = entryB.second;
                
                if (colA == rowB) { // Check for valid multiplication
                    int currentValue = result.getElement(rowA, colB);
                    int newValue = currentValue + (valueA * valueB);
                    result.setElement(rowA, colB, newValue);
                }
            }
        }
        return result;
    }

    // Save matrix to file
    void saveToFile(const std::string& filename) const {
        std::ofstream file(filename);
        if (!file.is_open()) {
            throw std::invalid_argument("Could not open file for writing: " + filename);
        }
        file << "rows=" << rows << "\n";
        file << "cols=" << cols << "\n";
        for (const auto& entry : elements) {
            file << "(" << entry.first.first << ", " << entry.first.second << ", " << entry.second << ")\n";
        }
        file.close();
        std::cout << "Matrix saved to " << filename << std::endl;
    }

    // Print matrix (for debugging)
    void printMatrix() const {
        std::cout << "Matrix " << rows << "x" << cols << ":" << std::endl;
        for (int r = 0; r < rows; ++r) {
            for (int c = 0; c < cols; ++c) {
                std::cout << getElement(r, c) << " ";
            }
            std::cout << std::endl;
        }
    }
    
    
    void printInfo() const {
        std::cout << "Matrix info: " << rows << "x" << cols << " with " 
                 << elements.size() << " non-zero elements" << std::endl;
    }
};

int main() {
    
    std::string base_path = "C:/Users/user/DSA-HW01/sample_inputs/";  
    
    std::cout << "Sparse Matrix Operation Menu\n";
    std::cout << "1. Addition\n";
    std::cout << "2. Subtraction\n";
    std::cout << "3. Multiplication\n";
    std::cout << "4. Exit\n";
    int choice;
    std::cout << "Enter your choice (1/2/3/4): ";
    std::cin >> choice;

    if (choice < 1 || choice > 4) {
        std::cout << "Invalid choice.\n";
        return 0;
    } else if (choice == 4) {
        std::cout << "Exiting...\n";
        return 0;
    }

    std::string file1, file2;
    std::cout << "Enter first matrix file name (e.g: easy_sample_03_1.txt): ";
    std::cin >> file1;
    std::cout << "Enter second matrix file name (e.g: easy_sample_03_2.txt): ";
    std::cin >> file2;
    
    std::string path1 = base_path + file1;
    std::string path2 = base_path + file2;
    
    // Check if files exist
    std::ifstream check1(path1);
    if (!check1) {
        std::cerr << "Error: File not found: " << path1 << std::endl;
        return 1;
    }
    check1.close();
    
    std::ifstream check2(path2);
    if (!check2) {
        std::cerr << "Error: File not found: " << path2 << std::endl;
        return 1;
    }
    check2.close();

    try {
        std::cout << "\nLoading matrices..." << std::endl;
        SparseMatrix matrix1(path1);
        matrix1.printInfo();
        
        SparseMatrix matrix2(path2);
        matrix2.printInfo();
        
        SparseMatrix result(1, 1); 
        
        std::cout << "\nPerforming operation..." << std::endl;
        if (choice == 1) {
            result = matrix1 + matrix2;
        } else if (choice == 2) {
            result = matrix1 - matrix2;
        } else if (choice == 3) {
            result = matrix1 * matrix2;
        }
        
        result.printInfo();
        std::string result_filename = "result_" + file1.substr(0, file1.find('.')) + "_" + file2.substr(0, file2.find('.')) + ".txt";
        result.saveToFile(base_path + result_filename);
        std::cout << "\nOperation successful. Result saved to " << result_filename << std::endl;
    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
    }

    return 0;
}
