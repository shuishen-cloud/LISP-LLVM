#include "./src/EvaLLVM.h"
#include <string>

int main(int argc, char const *argv[]) {
    std::string program = R"(
        // (printf "false: %d \n" false)
        // (printf "Value: %d \n" 42)
         (printf "Version: %d\n" VERSION)
        //(var VERSION 42)
    )";

    EvalLLVM vm;

    vm.exec(program);

    return 0;
}
