import "ifj25" for Ifj

class Program {
  static getValue {
      return 10000
  }

  static setValue = (val) {
      Ifj.write("setter zavolany s hodnotou:\n")
      Ifj.write(val)
  }

  static getAnswer(arg) {
        if (arg is Num) {   // podmineny prikaz, operator porovnani typu
            if (arg - 1 > 10 * 84.1 / 2.4) {  // porovnavaci vyraz ma na obou stranach zase vyrazy
                // v zakladnim zadani neumime volat funkci mimo prirazeni, proto zde prirazujeme
                // navratovou hodnotu (kazda funkce nejakou ma) napr. do "dummy" globalni promenne __d
                __d = Ifj.write("Celkem velke cislo!\n")
            } else {
            }
            arg = arg * 42  // prikaz prirazeni
            var ansStr 
            ansStr = Ifj.str(arg)  // volani vestavene funkce
            return "Odpoved je " + ansStr
        } else {  // else sekce je v zakladnim zadani povinna
            __d = Ifj.write("Neplatny argument\n")
            return null
        }
    }

  static main() {
      var x 
      x = getValue
      var myValue
      myValue = getAnswer(x)
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
        Ifj.write(myValue)
  }
}