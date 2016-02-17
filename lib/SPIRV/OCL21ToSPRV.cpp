//===- OCL21ToSPRV.cpp - Transform OCL21 to SPIR-V builtins -----*- C++ -*-===//
//
//                     The LLVM/SPIRV Translator
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
// Copyright (c) 2014 Advanced Micro Devices, Inc. All rights reserved.
//
// Permission is hereby granted, free of charge, to any person obtaining a
// copy of this software and associated documentation files (the "Software"),
// to deal with the Software without restriction, including without limitation
// the rights to use, copy, modify, merge, publish, distribute, sublicense,
// and/or sell copies of the Software, and to permit persons to whom the
// Software is furnished to do so, subject to the following conditions:
//
// Redistributions of source code must retain the above copyright notice,
// this list of conditions and the following disclaimers.
// Redistributions in binary form must reproduce the above copyright notice,
// this list of conditions and the following disclaimers in the documentation
// and/or other materials provided with the distribution.
// Neither the names of Advanced Micro Devices, Inc., nor the names of its
// contributors may be used to endorse or promote products derived from this
// Software without specific prior written permission.
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// CONTRIBUTORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS WITH
// THE SOFTWARE.
//
//===----------------------------------------------------------------------===//
//
// This file implements translation of OCL21 builtin functions.
//
//===----------------------------------------------------------------------===//
#define DEBUG_TYPE "cl21tospv"

#include "SPRVInternal.h"
#include "OCLUtil.h"
#include "llvm/ADT/StringSwitch.h"
#include "llvm/IR/InstVisitor.h"
#include "llvm/IR/Instructions.h"
#include "llvm/IR/IRBuilder.h"
#include "llvm/IR/Verifier.h"
#include "llvm/Pass.h"
#include "llvm/PassSupport.h"
#include "llvm/Support/Debug.h"
#include "llvm/Support/raw_ostream.h"

#include <set>

using namespace llvm;
using namespace SPRV;
using namespace OCLUtil;

namespace SPRV {

class OCL21ToSPRV: public ModulePass,
  public InstVisitor<OCL21ToSPRV> {
public:
  OCL21ToSPRV():ModulePass(ID), M(nullptr), Ctx(nullptr), CLVer(0) {
    initializeOCL21ToSPRVPass(*PassRegistry::getPassRegistry());
  }
  virtual bool runOnModule(Module &M);
  virtual void visitCallInst(CallInst &CI);

  /// Transform SPIR-V convert function
  //    __spirv{N}Op{ConvertOpName}(src, dummy)
  ///   =>
  ///   __spirv_{ConvertOpName}_R{TargeTyName}
  void visitCallConvert(CallInst *CI, StringRef MangledName, Op OC);

  /// Transform SPIR-V decoration
  ///   x = __spirv_{OpName};
  ///   y = __spirv{N}Op{Decorate}(x, type, value, dummy)
  ///   =>
  ///   y = __spirv_{OpName}{Postfix(type,value)}
  void visitCallDecorate(CallInst *CI, StringRef MangledName);

  /// Transform OCL C++ builtin function to SPIR-V builtin function.
  /// Assuming there is no argument changes.
  /// Should be called at last.
  void transBuiltin(CallInst *CI, Op OC);

  static char ID;
private:
  Module *M;
  LLVMContext *Ctx;
  unsigned CLVer;                   /// OpenCL version as major*10+minor
  std::set<Value *> ValuesToDelete;
};

char OCL21ToSPRV::ID = 0;

bool
OCL21ToSPRV::runOnModule(Module& Module) {
  M = &Module;
  Ctx = &M->getContext();
  CLVer = getOCLVersion(M);
  if (CLVer < kOCLVer::CL21)
    return false;

  DEBUG(dbgs() << "Enter OCL21ToSPRV:\n");
  visit(*M);

  for (auto &I:ValuesToDelete)
    if (auto Inst = dyn_cast<Instruction>(I))
      Inst->eraseFromParent();
  for (auto &I:ValuesToDelete)
    if (auto GV = dyn_cast<GlobalValue>(I))
      GV->eraseFromParent();

  DEBUG(dbgs() << "After OCL21ToSPRV:\n" << *M);
  std::string Err;
  raw_string_ostream ErrorOS(Err);
  if (verifyModule(*M, &ErrorOS)){
    DEBUG(errs() << "Fails to verify module: " << ErrorOS.str());
  }
  return true;
}

// The order of handling OCL builtin functions is important.
// Workgroup functions need to be handled before pipe functions since
// there are functions fall into both categories.
void
OCL21ToSPRV::visitCallInst(CallInst& CI) {
  DEBUG(dbgs() << "[visistCallInst] " << CI << '\n');
  auto F = CI.getCalledFunction();
  if (!F)
    return;

  auto MangledName = F->getName();
  std::string DemangledName;
  if (!oclIsBuiltin(MangledName, CLVer, &DemangledName, true))
    return;
  DEBUG(dbgs() << "DemangledName:" << DemangledName << '\n');
  StringRef Ref(DemangledName);
  assert(Ref.startswith("Op") && "Invalid builtin name");
  Ref = Ref.drop_front(2);

  Op OC = OpNop;
  if (!OpCodeNameMap::rfind(Ref.str(), &OC))
    return;
  DEBUG(dbgs() << "maps to opcode " << OC << '\n');

  if (isCvtOpCode(OC)) {
    visitCallConvert(&CI, MangledName, OC);
    return;
  }
  if (OC == OpDecorate) {
    visitCallDecorate(&CI, MangledName);
    return;
  }
  transBuiltin(&CI, OC);
}

void OCL21ToSPRV::visitCallConvert(CallInst* CI,
    StringRef MangledName, Op OC) {
  std::string TargetTyName = mapLLVMTypeToOCLType(CI->getType(),
      OC == OpSConvert || OC == OpConvertFToS || OC == OpSatConvertUToS);
  AttributeSet Attrs = CI->getCalledFunction()->getAttributes();
  mutateCallInstSPRV(M, CI, [=](CallInst *, std::vector<Value *> &Args){
    Args.pop_back();
    return getSPRVFuncName(OC, std::string("_R") + TargetTyName);
  }, &Attrs);
  ValuesToDelete.insert(CI);
  ValuesToDelete.insert(CI->getCalledFunction());
}

void OCL21ToSPRV::visitCallDecorate(CallInst* CI,
    StringRef MangledName) {
  auto Target = cast<CallInst>(CI->getArgOperand(0));
  auto F = Target->getCalledFunction();
  auto Name = F->getName().str();
  std::string DemangledName;
  oclIsBuiltin(Name, CLVer, &DemangledName);
  BuiltinFuncMangleInfo Info;
  F->setName(mangleBuiltin(DemangledName + "_" +
      getPostfix(getArgAsDecoration(CI, 1), getArgAsInt(CI, 2)),
      getTypes(getArguments(CI)), &Info));
  CI->replaceAllUsesWith(Target);
  ValuesToDelete.insert(CI);
  ValuesToDelete.insert(CI->getCalledFunction());
}

void
OCL21ToSPRV::transBuiltin(CallInst* CI, Op OC) {
  AttributeSet Attrs = CI->getCalledFunction()->getAttributes();
  assert(OC != OpExtInst && "not supported");
  mutateCallInstSPRV(M, CI, [=](CallInst *, std::vector<Value *> &Args){
    return getSPRVFuncName(OC);
  }, &Attrs);
  ValuesToDelete.insert(CI);
  ValuesToDelete.insert(CI->getCalledFunction());
}

}

INITIALIZE_PASS(OCL21ToSPRV, "cl21tospv", "Transform OCL 2.1 to SPIR-V",
    false, false)

ModulePass *llvm::createOCL21ToSPRV() {
  return new OCL21ToSPRV();
}
