// Program 2: Vypocet faktorialu (rekurzivne)
import "ifj25" for Ifj
class Program {
    // Hlavni funkce
    static main() {
        // v zakladnim zadani neumime volat funkci mimo prirazeni, proto zde prirazujeme
        // navratovou hodnotu (kazda funkce nejakou ma) napr. do "dummy" globalni promenne __d
        __d = Ifj.write("Zadejte cislo pro vypocet faktorialu: ")
        var inp 
        inp = Ifj.read_num()
        if (inp != null) {
            if (inp < 0) {
                __d = Ifj.write("Faktorial nelze spocitat!\n")
            } else {
                // Overime celociselnost
                var inpFloored 
                inpFloored = Ifj.floor(inp)
                if (inp == inpFloored) {
                    var vysl 
                    vysl = factorial(inp)
                    vysl = Ifj.str(vysl)
                    __d = Ifj.write("Vysledek: ")
                    __d = Ifj.write(vysl)
                    __d = Ifj.write("\n")
                } else {
                    __d = Ifj.write("Cislo neni cele!\n")
                }
            }
        } else {
            __d =  Ifj.write("Chyba pri nacitani celeho cisla!\n")
        }
    }
    // Funkce pro vypocet hodnoty faktorialu
    static factorial(n) {
        var result
        if (n < 2) {
            result = 1
        } else {
            var decremented_n 
            decremented_n = n - 1
            result = factorial(decremented_n)
            result = n * result
        }
        return result
    }
}
