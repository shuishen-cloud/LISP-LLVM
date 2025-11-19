#include "./src/EvaLLVM.h"
#include <string>

int main(int argc, char const *argv[]) {
    std::string program = R"(
    
    // 测试 类型系统和变量作用域
    // (var (x string) "linux")
    // (begin
    //     (var (x string) "Hello")
    //     (printf "x: %s\n" x)
    // )
    // (printf "X: %s\n" x)
    
    // 测试 二元运算
    // (printf "x : %d\n" (eq 1 1))
    // (printf "x : %d\n" (> 1 1))
    
    // 测试 分支判断
    // (if (eq x 1) 
    //     (set x 100)
    //     (set x 200))
    // (printf "x:%d\n" x)
    
    // 测试 循环
    // (var x 0)
    
    // (while ( < x 5)
    //     (begin 
    //         (set x (+ x 1))
    //         (printf "%d " x)))
    // (printf "\n\n")
    
    // 测试 函数声明和调用
    // (def square ((x number)) -> number ( + x x))
    // (printf "square: %d\n\n" (square 2))
    (def hello (x number) (printf "Hello\n\n"))
    (def sum ((a number) (b number)) -> number (+ a b))
    (printf "sum: %d\n\n" (sum 1 2))
    (hello 1)
    )";

    EvalLLVM vm;

    vm.exec(program);

    return 0;
}
