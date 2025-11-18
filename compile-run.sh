ulimit -c unlimited

clang++ -o eva-llvm `llvm-config --cxxflags --ldflags --system-libs --libs core` -fexceptions -g EvalLLVM.cpp

./eva-llvm

lli ./out.ll