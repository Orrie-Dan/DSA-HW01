import os

class SparseMatrix:
    def __init__(self, rows=0, cols=0, filepath=None):
        self.rows = rows
        self.cols = cols
        self.elements = {}  
        
        if filepath:
            self.load_from_file(filepath)
    
    def load_from_file(self, filepath):
        """Load matrix from file"""
        try:
            with open(filepath, 'r') as file:
                lines = file.readlines()
            
            if not lines:
                raise ValueError(f"Input file is empty: {filepath}")
            
            # Read number of rows
            line = lines[0].strip()
            if not line.startswith("rows="):
                raise ValueError(f"Input file hss wrong format: {line}")
            self.rows = int(line[5:])+1
            
            
            # Read number of columns
            if len(lines) < 2:
                raise ValueError(f"Input file is missing column : {filepath}")
            line = lines[1].strip()
            if not line.startswith("cols="):
                raise ValueError(f"Input file has wrong format: {line}")
            self.cols = int(line[5:])+1
            
            print(f"Loading matrix from {filepath} with dimensions: {self.rows}x{self.cols}")
            
            # Read matrix entries
            for line_num, line in enumerate(lines[2:], start=3):
                line = line.strip()
                if not line:  # Skips empty lines
                    continue
                
                try:
                    row, col, value = self.parse_entry(line)
                    
                    if row < 0 or row >= self.rows or col < 0 or col >= self.cols:
                        raise IndexError(f"Index ({row},{col}) out of bounds for matrix of size {self.rows}x{self.cols}")
                    
                    self.set_element(row, col, value)
                    
                except Exception as e:
                    raise ValueError(f"Error parsing line {line_num}: {line}\nError: {str(e)}")
                    
        except FileNotFoundError:
            raise FileNotFoundError(f"Could not open file: {filepath}")
    
    def parse_entry(self, line):
        """Parse a matrix entry in format (row, col, value)"""
        if not (line.startswith('(') and line.endswith(')')):
            raise ValueError(f"Entry must be in format (row, col, value) but got: {line}")
        
        # Remove parentheses and split by comma
        content = line[1:-1]
        parts = content.split(',')
        
        if len(parts) != 3:
            raise ValueError("Entry must have three integers separated by commas")
        
        try:
            row = int(parts[0].strip())
            col = int(parts[1].strip())
            value = int(parts[2].strip())
            return row, col, value
        except ValueError:
            raise ValueError("Entry must contain valid integers")
    
    def set_element(self, row, col, value):
        """Set element at specified position"""
        if row < 0 or row >= self.rows or col < 0 or col >= self.cols:
            raise IndexError(f"Invalid index ({row},{col}) for matrix of size {self.rows}x{self.cols}")
        
        if value != 0:
            self.elements[(row, col)] = value
        else:
            # Remove zero entries
            self.elements.pop((row, col), None)
    
    def get_element(self, row, col):
        """Get element at specified position"""
        if row < 0 or row >= self.rows or col < 0 or col >= self.cols:
            raise IndexError(f"Invalid index ({row},{col}) for matrix of size {self.rows}x{self.cols}")
        
        return self.elements.get((row, col), 0)
    
    def get_rows(self):
        """Get number of rows"""
        return self.rows
    
    def get_cols(self):
        """Get number of columns"""
        return self.cols

    def transpose(self):
        result = SparseMatrix(self.cols, self.rows)

        for(row, col), value in self.elements.items():
          result.set_element(col,row,value)

        return result
    
    def __add__(self, other):
        if self.rows != other.rows or self.cols != other.cols:
            raise ValueError(f"Matrix dimensions do not match for addition: "
                           f"{self.rows}x{self.cols} vs {other.rows}x{other.cols}")
        
        print(f"Performing matrix addition of size {self.rows}x{self.cols}")
        
        result = SparseMatrix(self.rows, self.cols)
        
        # Add elements from first matrix
        for (row, col), value in self.elements.items():
            other_value = other.get_element(row, col)
            result.set_element(row, col, value + other_value)
        
        # Add elements from second matrix that don't exist in first matrix
        for (row, col), value in other.elements.items():
            if (row, col) not in self.elements:
                result.set_element(row, col, value)
        
        return result
    
    def __sub__(self, other):
        if self.rows != other.rows or self.cols != other.cols:
            raise ValueError(f"Matrix dimensions do not match for subtraction: "
                           f"{self.rows}x{self.cols} vs {other.rows}x{other.cols}")
        
        print(f"Performing matrix subtraction of size {self.rows}x{self.cols}")
        
        result = SparseMatrix(self.rows, self.cols)
        
        # Process elements from first matrix
        for (row, col), value in self.elements.items():
            other_value = other.get_element(row, col)
            result.set_element(row, col, value - other_value)
        
        # Process elements from second matrix that don't exist in first matrix
        for (row, col), value in other.elements.items():
            if (row, col) not in self.elements:
                result.set_element(row, col, -value)
        
        return result
        
    def __mul__(self, other):
        if self.cols == other.rows:
            print(f"Performing matrix multiplication {self.rows}x{self.cols} with {other.rows}x{other.cols}")
        else:
            print(f"Matrix dimensions incompatible for multiplication: {self.rows}x{self.cols} with {other.rows}x{other.cols}")
            print(f"Attempting multiplication with transposed second matrix...")
            other = other.transpose()
            print(f"New dimensions: {self.rows}x{self.cols} with {other.rows}x{other.cols}")
        
            if self.cols != other.rows:
                raise ValueError(f"Cannot multiply matrices even after transpose: {self.rows}x{self.cols} with {other.rows}x{other.cols}")
    
        result = SparseMatrix(self.rows, other.cols)
    
    
        other_by_row = {}
        for (row_b, col_b), value_b in other.elements.items():
            if row_b not in other_by_row:
                other_by_row[row_b] = []
            other_by_row[row_b].append((col_b, value_b))
        for (row_a, col_a), value_a in self.elements.items():
        
            if col_a in other_by_row:
               for col_b, value_b in other_by_row[col_a]:
                current_value = result.get_element(row_a, col_b)
                new_value = current_value + (value_a * value_b)
                result.set_element(row_a, col_b, new_value)
        return result
    
    def save_to_file(self, filename):
        try:
            with open(filename, 'w') as file:
                file.write(f"rows={self.rows}\n")
                file.write(f"cols={self.cols}\n")
                for (row, col), value in self.elements.items():
                    file.write(f"({row}, {col}, {value})\n")
            print(f"Matrix saved to {filename}")
        except Exception as e:
            raise IOError(f"Could not open file for writing: {filename}")
    
    def print_matrix(self):
        print(f"Matrix {self.rows}x{self.cols}:")
        for r in range(self.rows):
            for c in range(self.cols):
                print(self.get_element(r, c), end=" ")
            print()
    
    def print_info(self):
        print(f"Matrix info: {self.rows}x{self.cols} with {len(self.elements)} non-zero elements")


def main():
    base_path = "C:/Users/user/DSA-HW01/sample_inputs/"
    
    print("Sparse Matrix Operation Menu")
    print("1. Addition")
    print("2. Subtraction")
    print("3. Multiplication")
    print("4. Exit")
    
    try:
        choice = int(input("Enter your choice (1/2/3/4): "))
    except ValueError:
        print("Invalid choice.")
        return
    
    if choice < 1 or choice > 4:
        print("Invalid choice.")
        return
    elif choice == 4:
        print("Exiting...")
        return
    
    file1 = input("Enter first matrix file name (e.g: easy_sample_03_1.txt): ")
    file2 = input("Enter second matrix file name (e.g: easy_sample_03_2.txt): ")
    
    path1 = os.path.join(base_path, file1)
    path2 = os.path.join(base_path, file2)
    
    # Check if files exist
    if not os.path.exists(path1):
        print(f"Error: File not found: {path1}")
        return
    
    if not os.path.exists(path2):
        print(f"Error: File not found: {path2}")
        return
    
    try:
        print("\nLoading matrices...")
        matrix1 = SparseMatrix(filepath=path1)
        matrix1.print_info()
        
        matrix2 = SparseMatrix(filepath=path2)
        matrix2.print_info()
        
        print("\nPerforming operation...")
        
        if choice == 1:
            result = matrix1 + matrix2
        elif choice == 2:
            result = matrix1 - matrix2
        elif choice == 3:
            result = matrix1 * matrix2
        
        result.print_info()
        
        # Create result filename
        name1 = os.path.splitext(file1)[0]  # Remove extension
        name2 = os.path.splitext(file2)[0]  # Remove extension
        result_filename = f"result_{name1}_{name2}.txt"
        result_filepath = os.path.join(base_path, result_filename)
        
        result.save_to_file(result_filepath)
        print(f"\nOperation successful. Result saved to {result_filename}")
        
    except Exception as e:
        print(f"Error: {e}")


if __name__ == "__main__":
    main()