// Test program for division operations
import "ifj25" for Ifj

class Program {
    static main() {
        // Test 1: Základné delenie
        var a
        var b
        var result
        a = 10.0
        b = 2.0
        result = a / b
        Ifj.write("10.0 / 2.0 = ")
        Ifj.write(result)
        Ifj.write("\n")

        // Test 2: Delenie s desatinnými číslami
        var c
        var d
        var result2
        c = 7.5
        d = 2.5
        result2 = c / d
        Ifj.write("7.5 / 2.5 = ")
        Ifj.write(result2)
        Ifj.write("\n")

        // Test 3: Delenie literálov
        var result3
        result3 = 20.0 / 4.0
        Ifj.write("20.0 / 4.0 = ")
        Ifj.write(result3)
        Ifj.write("\n")

        // Test 4: Delenie s výsledkom menším ako 1
        var result4
        result4 = 3.0 / 6.0
        Ifj.write("3.0 / 6.0 = ")
        Ifj.write(result4)
        Ifj.write("\n")
    }
}
