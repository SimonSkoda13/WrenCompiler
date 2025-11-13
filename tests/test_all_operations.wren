import "ifj25" for Ifj

class Program {
    static main() {
        Ifj.write("=== Comprehensive Operations Test ===\n\n")
        
        Ifj.write("--- Test 1: IS operator ---\n")
        Ifj.write("Testing: 42 is Num\n")
        if (42 is Num) {
            Ifj.write("PASS: 42 is Num\n")
        } else {
            Ifj.write("FAIL\n")
        }
        
        Ifj.write("Testing: 3.14 is Num\n")
        if (3.14 is Num) {
            Ifj.write("PASS: 3.14 is Num\n")
        } else {
            Ifj.write("FAIL\n")
        }
        
        Ifj.write("\n--- Test 2: String Concatenation ---\n")
        var str1
        str1 = "Hello"
        var str2
        str2 = " "
        var str3
        str3 = "World"
        var concat_result
        concat_result = str1 + str2
        concat_result = concat_result + str3
        Ifj.write("Result: ")
        Ifj.write(concat_result)
        Ifj.write("\n")
        
        Ifj.write("\n--- Test 3: String Multiplication ---\n")
        var repeat_str
        repeat_str = "Ha"
        var count
        count = 3
        var repeat_result
        repeat_result = repeat_str * count
        Ifj.write("Ha * 3 = ")
        Ifj.write(repeat_result)
        Ifj.write("\n")
        
        Ifj.write("\n--- Test 4: Division ---\n")
        var num1
        num1 = 10
        var num2
        num2 = 2
        var div_result1
        div_result1 = num1 / num2
        Ifj.write("10 / 2 = ")
        Ifj.write(div_result1)
        Ifj.write("\n")
        
        var num3
        num3 = 7
        var num4
        num4 = 2
        var div_result2
        div_result2 = num3 / num4
        Ifj.write("7 / 2 = ")
        Ifj.write(div_result2)
        Ifj.write("\n")
        
        Ifj.write("\n=== All tests completed ===\n")
    }
}
