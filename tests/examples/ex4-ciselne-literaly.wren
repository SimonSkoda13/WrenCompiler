import "ifj25" for Ifj

class Program {
    static main() {
        // Celociselne literaly (decimalne)
        var decInt1 = 0
        var decInt2 = 42
        var decInt3 = 12345
        
        // Hexadecimalne literaly
        var hexNum1 = 0x0
        var hexNum2 = 0xFF
        var hexNum3 = 0xDEADBEEF
        var hexNum4 = 0xabc123
        
        // Desatinne literaly (s desatinnou ciarkou)
        var floatNum1 = 0.0
        var floatNum2 = 3.14159
        var floatNum3 = 123.456
        
        // Exponencialne literaly (len cela cast + exponent)
        var expNum1 = 1e10
        var expNum2 = 5E3
        var expNum3 = 2e-3
        var expNum4 = 7E+2
        
        // Exponencialne literaly (desatinna cast + exponent)
        var expNum5 = 1.5e10
        var expNum6 = 3.14E-5
        var expNum7 = 0.25e+3
        var expNum8 = 2.718e0
        
        // Vypis hodnot
        Ifj.write(decInt1)
        Ifj.write(decInt2)
        Ifj.write(hexNum2)
        Ifj.write(floatNum2)
        Ifj.write(expNum1)
        Ifj.write(expNum5)
    }
}
