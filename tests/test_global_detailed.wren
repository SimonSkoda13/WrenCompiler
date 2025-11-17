import "ifj25" for Ifj

class Program {
    static main() {
        __global1 = 42
        __global2 = "hello"
        
        Ifj.write(__global1)
        Ifj.write("\n")
        Ifj.write(__global2)
        Ifj.write("\n")
        
        __global1 = 100
        Ifj.write(__global1)
        Ifj.write("\n")
    }
}
