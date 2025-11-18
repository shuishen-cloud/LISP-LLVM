#ifndef EvaLLVM_h
#define EvaLLVM_h

#include "llvm/IR/Verifier.h"
#include <llvm/Config/abi-breaking.h>
#include <llvm/IR/BasicBlock.h>
#include <llvm/IR/Constant.h>
#include <llvm/IR/DerivedTypes.h>
#include <llvm/IR/Function.h>
#include <llvm/IR/GlobalVariable.h>
#include <llvm/IR/Value.h>

#include <llvm/Support/Alignment.h>
#include <llvm/Support/Casting.h>
#include <llvm/Support/raw_ostream.h>
#include <memory>
#include <regex>
#include <string>
#include <system_error>
#include <vector>

#include "llvm/IR/IRBuilder.h"
#include "llvm/IR/LLVMContext.h"
#include "llvm/IR/Module.h"

#include "./Environment.h"
#include "./parser/EvaParser.h"

using syntax::EvaParser;
using Env = std::shared_ptr<Environment>;

class EvalLLVM {
  public:
    EvalLLVM() : parser(std::make_unique<EvaParser>()) {
        moduleInit();
        setupExternFunction();
        setupGlobalEnvironment();
    }

    void exec(const std::string &program) {
        // 将整个程序看作隐式 block
        auto ast = parser->parse("(begin" + program + ")");

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
        fn = createFunction("main", llvm::FunctionType::get(builder->getInt32Ty(), false),
                            GlobalEnv);

        // createGlobalVar("VERSION", builder->getInt32(123));

        auto result = gen(ast, GlobalEnv);
        auto i32Result = builder->CreateIntCast(result, builder->getInt32Ty(), true);

        builder->CreateRet(i32Result);
    }

    llvm::Function *createFunction(const std::string &fnName, llvm::FunctionType *fnType, Env env) {
        auto fn = module->getFunction(fnName);

        if (fn == nullptr) {
            fn = createFunctionProto(fnName, fnType, env);
        }

        createFunctionBlock(fn);
        return fn;
    }

    llvm::Function *createFunctionProto(const std::string &fnName, llvm::FunctionType *fnType,
                                        Env env) {
        auto fn = llvm::Function::Create(fnType, llvm::Function::ExternalLinkage, fnName, *module);

        llvm::verifyFunction(*fn);

        return fn;
    }

    llvm::Value *gen(const struct Exp &exp, Env env) {
        switch (exp.type) {

        case ExpType::NUMBER:
            return builder->getInt32(exp.number);

        case ExpType::STRING: {
            // 处理回车 \n
            auto re = std::regex("\\\\n");
            auto str = std::regex_replace(exp.string, re, "\n");

            return builder->CreateGlobalString(str);
        }

        case ExpType::SYMBOL: {
            if (exp.string == "true" || exp.string == "false") {
                return builder->getInt32(exp.string == "true" ? true : false);
            } else {
                // ! 这里是解决段错误的关键
                // auto global = module->getNamedGlobal(exp.string);
                // if (!global || !global->hasInitializer()) {
                //     // 处理错误：比如返回默认值或抛出异常
                //     return builder->getInt32(0);
                // }
                // // ! 这里是解决异常的关键，需要通过读取 IR 来判断
                // return module->getNamedGlobal(exp.string);

                auto varName = exp.string;
                auto value = env->lookup(varName);

                // 这个是干嘛的？
                if (auto globalVar = llvm::dyn_cast<llvm::GlobalVariable>(value)) {
                    return builder->CreateLoad(globalVar->getInitializer()->getType(), globalVar,
                                               varName.c_str());
                }
            }
        }

        case ExpType::LIST:
            auto tag = exp.list[0];

            if (tag.type == ExpType::SYMBOL) {
                auto op = tag.string;

                if (op == "var") {
                    auto varName = exp.list[1].string;

                    auto init = gen(exp.list[2], env);

                    return createGlobalVar(varName, (llvm::Constant *)init);
                } else if (op == "begin") {
                    /**
                    编译 block 内所有式子，取最后值，很符合 LISP 的设计。
                    */

                    llvm::Value *blockRes;

                    // 迭代生成，这里还是递归
                    for (int i = 1; i < exp.list.size(); i++) {
                        blockRes = gen(exp.list[i], env);
                    }

                    return blockRes;
                }

                if (op == "printf") {
                    auto printfn = module->getFunction("printf");
                    std::vector<llvm::Value *> args{};

                    for (auto i = 1; i < exp.list.size(); i++) {
                        // ! 这里使用了 递归解析，需要小心
                        args.push_back(gen(exp.list[i], env));
                    }

                    return builder->CreateCall(printfn, args);
                }
            }
        }

        return builder->getInt32(0);
    }

    llvm::GlobalVariable *createGlobalVar(const std::string &name, llvm::Constant *init) {
        module->getOrInsertGlobal(name, init->getType());

        auto variable = module->getNamedGlobal(name);
        variable->setAlignment(llvm::MaybeAlign(4));
        variable->setConstant(false);
        variable->setInitializer(init);

        return variable;
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

    void setupGlobalEnvironment() {
        std::map<std::string, llvm::Value *> globalObject{
            {"VERSION", builder->getInt32(42)},
        };

        std::map<std::string, llvm::Value *> globalRec{};

        for (auto entry : globalObject) {
            globalRec[entry.first] = createGlobalVar(entry.first, (llvm::Constant *)entry.second);
        }

        GlobalEnv = std::make_shared<Environment>(globalRec, nullptr);
    }

    llvm::Function *fn;

    std::unique_ptr<syntax::EvaParser> parser;

    std::shared_ptr<Environment> GlobalEnv;

    std::unique_ptr<llvm::LLVMContext> ctx;

    std::unique_ptr<llvm::Module> module;

    std::unique_ptr<llvm::IRBuilder<>> builder;
};

#endif
