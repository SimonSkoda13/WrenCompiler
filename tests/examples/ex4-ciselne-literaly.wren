import "ifj25" for Ifj

class Program {
    static main() {
        // Celociselne literaly (decimalne)
        var decInt1
        decInt1 = 0
        var decInt2
        decInt2 = 42
        var decInt3
        decInt3 = 12345
        
        // Hexadecimalne literaly
        var hexNum1
        hexNum1 = 0x0
        var hexNum2
        hexNum2 = 0xFF
        var hexNum3
        hexNum3 = 0xDEADBEEF
        var hexNum4
        hexNum4 = 0xabc123
        
        // Desatinne literaly (s desatinnou ciarkou)
        var floatNum1
        floatNum1 = 0.0
        var floatNum2
        floatNum2 = 3.14159
        var floatNum3
        floatNum3 = 123.456
        
        // Exponencialne literaly (len cela cast + exponent)
        var expNum1
        expNum1 = 1e10
        var expNum2
        expNum2 = 5E3
        var expNum3
        expNum3 = 2e-3
        var expNum4
        expNum4 = 7E+2
        
        // Exponencialne literaly (desatinna cast + exponent)
        var expNum5
        expNum5 = 1.5e10
        var expNum6
        expNum6 = 3.14E-5
        var expNum7
        expNum7 = 0.25e+3
        var expNum8
        expNum8 = 2.718e0
        
        // Vypis hodnot
        Ifj.write(decInt1)
        Ifj.write("\n")
        Ifj.write(decInt2)
        Ifj.write("\n")
        Ifj.write(hexNum2)
        Ifj.write("\n")
        Ifj.write(floatNum2)
        Ifj.write("\n")
        Ifj.write(expNum1)
        Ifj.write("\n")
        Ifj.write(expNum5)
    }
}
