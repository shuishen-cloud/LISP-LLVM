#ifndef EvaLLVM_h
#define EvaLLVM_h

#include <llvm/Support/raw_ostream.h>
#include <memory>
#include <string>
#include <system_error>

#include "llvm/IR/IRBuilder.h"
#include "llvm/IR/LLVMContext.h"
#include "llvm/IR/Module.h"

class EvalLLVM {
  public:
    EvalLLVM() { moduleInit(); }

    void exec(const std::string &program) {
        module->print(llvm::outs(), nullptr);
        saveModuleTofile("./out.ll");
    }

  private:
    void moduleInit() {
        /*
         * 这是一个典型的 LLVM IR表示方式
         */

        ctx = std::make_unique<llvm::LLVMContext>();
        module = std::make_unique<llvm::Module>("EvaLLVM", *ctx);
        builder = std::make_unique<llvm::IRBuilder<>>(*ctx);
    }

    void saveModuleTofile(const std::string &fileName) {
        std::error_code errorCode;
        llvm::raw_fd_ostream outll(fileName, errorCode);
        module->print(outll, nullptr);
    }

    std::unique_ptr<llvm::LLVMContext> ctx;

    std::unique_ptr<llvm::Module> module;

    std::unique_ptr<llvm::IRBuilder<>> builder;
};

#endif
