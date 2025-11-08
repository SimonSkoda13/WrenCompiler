import "ifj25" for Ifj

class Program {
  static getValue {
      return 42 * 75
  }

  static setValue = (val) {
      Ifj.write("setter zavolany s hodnotou:\n")
      Ifj.write(val)
  }

  static main() {
      var x 
      x = getValue
      Ifj.write(x)
      Ifj.write("\n")
      setValue = 2
  }
}