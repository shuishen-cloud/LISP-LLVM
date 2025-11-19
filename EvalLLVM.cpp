#include "./src/EvaLLVM.h"
#include <string>

int main(int argc, char const *argv[]) {
    std::string program = R"(
    
    // (var (x string) "linux")
    
    // (begin
    //     (var (x string) "Hello")
    //     (printf "x: %s\n" x)
    // )
    
    // (printf "X: %s\n" x)
    
    // (set x 100)
    
    // (printf "X: %d\n" x)
    
    // (set x 200)

    // (printf "X: %d\n" x)

    // (printf "x : %d\n" (eq 1 1))
    // (printf "x : %d\n" (> 1 1))
    
    // (if (eq x 1) 
    //     (set x 100)
    //     (set x 200))
    // (printf "x:%d\n" x)
    
    (var x 0)
    
    (while ( < x 5)
        (begin 
            (set x (+ x 1))
            (printf "%d " x))
    (printf " * \n"))
    )";

    EvalLLVM vm;

    vm.exec(program);

    return 0;
}
