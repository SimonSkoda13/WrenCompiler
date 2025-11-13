import "ifj25" for Ifj

class Program {
    static main() {
        Ifj.write("=== String Operations Test ===\n")
        
        Ifj.write("Test 1: String concatenation\n")
        var str1
        str1 = "Hello"
        var str2
        str2 = " World"
        var concat_result
        concat_result = str1 + str2
        Ifj.write(concat_result)
        Ifj.write("\n")
        
        Ifj.write("Test 2: String multiplication (int)\n")
        var str3
        str3 = "Ha"
        var count_int
        count_int = 3
        var mul_result_int
        mul_result_int = str3 * count_int
        Ifj.write(mul_result_int)
        Ifj.write("\n")
        
        Ifj.write("Test 3: Numeric addition\n")
        var num1
        num1 = 10
        var num2
        num2 = 20
        var add_result
        add_result = num1 + num2
        Ifj.write(add_result)
        Ifj.write("\n")
        
        Ifj.write("Test 4: Numeric multiplication\n")
        var num3
        num3 = 5
        var num4
        num4 = 7
        var mul_result_num
        mul_result_num = num3 * num4
        Ifj.write(mul_result_num)
        Ifj.write("\n")
        
        Ifj.write("=== All tests completed ===\n")
    }
}
