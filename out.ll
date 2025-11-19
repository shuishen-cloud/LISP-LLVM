; ModuleID = 'EvaLLVM'
source_filename = "EvaLLVM"

@VERSION = global i32 42, align 4
@0 = private unnamed_addr constant [8 x i8] c"Hello\0A\0A\00", align 1
@1 = private unnamed_addr constant [10 x i8] c"sum: %d\0A\0A\00", align 1

declare i32 @printf(ptr, ...)

define i32 @main() {
entry:
  %0 = call i32 @sum(i32 1, i32 2)
  %1 = call i32 (ptr, ...) @printf(ptr @1, i32 %0)
  %2 = call i32 @hello(i32 1)
  ret i32 %2
}

define i32 @hello(i32 %x, i32 %number) {
entry:
  %x1 = alloca i32, align 4
  store i32 %x, ptr %x1, align 4
  %number2 = alloca i32, align 4
  store i32 %number, ptr %number2, align 4
  %0 = call i32 (ptr, ...) @printf(ptr @0)
  ret i32 %0
}

define i32 @sum(i32 %a, i32 %b) {
entry:
  %a1 = alloca i32, align 4
  store i32 %a, ptr %a1, align 4
  %b2 = alloca i32, align 4
  store i32 %b, ptr %b2, align 4
  %a3 = load i32, ptr %a1, align 4
  %b4 = load i32, ptr %b2, align 4
  %tmpadd = add i32 %a3, %b4
  ret i32 %tmpadd
}
