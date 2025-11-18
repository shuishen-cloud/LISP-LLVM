#include "./src/EvaLLVM.h"
#include <string>

int main(int argc, char const *argv[]) {
    std::string program = R"(
        (var STRING "Hello")
        (var Linux "Linux")
        (printf "String: %s\n" STRING)
        (printf "Version: %d\n" VERSION)
        (printf "Linux: %s\n" Linux)
    )";

    EvalLLVM vm;

    vm.exec(program);

    return 0;
}
