// Test division by zero in complex expression
import "ifj25" for Ifj

class Program {
    static main() {
        Ifj.write("Testing: 10 / (5 - 5)\n")
        
        var result
        result = 10.0 / (5.0 - 5.0)
        
        Ifj.write("This should not print: ")
        Ifj.write(result)
        Ifj.write("\n")
    }
}
