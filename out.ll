; ModuleID = 'EvaLLVM'
source_filename = "EvaLLVM"

@VERSION = global i32 42, align 4
@0 = private unnamed_addr constant [8 x i8] c"x : %d\0A\00", align 1
@1 = private unnamed_addr constant [8 x i8] c"x : %d\0A\00", align 1

declare i32 @printf(ptr, ...)

define i32 @main() {
entry:
  %x = alloca i32, align 4
  store i32 1, ptr %x, align 4
  %0 = call i32 (ptr, ...) @printf(ptr @0, i1 true)
  %1 = call i32 (ptr, ...) @printf(ptr @1, i1 false)
  ret i32 %1
}
