import "ifj25" for Ifj

class Program {
    static main() {
        Ifj.write("=== Comprehensive Operations Test ===\n\n")
        
        Ifj.write("--- Test 1: IS operator ---\n")
        Ifj.write("Testing: 42 is Num\n")
        if (42 is Num) {
            Ifj.write("PASS: 42 is Num\n")
        } else {
            Ifj.write("FAIL: 42 should be Num\n")
        }
        
        Ifj.write("Testing: 3.14 is Num\n")
        if (3.14 is Num) {
            Ifj.write("PASS: 3.14 is Num\n")
        } else {
            Ifj.write("FAIL: 3.14 should be Num\n")
        }
        
        Ifj.write("Testing: \"Hello\" is String\n")
        var str_test
        str_test = "Hello"
        if (str_test is String) {
            Ifj.write("PASS: \"Hello\" is String\n")
        } else {
            Ifj.write("FAIL: \"Hello\" should be String\n")
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
        Ifj.write("\"Ha\" * 3 = ")
        Ifj.write(repeat_result)
        Ifj.write("\n")
        
        var repeat_str2
        repeat_str2 = "Test "
        var repeat_result2
        repeat_result2 = repeat_str2 * 5
        Ifj.write("\"Test \" * 5 = ")
        Ifj.write(repeat_result2)
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
        
        var num5
        num5 = 10.0
        var num6
        num6 = 3
        var div_result3
        div_result3 = num5 / num6
        Ifj.write("10.0 / 3 = ")
        Ifj.write(div_result3)
        Ifj.write("\n")
        
        var num7
        num7 = 100
        var num8
        num8 = 4
        var div_result4
        div_result4 = num7 / num8
        Ifj.write("100 / 4 = ")
        Ifj.write(div_result4)
        Ifj.write("\n")
        
        Ifj.write("\n--- Test 5: Combined Operations ---\n")
        var base_str
        base_str = "Test"
        var repeated
        repeated = base_str * 2
        var suffix
        suffix = "!"
        var final_str
        final_str = repeated + suffix
        Ifj.write("\"Test\" * 2 + \"!\" = ")
        Ifj.write(final_str)
        Ifj.write("\n")
        
        var a
        a = 20
        var b
        b = 4
        var c
        c = a / b
        Ifj.write("20 / 4 = ")
        Ifj.write(c)
        if (c is Num) {
            Ifj.write(" (and it is Num)")
        }
        Ifj.write("\n")
        
        Ifj.write("\n=== All tests completed ===\n")
    }
}
