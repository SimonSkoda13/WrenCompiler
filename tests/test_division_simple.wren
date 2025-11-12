// Simplified test for division with int/float
import "ifj25" for Ifj

class Program {
    static main() {
        Ifj.write("=== Division INT/FLOAT Test ===\n")
        
        // Test 1: int / float
        var r1
        r1 = 10 / 2.0
        Ifj.write("10 / 2.0 = ")
        Ifj.write(r1)
        Ifj.write("\n")
        
        // Test 2: float / int
        var r2
        r2 = 10.0 / 2
        Ifj.write("10.0 / 2 = ")
        Ifj.write(r2)
        Ifj.write("\n")
        
        // Test 3: int / int
        var r3
        r3 = 10 / 2
        Ifj.write("10 / 2 = ")
        Ifj.write(r3)
        Ifj.write("\n")
        
        // Test 4: int / int with decimal result
        var r4
        r4 = 7 / 2
        Ifj.write("7 / 2 = ")
        Ifj.write(r4)
        Ifj.write("\n")
    }
}
