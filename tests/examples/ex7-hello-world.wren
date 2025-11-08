import "ifj25" for Ifj
class Program {
    static unicorn {
        return "my little pony"
    }

    static add(num1) {
        Ifj.write("function add1:\n")
        var hash1
        hash1 = 17 * 31
        return num1 * hash1
    }

    static add(num1, num2) {
        Ifj.write("function add2:\n")
        var hash
        hash = 17 * 31
        return (num1 + num2) * hash
    }

    static add(num1, num2, num3) {
        Ifj.write("function add3:\n")
        return (num1 + num2 + num3)
    }

    static getString() {
        var sp
        var st
        st = 1
        sp = st
        return "string z funkcie\n"
    }

    // hlavni funkce main
    static main() {
        var myStr
        myStr = unicorn
        var added
        added = add(1)
        added = add(2,8)
        //added = add(1,2,3)
        Ifj.write(myStr)
        Ifj.write("\n")
        Ifj.write(added)
    }
}
