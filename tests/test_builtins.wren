// Test program for built-in functions
import "ifj25" for Ifj

class Program {
    static main() {
        // Test Ifj.write
        Ifj.write("=== Testing Ifj.write ===\n")
        Ifj.write("String: ")
        Ifj.write("Hello")
        Ifj.write("\n")
        Ifj.write("Integer: ")
        Ifj.write(42)
        Ifj.write("\n")
        Ifj.write("Float: ")
        Ifj.write(3.14)
        Ifj.write("\n")
        Ifj.write("Null: ")
        Ifj.write(null)
        Ifj.write("\n\n")
        
        // Test Ifj.str
        Ifj.write("=== Testing Ifj.str ===\n")
        var ifj_s1
        ifj_s1 = Ifj.str(123)
        Ifj.write("str(123) = ")
        Ifj.write(ifj_s1)
        Ifj.write("\n")
        
        var ifj_s2
        ifj_s2 = Ifj.str(3.14)
        Ifj.write("str(3.14) = ")
        Ifj.write(ifj_s2)
        Ifj.write("\n")
        
        var ifj_s3
        ifj_s3 = Ifj.str(null)
        Ifj.write("str(null) = ")
        Ifj.write(ifj_s3)
        Ifj.write("\n\n")
        
        // Test Ifj.floor
        Ifj.write("=== Testing Ifj.floor ===\n")
        var ifj_f1
        ifj_f1 = Ifj.floor(3.7)
        Ifj.write("floor(3.7) = ")
        Ifj.write(ifj_f1)
        Ifj.write("\n")
        
        var ifj_f2
        ifj_f2 = Ifj.floor(5)
        Ifj.write("floor(5) = ")
        Ifj.write(ifj_f2)
        Ifj.write("\n\n")
        
        // Test Ifj.length
        Ifj.write("=== Testing Ifj.length ===\n")
        var ifj_len
        ifj_len = Ifj.length("Hello")
        Ifj.write("length(\"Hello\") = ")
        Ifj.write(ifj_len)
        Ifj.write("\n\n")
        
        // Test Ifj.substring
        Ifj.write("=== Testing Ifj.substring ===\n")
        var ifj_sub
        ifj_sub = Ifj.substring("Hello World", 0, 5)
        Ifj.write("substring(\"Hello World\", 0, 5) = ")
        Ifj.write(ifj_sub)
        Ifj.write("\n\n")
        
        // Test Ifj.strcmp
        Ifj.write("=== Testing Ifj.strcmp ===\n")
        var ifj_cmp
        ifj_cmp = Ifj.strcmp("abc", "abc")
        Ifj.write("strcmp(\"abc\", \"abc\") = ")
        Ifj.write(ifj_cmp)
        Ifj.write("\n")
        
        ifj_cmp = Ifj.strcmp("abc", "def")
        Ifj.write("strcmp(\"abc\", \"def\") = ")
        Ifj.write(ifj_cmp)
        Ifj.write("\n\n")
        
        // Test Ifj.ord
        Ifj.write("=== Testing Ifj.ord ===\n")
        var ifj_o
        ifj_o = Ifj.ord("A", 0)
        Ifj.write("ord(\"A\", 0) = ")
        Ifj.write(ifj_o)
        Ifj.write("\n\n")
        
        // Test Ifj.chr
        Ifj.write("=== Testing Ifj.chr ===\n")
        var ifj_c
        ifj_c = Ifj.chr(65)
        Ifj.write("chr(65) = ")
        Ifj.write(ifj_c)
        Ifj.write("\n\n")
        
        Ifj.write("=== All tests completed ===\n")
    }
}
