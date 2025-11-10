import "ifj25" for Ifj

class Program {
    static main() {
        // Test 1: Global variable declaration and initialization
        var __global1
        __global1 = 42
        Ifj.write("Global1 = ")
        Ifj.write(__global1)
        Ifj.write("\n")
        
        // Test 2: Modify global variable
        __global1 = 100
        Ifj.write("Global1 modified = ")
        Ifj.write(__global1)
        Ifj.write("\n")
        
        // Test 3: Global variable with string
        var __str_global
        __str_global = "Hello from global"
        Ifj.write(__str_global)
        Ifj.write("\n")
        
        // Test 4: Mix with local variable
        var local
        local = 5
        var __global2
        __global2 = local * 2
        Ifj.write("Global2 = ")
        Ifj.write(__global2)
        Ifj.write("\n")
        
        // Test 5: Use global in expression
        var result
        result = __global1 + __global2
        Ifj.write("Result = ")
        Ifj.write(result)
        Ifj.write("\n")
    }
}
