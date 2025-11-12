// Test program for complex division expressions
import "ifj25" for Ifj

class Program {
    static main() {
        // Test 1: (10 + 5) / 3
        var result1
        result1 = (10.0 + 5.0) / 3.0
        Ifj.write("(10 + 5) / 3 = ")
        Ifj.write(result1)
        Ifj.write("\n")

        // Test 2: 20 / (2 + 2)
        var result2
        result2 = 20.0 / (2.0 + 2.0)
        Ifj.write("20 / (2 + 2) = ")
        Ifj.write(result2)
        Ifj.write("\n")

        // Test 3: (10 * 2) / (5 - 1)
        var result3
        result3 = (10.0 * 2.0) / (5.0 - 1.0)
        Ifj.write("(10 * 2) / (5 - 1) = ")
        Ifj.write(result3)
        Ifj.write("\n")

        // Test 4: Reťazenie delení: 100 / 5 / 2
        var result4
        result4 = 100.0 / 5.0 / 2.0
        Ifj.write("100 / 5 / 2 = ")
        Ifj.write(result4)
        Ifj.write("\n")
    }
}
