#include <string>
#include <initializer_list>

#include "llvm/IR/LLVMContext.h"
#include "llvm/IR/IRBuilder.h"
#include "llvm/IR/Module.h"

#include "llvm/ExecutionEngine/GenericValue.h"
#include "llvm/ExecutionEngine/Interpreter.h"
#include "llvm/ExecutionEngine/JIT.h"

#include "llvm/Support/TargetSelect.h"

using namespace llvm;

typedef void (*thunk)();

class BFLLVM {
  Module* module;
  LLVMContext& context;
  Function* function;
  Value* cursorPointer;
  Value* cellsPointer;
  std::vector<BasicBlock*> blockStack;
  BasicBlock* currentBlock;
  Function* putchar;

  Function* DeclareFunction(const Twine& name, Type* returnType, std::initializer_list<Type*> argTypes) {
    auto funcType = FunctionType::get(returnType, std::vector<Type*>(argTypes), false);
    return Function::Create(funcType, Function::ExternalLinkage, name, module);
  }

  Type* intTy(unsigned n) {
    return Type::getIntNTy(context, n);
  }

  Type* intPtrTy(unsigned n) {
    return Type::getIntNPtrTy(context, n);
  }
  
  Type* voidTy() {
    return Type::getVoidTy(context);
  }

  std::unique_ptr<IRBuilder<>> GetBuilder() {
    return std::unique_ptr<IRBuilder<>>(new IRBuilder<>(currentBlock));
  }
  
  Value* Incr(Value* value) {
    auto one = ConstantInt::get(value->getType(), 1);
    return GetBuilder()->CreateAdd(value, one);
  }

  Value* Decr(Value* value) {
    auto one = ConstantInt::get(value->getType(), 1);
    return GetBuilder()->CreateSub(value, one);
  }

  Value* GetValuePointer() {
    auto builder = GetBuilder();
    auto cursor = builder->CreateLoad(cursorPointer, "cursor");
    auto zero = ConstantInt::get(intTy(32), 0);
    std::vector<Value*> indices { zero, cursor };
    return builder->CreateGEP(cellsPointer, indices, "valuePointer");
  }

  void CompileIncr() {
    auto builder = GetBuilder();
    auto valuePointer = GetValuePointer();
    auto value = builder->CreateLoad(valuePointer, "value");
    builder->CreateStore(Incr(value), valuePointer);
  }

  void CompileDecr() {
    auto builder = GetBuilder();
    auto valuePointer = GetValuePointer();
    auto value = builder->CreateLoad(valuePointer, "value");
    builder->CreateStore(Decr(value), valuePointer);
  }

  void CompileOpen() {
    auto check = BasicBlock::Create(context, "check", function);
    GetBuilder()->CreateBr(check);
    currentBlock = check;
    auto builder = GetBuilder();
    auto valuePointer = GetValuePointer();
    auto value = builder->CreateLoad(valuePointer, "value");
    auto zero = ConstantInt::get(intTy(8), 0);
    auto condition = builder->CreateICmpNE(value, zero, "cond");
    auto ifTrue = BasicBlock::Create(context, "ifTrue", function);
    auto ifFalse = BasicBlock::Create(context, "ifFalse", function);
    builder->CreateCondBr(condition, ifTrue, ifFalse);
    blockStack.push_back(ifFalse);
    blockStack.push_back(check);
    currentBlock = ifTrue;
  }

  void CompileClose() {
    auto check = blockStack.back();
    blockStack.pop_back();
    auto ifFalse = blockStack.back();
    blockStack.pop_back();
    GetBuilder()->CreateBr(check);
    currentBlock = ifFalse;
  }

  void CompileLeft() {
    auto builder = GetBuilder();
    auto cursor = builder->CreateLoad(cursorPointer);
    builder->CreateStore(Decr(cursor), cursorPointer);
  }

  void CompileRight() {
    auto builder = GetBuilder();
    auto cursor = builder->CreateLoad(cursorPointer);
    builder->CreateStore(Incr(cursor), cursorPointer);
  }

  void CompilePut() {
    auto builder = GetBuilder();
    auto valuePointer = GetValuePointer();
    auto value = builder->CreateLoad(valuePointer, "value");
    auto extended = builder->CreateSExt(value, intTy(32));
    builder->CreateCall(putchar, extended);
  }

  void CompileChar(char c) {
    switch(c) {
      case '+': CompileIncr(); break;
      case '-': CompileDecr(); break;
      case '[': CompileOpen(); break;
      case ']': CompileClose(); break;
      case '<': CompileLeft(); break;
      case '>': CompileRight(); break;
      case '.': CompilePut(); break;
    }
  }

  void DoPreamble() {
    putchar = DeclareFunction("putchar", intTy(32), {intTy(32)});
    function = DeclareFunction("thefunc", voidTy(), {});
    auto entry = BasicBlock::Create(context, "entry", function);
    currentBlock = entry;
    auto builder = GetBuilder();
    cursorPointer = builder->CreateAlloca(intTy(64));
    auto zero = ConstantInt::get(intTy(64), 0);
    builder->CreateStore(zero, cursorPointer);
    auto cellsType = ArrayType::get(intTy(8), 30000);
    cellsPointer = builder->CreateAlloca(cellsType);
    auto firstCell = GetValuePointer();
    auto bzero = DeclareFunction("bzero", voidTy(), {intPtrTy(8), intTy(32)});
    std::vector<Value*> bzeroArgs { firstCell, ConstantInt::get(intTy(32), 30000) };
    builder->CreateCall(bzero, bzeroArgs);
  }

  void CompileTheBf() {
    DoPreamble();
    std::string s = "++++++++++[>+++++++>++++++++++>+++>+<<<<-]>++.>+."
    "+++++++..+++.>++.<<+++++++++++++++.>.+++.------.--------.>+.>.";
    for (auto c : s) {
      CompileChar(c);
    }
    GetBuilder()->CreateRetVoid();
  }

public:
  BFLLVM(LLVMContext& contextIn) : context(contextIn) {}
  
  int run() {
    InitializeNativeTarget();
    module = new Module("mymodule", context);
    CompileTheBf();
    auto ee = EngineBuilder(module).create();
    thunk compiled = (thunk)(intptr_t)ee->getPointerToFunction(function);
    compiled();
    return 0;
  }
};

int main(int argc, char** argv) {
  return BFLLVM(getGlobalContext()).run();
}
