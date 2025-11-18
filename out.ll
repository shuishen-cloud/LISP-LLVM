; ModuleID = 'EvaLLVM'
source_filename = "EvaLLVM"

@VERSION = global i32 42, align 4
@0 = private unnamed_addr constant [6 x i8] c"Hello\00", align 1
@STRING = global ptr @0, align 4
@1 = private unnamed_addr constant [6 x i8] c"Linux\00", align 1
@Linux = global ptr @1, align 4
@2 = private unnamed_addr constant [12 x i8] c"String: %s\0A\00", align 1
@3 = private unnamed_addr constant [13 x i8] c"Version: %d\0A\00", align 1
@4 = private unnamed_addr constant [11 x i8] c"Linux: %s\0A\00", align 1

declare i32 @printf(ptr, ...)

define i32 @main() {
entry:
  %0 = call i32 (ptr, ...) @printf(ptr @2, ptr @0)
  %VERSION = load i32, ptr @VERSION, align 4
  %1 = call i32 (ptr, ...) @printf(ptr @3, i32 %VERSION)
  %2 = call i32 (ptr, ...) @printf(ptr @4, ptr @1)
  ret i32 %2
}
