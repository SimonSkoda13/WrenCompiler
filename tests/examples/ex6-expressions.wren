import "ifj25" for Ifj
class Program {
    // hlavni funkce main
    static main() {
        var myValue         // definice promenne - v zakladnim zadani neni s prirazenim (var myValue = ...), 
                            // vychozi hodnota je null
        myValue = (5 + 3) * 2 // 16

        var myStr
        myStr = "super string"

        if (myValue == null) {
            __d = Ifj.write("ODPOVED NENI!\n")
            return  // konec programu
        } else {
            if (myValue is Null) {
                // tato podminka je semanticky ekvivalentni podmince myValue == null,
                // sem se proto uz nikdy nedostaneme
            } else {
            }
        }
        __d = Ifj.write(myValue)
        __d = Ifj.write("\n")

        while (valueFromUser > 0 - 2) {  // prikaz cyklu; nemame zaporne literaly, takze -2 napiseme jako (0-2)
            __d = Ifj.write("Jedeme dolu! ")
            __d = Ifj.write(valueFromUser)  // pokud byla zadana hodnota desetinna, vypise ji v hexadecimalnim formatu
            __d = Ifj.write(" -> ")
            valueFromUser = valueFromUser - 1

            var valStr 
            valStr = Ifj.str(valueFromUser)  /* pokud byla zadana hodnota desetinna, 
                                              * vypise ji v decimalnim formatu se dvema desetinnymi misty */
            __d = Ifj.write(valStr)
            __d = Ifj.write("\n")
        }
    }
}
