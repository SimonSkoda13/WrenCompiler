// Test if-else and while statements
import "ifj25" for Ifj

class Program {
    static main() {
        // Test 1: Simple if-else with literals
        var x
        x = 5
        
        if (x > 0) {
            var dm1
            dm1 = Ifj.write("positive\n")
        } else {
            var dm2
            dm2 = Ifj.write("negative\n")
        }
        
        // Test 2: While loop countdown
        var count
        count = 3
        while (count > 0) {
            var dm3
            dm3 = Ifj.write(count)
            dm3 = Ifj.write("\n")
            count = count - 1
        }
        
        // Test 3: If with null (should go to else)
        var y
        y = null
        if (y != null) {
            var dm4
            dm4 = Ifj.write("not null\n")
        } else {
            var dm5
            dm5 = Ifj.write("is null\n")
        }
    }
}
