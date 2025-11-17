#ifndef EvaLLVM_h
#define EvaLLVM_h

#include "llvm/IR/Verifier.h"
#include <llvm/Config/abi-breaking.h>
#include <llvm/IR/BasicBlock.h>
#include <llvm/IR/DerivedTypes.h>
#include <llvm/IR/Function.h>
#include <llvm/IR/Value.h>

#include <llvm/Support/raw_ostream.h>
#include <memory>
#include <regex>
#include <string>
#include <system_error>
#include <vector>

#include "llvm/IR/IRBuilder.h"
#include "llvm/IR/LLVMContext.h"
#include "llvm/IR/Module.h"

#include "./parser/EvaParser.h"

using syntax::EvaParser;

class EvalLLVM {
  public:
    EvalLLVM() : parser(std::make_unique<EvaParser>()) {
        moduleInit();
        setupExternFunction();
    }

    void exec(const std::string &program) {

        // 这个  parser 是哪里来得？
        auto ast = parser->parse(program);

        complie(ast);

        // ? 这里挺挺疑惑的。
        module->print(llvm::outs(), nullptr);

        // * 将 IR 表示存储为 out.ll
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

    void complie(const struct Exp &ast) {
        // 根据 规则 使用 LLVM 的内置函数创建相应的 IR
        fn = createFunction("main", llvm::FunctionType::get(builder->getInt32Ty(), false));
        auto result = gen(ast);
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

    llvm::Value *gen(const struct Exp &exp) {
        switch (exp.type) {

        case ExpType::NUMBER:
            return builder->getInt32(exp.number);

        case ExpType::STRING: {
            // 处理回车 \n
            auto re = std::regex("\\\\n");
            auto str = std::regex_replace(exp.string, re, "\n");

            return builder->CreateGlobalStringPtr(str);
        }

        case ExpType::SYMBOL:
            return builder->getInt32(0);

        case ExpType::LIST:
            auto tag = exp.list[0];

            if (tag.type == ExpType::SYMBOL) {
                auto op = tag.string;

                if (op == "printf") {
                    auto printfn = module->getFunction("printf");
                    std::vector<llvm::Value *> args{};

                    for (auto i = 1; i < exp.list.size(); i++) {
                        // ! 这里使用了 递归解析，需要小心
                        args.push_back(gen(exp.list[i]));
                    }

                    return builder->CreateCall(printfn, args);
                }
            }
        }

        // auto str = builder->CreateGlobalString("Hello world! IR\n");

        return builder->getInt32(0);
    }

    void setupExternFunction() {
        auto bytePtrTy = builder->getInt8Ty()->getPointerTo();

        module->getOrInsertFunction("printf", llvm::FunctionType::get(
                                                  /* return type */ builder->getInt32Ty(),
                                                  /* format arg */ bytePtrTy,
                                                  /* vararg */ true));
    }

    void createFunctionBlock(llvm::Function *fn) {
        auto entry = createBB("entry", fn);
        builder->SetInsertPoint(entry); // ! 显式声明
    }

    llvm::BasicBlock *createBB(std::string name, llvm::Function *fn = nullptr) {
        return llvm::BasicBlock::Create(*ctx, name, fn);
    }

    llvm::Function *fn;

    std::unique_ptr<syntax::EvaParser> parser;

    std::unique_ptr<llvm::LLVMContext> ctx;

    std::unique_ptr<llvm::Module> module;

    std::unique_ptr<llvm::IRBuilder<>> builder;
};

#endif
