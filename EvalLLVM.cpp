
#include "./src/EvaLLVM.h"
#include <string>

int main(int argc, char const *argv[]) {
    std::string program = R"(
    
    42

    )";

    EvalLLVM vm;

    vm.exec(program);

    return 0;
}
