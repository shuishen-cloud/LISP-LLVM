#ifndef EvaLLVM_h
#define EvaLLVM_h

#include "llvm/IR/Verifier.h"
#include <llvm/Config/abi-breaking.h>
#include <llvm/IR/BasicBlock.h>
#include <llvm/IR/DerivedTypes.h>
#include <llvm/IR/Function.h>
#include <llvm/IR/Value.h>

#include <iostream>
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
        complie();
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

    void complie(/* AST */) {
        // 根据 规则 使用 LLVM 的内置函数创建相应的 IR
        fn = createFunction("main", llvm::FunctionType::get(builder->getInt32Ty(), false));
        auto result = gen();
        auto i32Result = builder->CreateIntCast(result, builder->getInt32Ty(), true);

        builder->CreateRet(i32Result);
    }

    llvm::Function *createFunction(const std::string &fnName, llvm::FunctionType *fnType) {
        auto fn = module->getFunction(fnName);

        if (fn == nullptr) {
            fn = createFunctionProto(fnName, fnType);
        }

        createFunctionBlock(fn);
        return fn;
    }

    llvm::Function *createFunctionProto(const std::string &fnName, llvm::FunctionType *fnType) {
        auto fn = llvm::Function::Create(fnType, llvm::Function::ExternalLinkage, fnName, *module);

        llvm::verifyFunction(*fn);

        return fn;
    }

    llvm::Value *gen() { return builder->getInt32(42); }

    void createFunctionBlock(llvm::Function *fn) {
        auto entry = createBB("entry", fn);
        builder->SetInsertPoint(entry); // ! 显式声明
    }

    llvm::BasicBlock *createBB(std::string name, llvm::Function *fn = nullptr) {
        return llvm::BasicBlock::Create(*ctx, name, fn);
    }

    llvm::Function *fn;

    std::unique_ptr<llvm::LLVMContext> ctx;

    std::unique_ptr<llvm::Module> module;

    std::unique_ptr<llvm::IRBuilder<>> builder;
};

#endif
