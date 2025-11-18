; ModuleID = 'EvaLLVM'
source_filename = "EvaLLVM"

@VERSION = global i32 42, align 4
@0 = private unnamed_addr constant [6 x i8] c"linux\00", align 1
@1 = private unnamed_addr constant [6 x i8] c"Hello\00", align 1
@2 = private unnamed_addr constant [7 x i8] c"x: %s\0A\00", align 1
@3 = private unnamed_addr constant [7 x i8] c"X: %s\0A\00", align 1
@4 = private unnamed_addr constant [7 x i8] c"X: %d\0A\00", align 1
@5 = private unnamed_addr constant [7 x i8] c"X: %d\0A\00", align 1

declare i32 @printf(ptr, ...)

define i32 @main() {
entry:
  %x = alloca ptr, align 8
  store ptr @0, ptr %x, align 8
  %x1 = alloca ptr, align 8
  store ptr @1, ptr %x1, align 8
  %x2 = load ptr, ptr %x1, align 8
  %0 = call i32 (ptr, ...) @printf(ptr @2, ptr %x2)
  %x3 = load ptr, ptr %x, align 8
  %1 = call i32 (ptr, ...) @printf(ptr @3, ptr %x3)
  store i32 100, ptr %x, align 4
  %x4 = load ptr, ptr %x, align 8
  %2 = call i32 (ptr, ...) @printf(ptr @4, ptr %x4)
  store i32 200, ptr %x, align 4
  %x5 = load ptr, ptr %x, align 8
  %3 = call i32 (ptr, ...) @printf(ptr @5, ptr %x5)
  ret i32 %3
}
