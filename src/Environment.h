#ifndef ENVIRONMENT_H
#define ENVIRONMENT_H

/*
 * 实现环境有折磨人的一部分是，需要将 env 参数在各个方法中传递（complie, gen），不过我不清楚的是 env
 * 怎么在   createFunction 中使用
 */

#include <map>
#include <memory>
#include <string>

#include "./Logger.h"
#include "llvm/IR/Value.h"

// ! 这里有一个十分美妙的语法，使用继承来开启相应地功能
class Environment : public std::enable_shared_from_this<Environment> {
  public:
    /*
     Environment 实际上就是一个变量的对照表
     ? 哪里来这么多指针
    */
    Environment(std::map<std::string, llvm::Value *> record, std::shared_ptr<Environment> parent)
        : record_(record), parent_(parent) {}

    /*
    将变量名与值绑定在一起 */
    llvm::Value *define(const std::string &name, llvm::Value *value) {
        record_[name] = value;
        return value;
    }

    llvm::Value *lookup(const std::string &name) { return resolve(name)->record_[name]; }

  private:
    /*
     * 在作用域内寻找变量名的方法
     */
    std::shared_ptr<Environment> resolve(const std::string &name) {
        if (record_.count(name) != 0) {
            return shared_from_this();
        }

        if (parent_ == nullptr) {
            // ! 这里使用了宏定义
            DIE << "Variable \"" << name << "\" is not defined.";
        }

        return parent_->resolve(name);
    }

    /* 变量名字 和 变量值的绑定记录再 record_ 中
        TODO 这里实例变量名的下划线的使用应当和初始化参数相反
        */
    std::map<std::string, llvm::Value *> record_;

    std::shared_ptr<Environment> parent_;
};

#endif
