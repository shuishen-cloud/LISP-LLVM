#ifndef EvaLLVM_h
#define EvaLLVM_h

#include "llvm/IR/Verifier.h"
#include <alloca.h>
#include <llvm/Config/abi-breaking.h>
#include <llvm/IR/BasicBlock.h>
#include <llvm/IR/Constant.h>
#include <llvm/IR/DerivedTypes.h>
#include <llvm/IR/Function.h>
#include <llvm/IR/GlobalVariable.h>
#include <llvm/IR/Instructions.h>
#include <llvm/IR/Type.h>
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

// 二元操作符
#define GEN_BINARY_OP(Op, varName)                                                                 \
    do {                                                                                           \
        auto op1 = gen(exp.list[1], env);                                                          \
        auto op2 = gen(exp.list[2], env);                                                          \
        return builder->Op(op1, op2, varName);                                                     \
    } while (false)
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
        varsBuilder = std::make_unique<llvm::IRBuilder<>>(*ctx);
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
                auto varName = exp.string;
                auto value = env->lookup(varName);

                if (auto localVar = llvm::dyn_cast<llvm::AllocaInst>(value)) {
                    return builder->CreateLoad(localVar->getAllocatedType(), localVar,
                                               varName.c_str());
                } else if (auto globalVar = llvm::dyn_cast<llvm::GlobalVariable>(value)) {
                    // 区分变量类型：如果是整数类型，加载其值；如果是指针类型，直接传递指针
                    auto varType = globalVar->getInitializer()->getType();
                    if (varType->isIntegerTy()) {
                        return builder->CreateLoad(varType, globalVar, varName.c_str());
                    } else {
                        return globalVar;
                    }
                } else {
                    return value;
                }
            }
        }

        case ExpType::LIST:
            auto tag = exp.list[0];

            if (tag.type == ExpType::SYMBOL) {
                auto op = tag.string;

                // * 感觉没必要写写那么多运算，日后再补
                if (op == "+") {
                    GEN_BINARY_OP(CreateAdd, "tmpadd");
                } else if (op == "-") {
                    GEN_BINARY_OP(CreateSub, "tmpsub");
                } else if (op == "*") {
                    GEN_BINARY_OP(CreateMul, "tmpmul");
                } else if (op == "/") {
                    GEN_BINARY_OP(CreateSDiv, "tmpdiv");
                } else if (op == "eq") {
                    GEN_BINARY_OP(CreateICmpEQ, "tmpcmp");
                } else if (op == ">") {
                    GEN_BINARY_OP(CreateICmpUGT, "tmpcmp");
                } else if (op == "<") {
                    GEN_BINARY_OP(CreateICmpULT, "tmpcmp");
                } else if (op == "if") {
                    auto cond = gen(exp.list[1], env);

                    auto thenBlock = createBB("then", fn);
                    auto elseBlock = createBB("else", fn);
                    auto ifEndBlock = createBB("ifend", fn);

                    // * 这一步是干什么用的？
                    builder->CreateCondBr(cond, thenBlock, elseBlock);

                    // then branch
                    builder->SetInsertPoint(thenBlock);
                    auto thenRes = gen(exp.list[2], env);
                    builder->CreateBr(ifEndBlock);

                    // else branch
                    builder->SetInsertPoint(elseBlock);
                    auto elseRes = gen(exp.list[3], env);
                    builder->CreateBr(ifEndBlock);

                    builder->SetInsertPoint(ifEndBlock);

                    auto phi = builder->CreatePHI(builder->getInt32Ty(), 2, "tmpif");

                    phi->addIncoming(thenRes, thenBlock);
                    phi->addIncoming(elseRes, elseBlock);

                    return phi;
                } else if (op == "while") {
                    auto condBlock = createBB("cond", fn);

                    // 无条件跳转到 condBlock
                    builder->CreateBr(condBlock);

                    auto bodyBlock = createBB("body", fn);
                    auto loopEndBlock = createBB("loopend", fn);

                    // 设置插入点，并进行判断
                    builder->SetInsertPoint(condBlock);
                    auto cond = gen(exp.list[1], env);

                    builder->CreateCondBr(cond, bodyBlock, loopEndBlock);

                    // fn->getBasicBlockiList().push_back(bodyBlock);
                    builder->SetInsertPoint(bodyBlock);
                    gen(exp.list[2], env);
                    builder->CreateBr(condBlock);

                    // fn->getBasicBlockiList().push_back(loopEndBlock);
                    builder->SetInsertPoint(loopEndBlock);

                    llvm::LLVMContext &Ctx = fn->getContext();
                    auto *MyBlock = llvm::BasicBlock::Create(Ctx, "Myblock");

                    return builder->getInt32(0);
                } else if (op == "def") {
                    return complieFunction(exp, exp.list[1].string, env);
                }

                if (op == "var") {
                    /**
                     * (var (x number) 1)
                     */

                    auto varNameDecl = exp.list[1];
                    auto varName = extractVarName(varNameDecl);

                    auto init = gen(exp.list[2], env);

                    auto varTy = extractVarType(varNameDecl);

                    auto varBinding = allocVar(varName, varTy, env);

                    return builder->CreateStore(init, varBinding);
                } else if (op == "set") {
                    auto value = gen(exp.list[2], env);

                    auto varName = exp.list[1].string;

                    auto varBinding = env->lookup(varName);

                    builder->CreateStore(value, varBinding);

                    return value;
                } else if (op == "begin") {
                    /**
                    编译 block 内所有式子，取最后值，很符合 LISP 的设计。
                    */

                    // ! 创建新环境
                    auto blockEnv =
                        std::make_shared<Environment>(std::map<std::string, llvm::Value *>(), env);

                    llvm::Value *blockRes;

                    // 迭代生成，这里还是递归
                    for (int i = 1; i < exp.list.size(); i++) {
                        blockRes = gen(exp.list[i], blockEnv);
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
                } else {
                    auto callable = gen(exp.list[0], env);

                    std::vector<llvm::Value *> args{};

                    for (auto i = 1; i < exp.list.size(); i++) {
                        args.push_back(gen(exp.list[i], env));
                    }

                    // auto fn = (llvm::Function *)callable;

                    return builder->CreateCall((llvm::Function *)callable, args);
                }
            }
        }

        return builder->getInt32(0);
    }

    bool hasReturnType(const Exp &fnExp) {
        return fnExp.list[3].type == ExpType::SYMBOL && fnExp.list[3].string == "->";
    }

    llvm::FunctionType *extractFunctionType(const Exp &fnExp) {
        auto paramas = fnExp.list[2];

        auto returnType =
            hasReturnType(fnExp) ? getTypeFromString(fnExp.list[4].string) : builder->getInt32Ty();

        std::vector<llvm::Type *> paramTypes{};

        for (auto &para : paramas.list) {
            auto paraTy = extractVarType(para);
            paramTypes.push_back(paraTy);
        }

        return llvm::FunctionType::get(returnType, paramTypes, false);
    }

    llvm::Value *complieFunction(const Exp &fnExp, std::string fnName, Env env) {
        auto params = fnExp.list[2];

        // 需要根据是否显式指定返回值类型来确定 body 所在的列表
        auto body = hasReturnType(fnExp) ? fnExp.list[5] : fnExp.list[3];

        auto prevFn = fn;
        // 使用 builder->GetInsertBlock() 来插入？
        auto prevBlock = builder->GetInsertBlock();

        // * 关注思考的层级，思考对象是否在同一层次
        auto newFn = createFunction(fnName, extractFunctionType(fnExp), env);
        // 更新 fn ?（说起来 fn 不是一个全局的变量吗？）更新是不是有些危险——果然最后还是要复位的。
        fn = newFn;

        auto idx = 0;
        // 创建新的 函数环境
        auto fnEnv = std::make_shared<Environment>(std::map<std::string, llvm::Value *>{}, env);

        // ? 为函数的参数开辟空间，这里 auto 为什么要使用 & ？
        for (auto &arg : fn->args()) {
            auto param = params.list[idx++];
            auto argName = extractVarName(param);

            arg.setName(argName);

            // allocVar 创建变量绑定并将绑定 store
            auto argBinding = allocVar(argName, arg.getType(), fnEnv);
            builder->CreateStore(&arg, argBinding);
        }

        // ? 创建返回值？
        builder->CreateRet(gen(body, fnEnv));

        // ! 复位
        builder->SetInsertPoint(prevBlock);
        fn = prevFn;

        env->define(fnName, newFn);

        return newFn;
    }

    std::string extractVarName(const Exp &exp) {
        return exp.type == ExpType::LIST ? exp.list[0].string : exp.string;
    }

    llvm::Type *extractVarType(const Exp &exp) {
        return exp.type == ExpType::LIST ? getTypeFromString(exp.list[1].string)
                                         : builder->getInt32Ty();
    }

    // * 类型系统的支持
    llvm::Type *getTypeFromString(const std::string &type_) {
        if (type_ == "number") {
            return builder->getInt32Ty();
        }

        if (type_ == "string") {
            return builder->getInt8Ty()->getPointerTo();
        }

        // 默认类型：（默认类型更应该是列表才对吧）
        return builder->getInt32Ty();
    }

    llvm::Value *allocVar(const std::string &name, llvm::Type *type_, Env env) {
        // 要插到开头，所以使用 getEntryBlock
        varsBuilder->SetInsertPoint(&fn->getEntryBlock());

        // ? 不太清楚
        auto varAlloc = varsBuilder->CreateAlloca(type_, 0, name.c_str());

        env->define(name, varAlloc);

        return varAlloc;
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

    std::unique_ptr<llvm::IRBuilder<>> varsBuilder;

    std::unique_ptr<llvm::IRBuilder<>> builder;
};

#endif
