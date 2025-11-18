#include "./src/EvaLLVM.h"
#include <string>

int main(int argc, char const *argv[]) {
    std::string program = R"(
    
    (var (x string) "linux")
    
    (begin
        (var (x string) "Hello")
        (printf "x: %s\n" x)
    )
    
    (printf "X: %s\n" x)
    
    (set x 100)
    
    (printf "X: %d\n" x)
    
    (set x 200)

    (printf "X: %d\n" x)
    
    )";

    EvalLLVM vm;

    vm.exec(program);

    return 0;
}
