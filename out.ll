; ModuleID = 'EvaLLVM'
source_filename = "EvaLLVM"

@VERSION = global i32 123, align 4
@0 = private unnamed_addr constant [13 x i8] c"Version: %d\0A\00", align 1

declare i32 @printf(ptr, ...)

define i32 @main() {
entry:
  %0 = call i32 (ptr, ...) @printf(ptr @0, i32 123)
  ret i32 %0
}
