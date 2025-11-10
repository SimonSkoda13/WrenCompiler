// Test if-else and while statements
import "ifj25" for Ifj

class Program {
    static main() {
        // Test 1: Simple if-else with literals
        var x
        x = 5
        
        if (x > 0) {
            var dm
            dm = Ifj.write("positive\n")
        } else {
            var dm
            dm = Ifj.write("negative\n")
        }
        
        // Test 2: While loop countdown
        var count
        count = 3
        while (count > 0) {
            var dm
            dm = Ifj.write(count)
            dm = Ifj.write("\n")
            count = count - 1
        }
        
        // Test 3: If with null (should go to else)
        var y
        y = null
        if (y != null) {
            var dm
            dm = Ifj.write("not null\n")
        } else {
            var dm
            dm = Ifj.write("is null\n")
        }
    }
}
