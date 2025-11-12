// Test program for division with negative numbers
import "ifj25" for Ifj

class Program {
    static main() {
        Ifj.write("=== Testing division with negative numbers ===\n")
        
        // Test 1: Záporné / Kladné (using subtraction to create negative)
        var a1
        var b1
        var result1
        a1 = 0.0 - 10.0
        b1 = 2.0
        result1 = a1 / b1
        Ifj.write("-10.0 / 2.0 = ")
        Ifj.write(result1)
        Ifj.write("\n")
        
        // Test 2: Kladné / Záporné
        var a2
        var b2
        var result2
        a2 = 10.0
        b2 = 0.0 - 2.0
        result2 = a2 / b2
        Ifj.write("10.0 / -2.0 = ")
        Ifj.write(result2)
        Ifj.write("\n")
        
        // Test 3: Záporné / Záporné
        var a3
        var b3
        var result3
        a3 = 0.0 - 10.0
        b3 = 0.0 - 2.0
        result3 = a3 / b3
        Ifj.write("-10.0 / -2.0 = ")
        Ifj.write(result3)
        Ifj.write("\n")
        
        // Test 4: Malé záporné číslo
        var a4
        var b4
        var result4
        a4 = 0.0 - 1.5
        b4 = 0.0 - 0.5
        result4 = a4 / b4
        Ifj.write("-1.5 / -0.5 = ")
        Ifj.write(result4)
        Ifj.write("\n")
    }
}
