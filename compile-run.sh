clang++ -o eva-llvm `llvm-config --cxxflags --ldflags --system-libs --libs core` EvalLLVM.cpp

./eva-llvm

lli ./out.ll