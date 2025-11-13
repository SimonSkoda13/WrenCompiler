import "ifj25" for Ifj

class Program {
    static main() {
        Ifj.write("=== Testing Basic Numeric Operations ===\n\n")
        
        Ifj.write("--- Test 1: Addition ---\n")
        var a
        a = 10
        var b
        b = 20
        var result_add
        result_add = a + b
        Ifj.write("10 + 20 = ")
        Ifj.write(result_add)
        Ifj.write("\n")
        
        var c
        c = 5
        var d
        d = 3
        var result_add2
        result_add2 = c + d
        Ifj.write("5 + 3 = ")
        Ifj.write(result_add2)
        Ifj.write("\n")
        
        Ifj.write("\n--- Test 2: Multiplication ---\n")
        var e
        e = 7
        var f
        f = 8
        var result_mul
        result_mul = e * f
        Ifj.write("7 * 8 = ")
        Ifj.write(result_mul)
        Ifj.write("\n")
        
        var g
        g = 12
        var h
        h = 3
        var result_mul2
        result_mul2 = g * h
        Ifj.write("12 * 3 = ")
        Ifj.write(result_mul2)
        Ifj.write("\n")
        
        Ifj.write("\n--- Test 3: Subtraction ---\n")
        var i
        i = 100
        var j
        j = 37
        var result_sub
        result_sub = i - j
        Ifj.write("100 - 37 = ")
        Ifj.write(result_sub)
        Ifj.write("\n")
        
        Ifj.write("\n--- Test 4: Division ---\n")
        var k
        k = 50
        var l
        l = 5
        var result_div
        result_div = k / l
        Ifj.write("50 / 5 = ")
        Ifj.write(result_div)
        Ifj.write("\n")
        
        Ifj.write("\n--- Test 5: Mixed INT and FLOAT ---\n")
        var m
        m = 10
        var n
        n = 3.5
        var result_mixed
        result_mixed = m + n
        Ifj.write("10 + 3.5 = ")
        Ifj.write(result_mixed)
        Ifj.write("\n")
        
        var o
        o = 5.5
        var p
        p = 2
        var result_mixed2
        result_mixed2 = o * p
        Ifj.write("5.5 * 2 = ")
        Ifj.write(result_mixed2)
        Ifj.write("\n")
        
        Ifj.write("\n=== All numeric tests completed ===\n")
    }
}
