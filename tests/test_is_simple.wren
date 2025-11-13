import "ifj25" for Ifj

class Program {
    static main() {
        Ifj.write("Testing is operator\n")
        
        if (42 is Num) {
            Ifj.write("PASS: 42 is Num\n")
        } else {
            Ifj.write("FAIL: 42 should be Num\n")
        }
    }
}
