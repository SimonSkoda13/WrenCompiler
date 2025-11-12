// Test program for division with mixed int/float types
import "ifj25" for Ifj

class Program {
    static main() {
        Ifj.write("=== Testing division with int and float ===\n")
        
        // Test 1: int / float
        var result1
        result1 = 10 / 2.0
        Ifj.write("10 / 2.0 = ")
        Ifj.write(result1)
        Ifj.write("\n")
        
        // Test 2: float / int
        var result2
        result2 = 10.0 / 2
        Ifj.write("10.0 / 2 = ")
        Ifj.write(result2)
        Ifj.write("\n")
        
        // Test 3: int / int
        var result3
        result3 = 10 / 2
        Ifj.write("10 / 2 = ")
        Ifj.write(result3)
        Ifj.write("\n")
        
        // Test 4: Zložitý výraz s int a float
        var result4
        result4 = (10 + 5.0) / 3
        Ifj.write("(10 + 5.0) / 3 = ")
        Ifj.write(result4)
        Ifj.write("\n")
        
        // Test 5: int / int s desatinným výsledkom
        var result5
        result5 = 7 / 2
        Ifj.write("7 / 2 = ")
        Ifj.write(result5)
        Ifj.write("\n")
        
        // Test 6: Delenie int nulou
        Ifj.write("Testing: 10 / 0\n")
        var result6
        result6 = 10 / 0
        Ifj.write("This should not print: ")
        Ifj.write(result6)
        Ifj.write("\n")
    }
}
