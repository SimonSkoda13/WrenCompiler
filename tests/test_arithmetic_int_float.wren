// Test for ADD, SUB, MUL with int/float mix
import "ifj25" for Ifj

class Program {
    static main() {
        Ifj.write("=== Testing ADD with INT/FLOAT ===\n")
        
        // Test 1: int + float
        var r1
        r1 = 10 + 5.0
        Ifj.write("10 + 5.0 = ")
        Ifj.write(r1)
        Ifj.write("\n")
        
        Ifj.write("=== Testing SUB with INT/FLOAT ===\n")
        
        // Test 2: int - float
        var r2
        r2 = 10 - 3.0
        Ifj.write("10 - 3.0 = ")
        Ifj.write(r2)
        Ifj.write("\n")
        
        Ifj.write("=== Testing MUL with INT/FLOAT ===\n")
        
        // Test 3: int * float
        var r3
        r3 = 10 * 2.5
        Ifj.write("10 * 2.5 = ")
        Ifj.write(r3)
        Ifj.write("\n")
    }
}
