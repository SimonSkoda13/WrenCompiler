// Test program for division by zero
import "ifj25" for Ifj

class Program {
    static main() {
        Ifj.write("Testing division by zero...\n")
        
        var a
        var b
        var result
        a = 10.0
        b = 0.0
        result = a / b
        
        Ifj.write("This should not print!\n")
        Ifj.write(result)
    }
}
