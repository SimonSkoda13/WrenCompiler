import "ifj25" for Ifj

class Program {
    static main() {
        Ifj.write("=== Testing 'is' operator ===\n")
        
        // Test 1: int literal is Num
        Ifj.write("\nTest 1: int literal is Num\n")
        if (42 is Num) {
            Ifj.write("PASS: int literal is Num\n")
        } else {
            Ifj.write("FAIL: int literal should be Num\n")
        }
        
        // Test 2: int variable is Num
        Ifj.write("\nTest 2: int variable is Num\n")
        var intVal
        intVal = 100
        if (intVal is Num) {
            Ifj.write("PASS: int variable is Num\n")
        } else {
            Ifj.write("FAIL: int variable should be Num\n")
        }
        
        // Test 3: float literal is Num
        Ifj.write("\nTest 3: float literal is Num\n")
        if (3.14 is Num) {
            Ifj.write("PASS: float literal is Num\n")
        } else {
            Ifj.write("FAIL: float literal should be Num\n")
        }
        
        // Test 4: float variable is Num
        Ifj.write("\nTest 4: float variable is Num\n")
        var floatVal
        floatVal = 2.718
        if (floatVal is Num) {
            Ifj.write("PASS: float variable is Num\n")
        } else {
            Ifj.write("FAIL: float variable should be Num\n")
        }
        
        // Test 5: expression result is Num
        Ifj.write("\nTest 5: expression result is Num\n")
        if (10 + 20 is Num) {
            Ifj.write("PASS: expression result is Num\n")
        } else {
            Ifj.write("FAIL: expression result should be Num\n")
        }
        
        // Test 6: null literal is Null
        Ifj.write("\nTest 6: null literal is Null\n")
        if (null is Null) {
            Ifj.write("PASS: null literal is Null\n")
        } else {
            Ifj.write("FAIL: null literal should be Null\n")
        }
        
        // Test 7: null variable is Null
        Ifj.write("\nTest 7: null variable is Null\n")
        var nullVal
        nullVal = null
        if (nullVal is Null) {
            Ifj.write("PASS: null variable is Null\n")
        } else {
            Ifj.write("FAIL: null variable should be Null\n")
        }
        
        // Test 8: null is NOT Num (negative test)
        Ifj.write("\nTest 8: null is NOT Num\n")
        if (nullVal is Num) {
            Ifj.write("FAIL: null should NOT be Num\n")
        } else {
            Ifj.write("PASS: null is not Num\n")
        }
        
        // Test 9: null is NOT String (negative test)
        Ifj.write("\nTest 9: null is NOT String\n")
        if (nullVal is String) {
            Ifj.write("FAIL: null should NOT be String\n")
        } else {
            Ifj.write("PASS: null is not String\n")
        }
        
        // Test 10: string literal is String
        Ifj.write("\nTest 10: string literal is String\n")
        if ("hello" is String) {
            Ifj.write("PASS: string literal is String\n")
        } else {
            Ifj.write("FAIL: string literal should be String\n")
        }
        
        // Test 11: string variable is String
        Ifj.write("\nTest 11: string variable is String\n")
        var strVal
        strVal = "world"
        if (strVal is String) {
            Ifj.write("PASS: string variable is String\n")
        } else {
            Ifj.write("FAIL: string variable should be String\n")
        }
        
        // Test 12: empty string is String
        Ifj.write("\nTest 12: empty string is String\n")
        var emptyStr
        emptyStr = ""
        if (emptyStr is String) {
            Ifj.write("PASS: empty string is String\n")
        } else {
            Ifj.write("FAIL: empty string should be String\n")
        }
        
        // Test 13: string is NOT Num (negative test)
        Ifj.write("\nTest 13: string is NOT Num\n")
        if (strVal is Num) {
            Ifj.write("FAIL: string should NOT be Num\n")
        } else {
            Ifj.write("PASS: string is not Num\n")
        }
        
        // Test 14: number is NOT String (negative test)
        Ifj.write("\nTest 14: number is NOT String\n")
        if (42 is String) {
            Ifj.write("FAIL: number should NOT be String\n")
        } else {
            Ifj.write("PASS: number is not String\n")
        }
        
        // Test 15: number is NOT Null (negative test)
        Ifj.write("\nTest 15: number is NOT Null\n")
        if (100 is Null) {
            Ifj.write("FAIL: number should NOT be Null\n")
        } else {
            Ifj.write("PASS: number is not Null\n")
        }
        
        Ifj.write("\n=== All tests completed ===\n")
    }
}
