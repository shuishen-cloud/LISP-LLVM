#include "./src/EvaLLVM.h"
#include <string>

int main(int argc, char const *argv[]) {
    std::string program = R"(
        // (var VERSION 42)
        (printf "Version: %d\n" VERSION)
    )";

    EvalLLVM vm;

    vm.exec(program);

    return 0;
}
