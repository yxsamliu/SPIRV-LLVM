//===- SPRVInstruction.h � Class to represent SPIRV instruction -*- C++ -*-===//
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
/// \file
///
/// This file defines Instruction class for SPIR-V.
///
//===----------------------------------------------------------------------===//

#ifndef SPRVINSTRUCTION_HPP_
#define SPRVINSTRUCTION_HPP_

#include "SPRVEnum.h"
#include "SPRVStream.h"
#include "SPRVValue.h"
#include "SPRVBasicBlock.h"

#include <cassert>
#include <cstdint>
#include <functional>
#include <vector>
#include <map>
#include <iostream>

namespace SPRV{

class SPRVBasicBlock;
class SPRVFunction;

class SPRVComponentExecutionScope {
public:
  SPRVComponentExecutionScope(SPRVExecutionScopeKind TheScope = SPRVES_Count):
    ExecScope(TheScope){}
  SPRVExecutionScopeKind ExecScope;
};

class SPRVComponentMemorySemanticsMask {
public:
  SPRVComponentMemorySemanticsMask(SPRVWord TheSema = SPRVWORD_MAX):
    MemSema(TheSema){}
  SPRVWord MemSema;
};

class SPRVComponentOperands {
public:
  SPRVComponentOperands(){};
  SPRVComponentOperands(const std::vector<SPRVValue *> &TheOperands):
    Operands(TheOperands){};
  SPRVComponentOperands(std::vector<SPRVValue *> &&TheOperands):
    Operands(std::move(TheOperands)){};
  std::vector<SPRVValue *> getCompOperands() {
    return Operands;
  }
  std::vector<SPRVType *> getCompOperandTypes() {
    std::vector<SPRVType *> Tys;
    for (auto &I:getCompOperands())
      Tys.push_back(I->getType());
    return Tys;
  }
protected:
  std::vector<SPRVValue *> Operands;
};

class SPRVInstruction: public SPRVValue {
public:
  // Complete constructor for instruction with type and id
  SPRVInstruction(unsigned TheWordCount, SPRVOpCode TheOC, SPRVType *TheType,
      SPRVId TheId, SPRVBasicBlock *TheBB);
  // Complete constructor for instruction with module, type and id
  SPRVInstruction(unsigned TheWordCount, SPRVOpCode TheOC,
      SPRVType *TheType, SPRVId TheId, SPRVBasicBlock *TheBB,
      SPRVModule *TheBM);
  // Complete constructor for instruction with id but no type
  SPRVInstruction(unsigned TheWordCount, SPRVOpCode TheOC, SPRVId TheId,
      SPRVBasicBlock *TheBB);
  // Complete constructor for instruction without type and id
  SPRVInstruction(unsigned TheWordCount, SPRVOpCode TheOC,
      SPRVBasicBlock *TheBB);
  // Complete constructor for instruction with type but no id
  SPRVInstruction(unsigned TheWordCount, SPRVOpCode TheOC, SPRVType *TheType,
      SPRVBasicBlock *TheBB);
  // Incomplete constructor
  SPRVInstruction(SPRVOpCode TheOC):SPRVValue(TheOC), BB(NULL){}

  SPRVBasicBlock *getParent() const {return BB;}
  SPRVInstruction *getPrevious() const { return BB->getPrevious(this);}
  SPRVInstruction *getNext() const { return BB->getNext(this);}
  virtual std::vector<SPRVValue *> getOperands();
  std::vector<SPRVType*> getOperandTypes();

  void setParent(SPRVBasicBlock *);
  void setScope(SPRVEntry *);
  void addFPRoundingMode(SPRVFPRoundingModeKind Kind) {
    addDecorate(SPRVDEC_FPRoundingMode, Kind);
  }
  void eraseFPRoundingMode() {
    eraseDecorate(SPRVDEC_FPRoundingMode);
  }
  void setSaturatedConversion(bool Enable) {
    if (Enable)
      addDecorate(SPRVDEC_FPSaturatedConversion);
    else
      eraseDecorate(SPRVDEC_FPSaturatedConversion);
  }
  bool hasFPRoundingMode(SPRVFPRoundingModeKind *Kind = nullptr) {
    SPRVWord V;
    auto Found = hasDecorate(SPRVDEC_FPRoundingMode, 0, &V);
    if (Found && Kind)
      *Kind = static_cast<SPRVFPRoundingModeKind>(V);
    return Found;
  }
  bool isSaturatedConversion() {
    return hasDecorate(SPRVDEC_FPSaturatedConversion) ||
        OpCode == SPRVOC_OpSatConvertSToU ||
        OpCode == SPRVOC_OpSatConvertUToS;
  }
protected:
  void validate()const {
    SPRVValue::validate();
  }
private:
  SPRVBasicBlock *BB;
};

class SPRVMemoryAccess {
public:
  SPRVMemoryAccess(const std::vector<SPRVWord> &TheMemoryAccess):
    Alignment(0), Volatile(0) {
    MemoryAccessUpdate(TheMemoryAccess);
  }

  SPRVMemoryAccess() : Alignment(0), Volatile(0){}

  void MemoryAccessUpdate(const std::vector<SPRVWord> &MemoryAccess) {
    unsigned i = 0;
    while (i < MemoryAccess.size()) {
      if (MemoryAccess[i] == SPRVMA_Volatile)
        Volatile = MemoryAccess[i++];
      else if (MemoryAccess[i] == SPRVMA_Aligned) {
        Alignment = MemoryAccess[i + 1];
        i += 2;
      }
    }
  }
  SPRVWord isVolatile() const { return Volatile; }
  SPRVWord getAlignment() const { return Alignment; }

protected:
  SPRVWord Alignment;
  SPRVWord Volatile;
};

class SPRVVariable : public SPRVInstruction {
public:
  // Complete constructor for integer constant
  SPRVVariable(SPRVType *TheType, SPRVId TheId,
    SPRVValue *TheInitializer, const std::string &TheName,
    SPRVStorageClassKind TheStorageClass, SPRVBasicBlock *TheBB,
    SPRVModule *TheM)
    :SPRVInstruction(TheInitializer ? 5 : 4, SPRVOC_OpVariable, TheType,
        TheId, TheBB, TheM),
    StorageClass(TheStorageClass){
    if (TheInitializer)
      Initializer.push_back(TheInitializer->getId());
    Name = TheName;
    validate();
  }
  // Incomplete constructor
  SPRVVariable() :SPRVInstruction(SPRVOC_OpVariable),
      StorageClass(SPRVSC_Count){}

  SPRVStorageClassKind getStorageClass() const { return StorageClass; }
  SPRVValue *getInitializer() const {
    if (Initializer.empty())
      return nullptr;
    assert(Initializer.size() == 1);
    return getValue(Initializer[0]);
  }
  bool isConstant() const {
    return hasDecorate(SPRVDEC_Constant);
  }
  bool isBuiltin(SPRVBuiltinVariableKind *BuiltinKind = nullptr) const {
    SPRVWord Kind;
    bool Found = hasDecorate(SPRVDEC_BuiltIn, 0, &Kind);
    if (!Found)
      return false;
    if (BuiltinKind)
      *BuiltinKind = static_cast<SPRVBuiltinVariableKind>(Kind);
    return true;
  }
  void setBuiltin(SPRVBuiltinVariableKind Kind) {
    assert(isValid(Kind));
    addDecorate(new SPRVDecorate(SPRVDEC_BuiltIn, this, Kind));
  }
  void setIsConstant(bool Is) {
    if (Is)
      addDecorate(new SPRVDecorate(SPRVDEC_Constant, this));
    else
      eraseDecorate(SPRVDEC_Constant);
  }
protected:
  void validate() const {
    SPRVValue::validate();
    assert(isValid(StorageClass));
    assert(Initializer.size() == 1 || Initializer.empty());
  }
  void setWordCount(SPRVWord TheWordCount) {
    SPRVEntry::setWordCount(TheWordCount);
    Initializer.resize(WordCount - 4);
  }
  _SPRV_DEF_ENCDEC4(Type, Id, StorageClass, Initializer)

    SPRVStorageClassKind StorageClass;
  std::vector<SPRVId> Initializer;
};

class SPRVVariableArray:public SPRVInstruction {
public:
  // Complete constructor
  SPRVVariableArray(SPRVType *TheType, SPRVId TheId,
    const std::string &TheName, SPRVStorageClassKind TheStorageClass,
    SPRVWord TheLength, SPRVBasicBlock *TheBB)
    :SPRVInstruction(5, SPRVOC_OpVariableArray, TheType, TheId, TheBB),
    StorageClass(TheStorageClass), Length(TheLength){
    Name = TheName;
    validate();
    assert(TheBB && "Invalid BB");
  }
  // Incomplete constructor
  SPRVVariableArray() :SPRVInstruction(SPRVOC_OpVariableArray),
    StorageClass(SPRVSC_Count), Length(0){}

  SPRVStorageClassKind getStorageClass() const { return StorageClass; }
 
  SPRVWord getArraySize() const { return Length; }

protected:
  void validate() const {
    SPRVValue::validate();
    assert(isValid(StorageClass));
  }

  _SPRV_DEF_ENCDEC4(Type, Id, StorageClass, Length)

  SPRVStorageClassKind StorageClass;
  SPRVWord Length;
};

class SPRVStore:public SPRVInstruction, public SPRVMemoryAccess {
public:
  const static SPRVWord FixedWords = 4;
  // Complete constructor
  SPRVStore(SPRVId TheId, SPRVId PointerId, SPRVId ValueId,
      const std::vector<SPRVWord> &TheMemoryAccess, SPRVBasicBlock *TheBB)
    :SPRVInstruction(FixedWords + TheMemoryAccess.size(), SPRVOC_OpStore,
        TheId, TheBB),
     SPRVMemoryAccess(TheMemoryAccess),
     MemoryAccess(TheMemoryAccess),
     PtrId(PointerId),
     ValId(ValueId){
    setAttr();
    validate();
    assert(TheBB && "Invalid BB");
  }
  // Incomplete constructor
  SPRVStore():SPRVInstruction(SPRVOC_OpStore), SPRVMemoryAccess(),
      PtrId(SPRVID_INVALID), ValId(SPRVID_INVALID){
    setAttr();
  }

  SPRVValue *getSrc() const { return getValue(ValId);}
  SPRVValue *getDst() const { return getValue(PtrId);}
protected:
  void setAttr() {
    setHasNoType();
  }

  void setWordCount(SPRVWord TheWordCount) {
    SPRVEntry::setWordCount(TheWordCount);
    MemoryAccess.resize(TheWordCount - FixedWords);
  }
  void encode(std::ostream &O) const {
    getEncoder(O) << Id << PtrId << ValId << MemoryAccess;
  }

  void decode(std::istream &I) {
    getDecoder(I) >> Id >> PtrId >> ValId >> MemoryAccess;
    MemoryAccessUpdate(MemoryAccess);
  }

  void validate()const {
    SPRVInstruction::validate();
    if (getSrc()->isForward() || getDst()->isForward())
      return;
    assert(getValueType(PtrId)->getPointerElementType() == getValueType(ValId)
        && "Inconsistent operand types");
  }
private:
  std::vector<SPRVWord> MemoryAccess;
  SPRVId PtrId;
  SPRVId ValId;
};

class SPRVLoad:public SPRVInstruction, public SPRVMemoryAccess {
public:
  const static SPRVWord FixedWords = 4;
  // Complete constructor
  SPRVLoad(SPRVId TheId, SPRVId PointerId,
      const std::vector<SPRVWord> &TheMemoryAccess, SPRVBasicBlock *TheBB)
    :SPRVInstruction(FixedWords + TheMemoryAccess.size() , SPRVOC_OpLoad,
        TheBB->getValueType(PointerId)->getPointerElementType(), TheId, TheBB),
        SPRVMemoryAccess(TheMemoryAccess), PtrId(PointerId),
        MemoryAccess(TheMemoryAccess) {
      validate();
      assert(TheBB && "Invalid BB");
    }
  // Incomplete constructor
  SPRVLoad():SPRVInstruction(SPRVOC_OpLoad), SPRVMemoryAccess(),
      PtrId(SPRVID_INVALID){}

  SPRVValue *getSrc() const { return Module->get<SPRVValue>(PtrId);}

protected:
  void setWordCount(SPRVWord TheWordCount) {
    SPRVEntry::setWordCount(TheWordCount);
    MemoryAccess.resize(TheWordCount - FixedWords);
  }

  void encode(std::ostream &O) const {
    getEncoder(O) << Type << Id << PtrId << MemoryAccess;
  }

  void decode(std::istream &I) {
    getDecoder(I) >> Type >> Id >> PtrId >> MemoryAccess;
    MemoryAccessUpdate(MemoryAccess);
  }

  void validate()const {
    SPRVInstruction::validate();
    assert((getValue(PtrId)->isForward() ||
        Type == getValueType(PtrId)->getPointerElementType()) &&
        "Inconsistent types");
  }
private:
  SPRVId PtrId;
  std::vector<SPRVWord> MemoryAccess;
};

class SPRVBinary:public SPRVInstruction {
public:
  // Complete constructor
  SPRVBinary(SPRVOpCode TheOpCode, SPRVType *TheType, SPRVId TheId,
      SPRVId TheOp1, SPRVId TheOp2, SPRVBasicBlock *TheBB)
    :SPRVInstruction(5, TheOpCode, TheType, TheId, TheBB), Op1(TheOp1),
     Op2(TheOp2){
    validate();
    assert(TheBB && "Invalid BB");
  }
  // Incomplete constructor
  SPRVBinary(SPRVOpCode TheOpCode):SPRVInstruction(TheOpCode),
      Op1(SPRVID_INVALID), Op2(SPRVID_INVALID){}

  SPRVValue *getOperand(unsigned i) {
    assert(i <= 1);
    return getValue(i ? Op2 : Op1);
  }

  virtual std::vector<SPRVValue *> getOperands() {
    std::vector<SPRVValue *> Operands;
    Operands.push_back(getValue(Op1));
    Operands.push_back(getValue(Op2));
    return Operands;
  }

protected:
  _SPRV_DEF_ENCDEC4(Type, Id, Op1, Op2)
  void validate()const {
    SPRVType *op1Ty, *op2Ty;
    SPRVInstruction::validate();
    if (getValue(Op1)->isForward() || getValue(Op2)->isForward())
      return;
    if (getValueType(Op1)->isTypeVector()) {
      op1Ty = getValueType(Op1)->getVectorComponentType();
      op2Ty = getValueType(Op2)->getVectorComponentType();
      assert(getValueType(Op1)->getVectorComponentCount() ==
             getValueType(Op2)->getVectorComponentCount() &&
               "Inconsistent Vector component width");
    }
    else {
      op1Ty = getValueType(Op1);
      op2Ty = getValueType(Op2);
    }

    if (isBinaryOpCode(OpCode)) {
      assert(getValueType(Op1)== getValueType(Op2) &&
             "Invalid type for binary instruction");
      assert((op1Ty->isTypeInt() || op2Ty->isTypeFloat()) &&
               "Invalid type for Binary instruction");
      assert((op1Ty->getBitWidth() == op2Ty->getBitWidth()) &&
               "Inconsistent BitWidth");
    } else if (isShiftOpCode(OpCode)) {
      assert((op1Ty->isTypeInt() || op2Ty->isTypeInt()) &&
          "Invalid type for shift instruction");
    } else if (isLogicalOpCode(OpCode)) {
      assert((op1Ty->isTypeBool() || op2Ty->isTypeBool()) &&
          "Invalid type for logical instruction");
    } else if (isBitwiseOpCode(OpCode)) {
      assert((op1Ty->isTypeInt() || op2Ty->isTypeInt()) &&
          "Invalid type for bitwise instruction");
      assert((op1Ty->getIntegerBitWidth() == op2Ty->getIntegerBitWidth()) &&
          "Inconsistent BitWidth");
    } else {
      assert(0 && "Invalid op code!");
    }
  }
  SPRVId Op1;
  SPRVId Op2;
};

template<SPRVOpCode TheOpCode>
class SPRVBinaryInst:public SPRVBinary {
public:
  // Complete constructor
  SPRVBinaryInst(SPRVId TheId, SPRVType *TheType, SPRVId TheOp1, SPRVId TheOp2,
      SPRVBasicBlock *TheBB)
    :SPRVBinary(TheOpCode, TheType, TheId, TheOp1, TheOp2, TheBB){}
  // Incomplete constructor
  SPRVBinaryInst():SPRVBinary(TheOpCode){}
};

/* ToDo: SMod and FMod to be added */
#define _SPRV_OP(x) typedef SPRVBinaryInst<SPRVOC_Op##x> SPRV##x;
_SPRV_OP(IAdd)
_SPRV_OP(FAdd)
_SPRV_OP(ISub)
_SPRV_OP(FSub)
_SPRV_OP(IMul)
_SPRV_OP(FMul)
_SPRV_OP(UDiv)
_SPRV_OP(SDiv)
_SPRV_OP(FDiv)
_SPRV_OP(SRem)
_SPRV_OP(FRem)
_SPRV_OP(UMod)
_SPRV_OP(ShiftLeftLogical)
_SPRV_OP(ShiftRightLogical)
_SPRV_OP(ShiftRightArithmetic)
_SPRV_OP(LogicalAnd)
_SPRV_OP(LogicalOr)
_SPRV_OP(LogicalXor)
_SPRV_OP(BitwiseAnd)
_SPRV_OP(BitwiseOr)
_SPRV_OP(BitwiseXor)
_SPRV_OP(Dot)
#undef _SPRV_OP

template<SPRVOpCode TheOpCode>
class SPRVInstNoOperand:public SPRVInstruction {
public:
  // Complete constructor
  SPRVInstNoOperand(SPRVBasicBlock *TheBB):SPRVInstruction(1, TheOpCode,
      TheBB){
    setAttr();
    validate();
  }
  // Incomplete constructor
  SPRVInstNoOperand():SPRVInstruction(TheOpCode){
    setAttr();
  }
protected:
  void setAttr() {
    setHasNoId();
    setHasNoType();
  }
  _SPRV_DEF_ENCDEC0
};

typedef SPRVInstNoOperand<SPRVOC_OpReturn> SPRVReturn;

class SPRVReturnValue:public SPRVInstruction {
public:
  static const SPRVOpCode OC = SPRVOC_OpReturnValue;
  // Complete constructor
  SPRVReturnValue(SPRVValue *TheReturnValue, SPRVBasicBlock *TheBB)
    :SPRVInstruction(2, OC, TheBB), ReturnValueId(TheReturnValue->getId()){
    setAttr();
    validate();
    assert(TheBB && "Invalid BB");
  }
  // Incomplete constructor
  SPRVReturnValue():SPRVInstruction(OC), ReturnValueId(SPRVID_INVALID) {
    setAttr();
  }

  SPRVValue *getReturnValue() const {
    return getValue(ReturnValueId);
  }
protected:
  void setAttr() {
    setHasNoId();
    setHasNoType();
  }
  _SPRV_DEF_ENCDEC1(ReturnValueId)
  void validate()const {
    SPRVInstruction::validate();
  }
  SPRVId ReturnValueId;
};

class SPRVBranch:public SPRVInstruction {
public:
  static const SPRVOpCode OC = SPRVOC_OpBranch;
  // Complete constructor
  SPRVBranch(SPRVLabel *TheTargetLabel,SPRVBasicBlock *TheBB)
    :SPRVInstruction(2, OC, TheBB), TargetLabelId(TheTargetLabel->getId()) {
    validate();
    assert(TheBB && "Invalid BB");
  }
  // Incomplete constructor
  SPRVBranch():SPRVInstruction(OC), TargetLabelId(SPRVID_INVALID) {
    setHasNoId();
    setHasNoType();
  }
  SPRVValue *getTargetLabel() const {
    return getValue(TargetLabelId);
  }
protected:
  _SPRV_DEF_ENCDEC1(TargetLabelId)
  void validate()const {
    SPRVInstruction::validate();
    assert(WordCount == 2);
    assert(OpCode == OC);
    assert(getTargetLabel()->isLabel() || getTargetLabel()->isForward());
  }
  SPRVId TargetLabelId;
};

class SPRVBranchConditional:public SPRVInstruction {
public:
  static const SPRVOpCode OC = SPRVOC_OpBranchConditional;
  // Complete constructor
  SPRVBranchConditional(SPRVValue *TheCondition, SPRVLabel *TheTrueLabel,
      SPRVLabel *TheFalseLabel, SPRVBasicBlock *TheBB)
    :SPRVInstruction(4, OC, TheBB), ConditionId(TheCondition->getId()),
     TrueLabelId(TheTrueLabel->getId()), FalseLabelId(TheFalseLabel->getId()){
    validate();
  }
  SPRVBranchConditional(SPRVValue *TheCondition, SPRVLabel *TheTrueLabel,
      SPRVLabel *TheFalseLabel, SPRVBasicBlock *TheBB, SPRVWord TrueWeight,
      SPRVWord FalseWeight)
    :SPRVInstruction(6, OC, TheBB), ConditionId(TheCondition->getId()),
     TrueLabelId(TheTrueLabel->getId()), FalseLabelId(TheFalseLabel->getId()){
    BranchWeights.push_back(TrueWeight);
    BranchWeights.push_back(FalseWeight);
    validate();
    assert(TheBB && "Invalid BB");
  }
  // Incomplete constructor
  SPRVBranchConditional():SPRVInstruction(OC), ConditionId(SPRVID_INVALID),
      TrueLabelId(SPRVID_INVALID), FalseLabelId(SPRVID_INVALID) {
    setHasNoId();
    setHasNoType();
  }
  SPRVValue *getCondition() const {
    return getValue(ConditionId);
  }
  SPRVLabel *getTrueLabel() const {
    return get<SPRVLabel>(TrueLabelId);
  }
  SPRVLabel *getFalseLabel() const {
    return get<SPRVLabel>(FalseLabelId);
  }
protected:
  void setWordCount(SPRVWord TheWordCount) {
    SPRVEntry::setWordCount(TheWordCount);
    BranchWeights.resize(TheWordCount - 4);
  }
  _SPRV_DEF_ENCDEC4(ConditionId, TrueLabelId, FalseLabelId, BranchWeights)
  void validate()const {
    SPRVInstruction::validate();
    assert(WordCount == 4 || WordCount == 6);
    assert(WordCount == BranchWeights.size() + 4);
    assert(OpCode == OC);
    assert(getCondition()->isForward() ||
        getCondition()->getType()->isTypeBool());
    assert(getTrueLabel()->isForward() || getTrueLabel()->isLabel());
    assert(getFalseLabel()->isForward() || getFalseLabel()->isLabel());
  }
  SPRVId ConditionId;
  SPRVId TrueLabelId;
  SPRVId FalseLabelId;
  std::vector<SPRVWord> BranchWeights;
};

class SPRVPhi: public SPRVInstruction {
public:
  static const SPRVOpCode OC = SPRVOC_OpPhi;
  static const SPRVWord FixedWordCount = 3;
  SPRVPhi(SPRVType *TheType, SPRVId TheId,
      const std::vector<SPRVValue *> &ThePairs, SPRVBasicBlock *BB)
    :SPRVInstruction(ThePairs.size() + FixedWordCount, OC, TheType, TheId, BB){
    Pairs = getIds(ThePairs);
    validate();
    assert(BB && "Invalid BB");
  }
  SPRVPhi():SPRVInstruction(OC) {}
  std::vector<SPRVValue *> getPairs() {
    return getValues(Pairs);
  }
  void addPair(SPRVValue *Value, SPRVBasicBlock *BB) {
    Pairs.push_back(Value->getId());
    Pairs.push_back(BB->getId());
    WordCount = Pairs.size() + FixedWordCount;
    validate();
  }
  void setPairs(const std::vector<SPRVValue *> &ThePairs) {
    Pairs = getIds(ThePairs);
    WordCount = Pairs.size() + FixedWordCount;
    validate();
  }
  void foreachPair(std::function<void(SPRVValue *, SPRVBasicBlock *,
      size_t)> Func) {
    for (size_t I = 0, E = Pairs.size()/2; I != E; ++I) {
      SPRVEntry *Value, *BB;
      if (!Module->exist(Pairs[2*I], &Value) ||
          !Module->exist(Pairs[2*I+1], &BB))
        continue;
      Func(static_cast<SPRVValue *>(Value), static_cast<SPRVBasicBlock *>(BB),
          I);
    }
  }
  void foreachPair(std::function<void(SPRVValue *, SPRVBasicBlock *)> Func)
    const {
    for (size_t I = 0, E = Pairs.size()/2; I != E; ++I) {
      SPRVEntry *Value, *BB;
      if (!Module->exist(Pairs[2*I], &Value) ||
          !Module->exist(Pairs[2*I+1], &BB))
        continue;
      Func(static_cast<SPRVValue *>(Value), static_cast<SPRVBasicBlock *>(BB));
    }
  }
  void setWordCount(SPRVWord TheWordCount) {
    SPRVEntry::setWordCount(TheWordCount);
    Pairs.resize(TheWordCount - FixedWordCount);
  }
  _SPRV_DEF_ENCDEC3(Type, Id, Pairs)
  void validate()const {
    assert(WordCount == Pairs.size() + FixedWordCount);
    assert(OpCode == OC);
    assert(Pairs.size() % 2 == 0);
    foreachPair([=](SPRVValue *IncomingV, SPRVBasicBlock *IncomingBB){
      assert(IncomingV->isForward() || IncomingV->getType() == Type);
      assert(IncomingBB->isBasicBlock() || IncomingBB->isForward());
    });
    SPRVInstruction::validate();
  }
protected:
  std::vector<SPRVId> Pairs;
};

class SPRVCompare:public SPRVInstruction {
public:
  // Complete constructor
  SPRVCompare(SPRVOpCode TheOpCode, SPRVType *TheBoolType, SPRVId TheId,
      SPRVId TheOp1, SPRVId TheOp2, SPRVBasicBlock *TheBB)
    :SPRVInstruction(5, TheOpCode, TheBoolType, TheId, TheBB), Op1(TheOp1),
     Op2(TheOp2){
    validate();
    assert(TheBB && "Invalid BB");
  }
  // Incomplete constructor
  SPRVCompare(SPRVOpCode TheOpCode):SPRVInstruction(TheOpCode),
      Op1(SPRVID_INVALID), Op2(SPRVID_INVALID){}

  SPRVValue *getOperand(unsigned i) {
    assert(i <= 1);
    return getValue(i ? Op2 : Op1);
  }
  virtual std::vector<SPRVValue *> getOperands() {
    std::vector<SPRVValue *> Operands;
    Operands.push_back(getValue(Op1));
    Operands.push_back(getValue(Op2));
    return Operands;
  }
protected:
  _SPRV_DEF_ENCDEC4(Type, Id, Op1, Op2)
  void validate()const {
    SPRVType *op1Ty, *op2Ty, *resTy;
    SPRVInstruction::validate();
    if (getValue(Op1)->isForward() || getValue(Op2)->isForward())
      return;

    if (getValueType(Op1)->isTypeVector()) {
      op1Ty = getValueType(Op1)->getVectorComponentType();
      op2Ty = getValueType(Op2)->getVectorComponentType();
      resTy = Type->getVectorComponentType();
      assert(getValueType(Op1)->getVectorComponentCount() ==
             getValueType(Op2)->getVectorComponentCount() &&
               "Inconsistent Vector component width");
    }
    else {
      op1Ty = getValueType(Op1);
      op2Ty = getValueType(Op2);
      resTy = Type;
    }
    assert(isCmpOpCode(OpCode) && "Invalid op code for cmp inst");
    assert((resTy->isTypeBool() || resTy->isTypeInt()) &&
        "Invalid type for compare instruction");
    assert(op1Ty == op2Ty && "Inconsistent types");
  }
  SPRVId Op1;
  SPRVId Op2;
};

template<SPRVOpCode TheOpCode>
class SPRVCmpInst:public SPRVCompare {
public:
  // Complete constructor
  SPRVCmpInst(SPRVType *TheBoolType, SPRVId TheId, SPRVId TheOp1, SPRVId TheOp2,
      SPRVBasicBlock *TheBB)
    :SPRVCompare(TheOpCode, TheBoolType, TheId, TheOp1, TheOp2, TheBB){}
  // Incomplete constructor
  SPRVCmpInst():SPRVCompare(TheOpCode){}
};

#define _SPRV_OP(x) typedef SPRVCmpInst<SPRVOC_Op##x> SPRV##x;
_SPRV_OP(IEqual)
_SPRV_OP(FOrdEqual)
_SPRV_OP(FUnordEqual)
_SPRV_OP(INotEqual)
_SPRV_OP(FOrdNotEqual)
_SPRV_OP(FUnordNotEqual)
_SPRV_OP(ULessThan)
_SPRV_OP(SLessThan)
_SPRV_OP(FOrdLessThan)
_SPRV_OP(FUnordLessThan)
_SPRV_OP(UGreaterThan)
_SPRV_OP(SGreaterThan)
_SPRV_OP(FOrdGreaterThan)
_SPRV_OP(FUnordGreaterThan)
_SPRV_OP(ULessThanEqual)
_SPRV_OP(SLessThanEqual)
_SPRV_OP(FOrdLessThanEqual)
_SPRV_OP(FUnordLessThanEqual)
_SPRV_OP(UGreaterThanEqual)
_SPRV_OP(SGreaterThanEqual)
_SPRV_OP(FOrdGreaterThanEqual)
_SPRV_OP(FUnordGreaterThanEqual)
_SPRV_OP(LessOrGreater)
_SPRV_OP(Ordered)
_SPRV_OP(Unordered)
#undef _SPRV_OP

class SPRVSelect:public SPRVInstruction {
public:
  // Complete constructor
  SPRVSelect(SPRVId TheId, SPRVId TheCondition, SPRVId TheOp1, SPRVId TheOp2,
      SPRVBasicBlock *TheBB)
    :SPRVInstruction(6, SPRVOC_OpSelect, TheBB->getValueType(TheOp1), TheId,
        TheBB), Condition(TheCondition), Op1(TheOp1), Op2(TheOp2){
    validate();
    assert(TheBB && "Invalid BB");
  }
  // Incomplete constructor
  SPRVSelect():SPRVInstruction(SPRVOC_OpSelect), Condition(SPRVID_INVALID),
      Op1(SPRVID_INVALID), Op2(SPRVID_INVALID){}
  SPRVValue *getCondition() { return getValue(Condition);}
  SPRVValue *getTrueValue() { return getValue(Op1);}
  SPRVValue *getFalseValue() { return getValue(Op2);}
protected:
  _SPRV_DEF_ENCDEC5(Type, Id, Condition, Op1, Op2)
  void validate()const {
    SPRVInstruction::validate();
    if (getValue(Condition)->isForward() ||
        getValue(Op1)->isForward() ||
        getValue(Op2)->isForward())
      return;

    SPRVType *conTy = getValueType(Condition)->isTypeVector() ?
        getValueType(Condition)->getVectorComponentType() :
        getValueType(Condition);
    assert(conTy->isTypeBool() && "Invalid type");
    assert(getType() == getValueType(Op1) && getType() == getValueType(Op2) &&
        "Inconsistent type");
  }
  SPRVId Condition;
  SPRVId Op1;
  SPRVId Op2;
};

class SPRVSwitch: public SPRVInstruction {
public:
  static const SPRVOpCode OC = SPRVOC_OpSwitch;
  static const SPRVWord FixedWordCount = 3;
  SPRVSwitch(SPRVValue *TheSelect, SPRVBasicBlock *TheDefault,
      const std::vector<std::pair<SPRVWord, SPRVBasicBlock *>> &ThePairs,
      SPRVBasicBlock *BB)
    :SPRVInstruction(ThePairs.size() * 2 + FixedWordCount, OC, BB),
     Select(TheSelect->getId()), Default(TheDefault->getId()) {
    for (auto &I:ThePairs) {
      Pairs.push_back(I.first);
      Pairs.push_back(I.second->getId());
    }
    validate();
    assert(BB && "Invalid BB");
  }
  SPRVSwitch():SPRVInstruction(OC), Select(SPRVWORD_MAX),
      Default(SPRVWORD_MAX) {
    setHasNoId();
    setHasNoType();
  }
  std::vector<SPRVValue *> getPairs() {
    return getValues(Pairs);
  }
  SPRVValue *getSelect() const { return getValue(Select);}
  SPRVBasicBlock *getDefault() const {
    return static_cast<SPRVBasicBlock *>(getValue(Default));
  }
  size_t getNumPairs() const { return Pairs.size()/2;}
  void foreachPair(std::function<void(SPRVWord, SPRVBasicBlock *, size_t)> Func)
    const {
    for (size_t I = 0, E = Pairs.size()/2; I != E; ++I) {
      SPRVEntry *BB;
      if (!Module->exist(Pairs[2*I+1], &BB))
        continue;
      Func(Pairs[2*I], static_cast<SPRVBasicBlock *>(BB), I);
    }
  }
  void setWordCount(SPRVWord TheWordCount) {
    SPRVEntry::setWordCount(TheWordCount);
    Pairs.resize(TheWordCount - FixedWordCount);
  }
  _SPRV_DEF_ENCDEC3(Select, Default, Pairs)
  void validate()const {
    assert(WordCount == Pairs.size() + FixedWordCount);
    assert(OpCode == OC);
    assert(Pairs.size() % 2 == 0);
    foreachPair([=](SPRVWord Literal, SPRVBasicBlock *BB, size_t Index){
      assert(BB->isBasicBlock() || BB->isForward());
    });
    SPRVInstruction::validate();
  }
protected:
  SPRVId Select;
  SPRVId Default;
  std::vector<SPRVWord> Pairs;
};

class SPRVUnary:public SPRVInstruction {
public:
  // Complete constructor
  SPRVUnary(SPRVOpCode TheOpCode, SPRVType *TheType, SPRVId TheId,
      SPRVId TheOp, SPRVBasicBlock *TheBB, SPRVModule *TheModule)
    :SPRVInstruction(4, TheOpCode, TheType, TheId, TheBB, TheModule), Op(TheOp){
    validate();
    assert(TheModule && "Invalid module");
  }
  // Incomplete constructor
  SPRVUnary(SPRVOpCode TheOpCode):SPRVInstruction(TheOpCode),
      Op(SPRVID_INVALID) {}

  SPRVValue *getOperand() {
    return getValue(Op);
  }
  std::vector<SPRVValue *> getOperands() {
    std::vector<SPRVValue *> Res;
    Res.push_back(getOperand());
    return Res;
  }
protected:
  _SPRV_DEF_ENCDEC3(Type, Id, Op)
  void validate()const {
    SPRVInstruction::validate();
    if (getValue(Op)->isForward())
      return;
    if (isGenericNegateOpCode(OpCode)) {
      SPRVType *resTy = Type->isTypeVector() ?
        Type->getVectorComponentType() : Type;
      SPRVType *opTy = Type->isTypeVector() ?
        getValueType(Op)->getVectorComponentType() : getValueType(Op);

      assert(getType() == getValueType(Op)  &&
        "Inconsistent type");
      assert((resTy->isTypeInt() || resTy->isTypeFloat()) &&
        "Invalid type for Generic Negate instruction");
      assert((resTy->getBitWidth() == opTy->getBitWidth()) &&
        "Invalid bitwidth for Generic Negate instruction");
      assert((Type->isTypeVector() ? (Type->getVectorComponentCount() ==
          getValueType(Op)->getVectorComponentCount()): 1) &&
          "Invalid vector component Width for Generic Negate instruction");
    }
  }
  SPRVId Op;
};

template<SPRVOpCode TheOpCode>
class SPRVUnaryInst:public SPRVUnary {
public:
  // Complete constructor
  SPRVUnaryInst(SPRVType *TheType, SPRVId TheId, SPRVId TheOp,
      SPRVBasicBlock *TheBB, SPRVModule *TheModule)
    :SPRVUnary(TheOpCode, TheType, TheId, TheOp, TheBB, TheModule){}
  // Incomplete constructor
  SPRVUnaryInst():SPRVUnary(TheOpCode){}
};

#define _SPRV_OP(x) typedef SPRVUnaryInst<SPRVOC_Op##x> SPRV##x;
_SPRV_OP(ConvertFToU)
_SPRV_OP(ConvertFToS)
_SPRV_OP(ConvertSToF)
_SPRV_OP(ConvertUToF)
_SPRV_OP(UConvert)
_SPRV_OP(SConvert)
_SPRV_OP(FConvert)
_SPRV_OP(SatConvertSToU)
_SPRV_OP(SatConvertUToS)
_SPRV_OP(ConvertPtrToU)
_SPRV_OP(ConvertUToPtr)
_SPRV_OP(PtrCastToGeneric)
_SPRV_OP(GenericCastToPtr)
_SPRV_OP(Bitcast)
_SPRV_OP(SNegate)
_SPRV_OP(FNegate)
_SPRV_OP(Not)
_SPRV_OP(IsNan)
_SPRV_OP(IsInf)
_SPRV_OP(IsFinite)
_SPRV_OP(IsNormal)
_SPRV_OP(SignBitSet)
_SPRV_OP(Any)
_SPRV_OP(All)
#undef _SPRV_OP

template<SPRVOpCode TheOpCode>
class SPRVAccessChainGeneric:public SPRVInstruction {
public:
  // Complete constructor
  SPRVAccessChainGeneric(SPRVType *TheType, SPRVId TheId, SPRVValue *TheBase,
      const std::vector<SPRVValue *>& TheIndices, SPRVBasicBlock *TheBB,
      SPRVModule *TheModule):
        SPRVInstruction(TheIndices.size() + 4, TheOpCode, TheType, TheId, TheBB,
            TheModule), Base(TheBase->getId()) {
    Indices = getIds(TheIndices);
    validate();
    assert(TheModule && "Invalid Module");
  }
  // Incomplete constructor
  SPRVAccessChainGeneric():SPRVInstruction(TheOpCode), Base(SPRVID_INVALID){}

  SPRVValue *getBase() { return getValue(Base);}
  std::vector<SPRVValue *> getIndices()const {
    return getValues(Indices);
  }

protected:
  void setWordCount(SPRVWord TheWordCount) {
    SPRVEntry::setWordCount(TheWordCount);
    Indices.resize(TheWordCount - 4);
  }
  _SPRV_DEF_ENCDEC4(Type, Id, Base, Indices)
  // ToDo: validate the result type is consistent with the base type and indices
  // need to trace through the base type for struct types
  void validate()const {
    SPRVInstruction::validate();
  }
  SPRVId Base;
  std::vector<SPRVId> Indices;
};

typedef SPRVAccessChainGeneric<SPRVOC_OpAccessChain> SPRVAccessChain;
typedef SPRVAccessChainGeneric<SPRVOC_OpInBoundsAccessChain>
  SPRVInBoundsAccessChain;

template<SPRVOpCode OC, SPRVWord FixedWordCount>
class SPRVFunctionCallGeneric: public SPRVInstruction {
public:
  SPRVFunctionCallGeneric(SPRVType *TheType, SPRVId TheId,
      const std::vector<SPRVWord> &TheArgs, SPRVBasicBlock *BB)
    :SPRVInstruction(TheArgs.size() + FixedWordCount, OC, TheType, TheId, BB),
     Args(TheArgs){
    validate();
    assert(BB && "Invalid BB");
  }
  SPRVFunctionCallGeneric(SPRVType *TheType, SPRVId TheId,
      const std::vector<SPRVValue *> &TheArgs, SPRVBasicBlock *BB)
    :SPRVInstruction(TheArgs.size() + FixedWordCount, OC, TheType, TheId, BB) {
    Args = getIds(TheArgs);
    validate();
    assert(BB && "Invalid BB");
  }
  SPRVFunctionCallGeneric():SPRVInstruction(OC) {}
  const std::vector<SPRVWord> &getArguments() {
    return Args;
  }
  std::vector<SPRVValue *> getArgumentValues() {
    return getValues(Args);
  }
  std::vector<SPRVType *> getArgumentValueTypes()const {
    std::vector<SPRVType *> ArgTypes;
    for (auto &I:Args)
      ArgTypes.push_back(getValue(I)->getType());
    return ArgTypes;
  }
  void setWordCount(SPRVWord TheWordCount) {
    SPRVEntry::setWordCount(TheWordCount);
    Args.resize(TheWordCount - FixedWordCount);
  }
  void validate()const {
    SPRVInstruction::validate();
  }
protected:
  std::vector<SPRVWord> Args;
};

class SPRVFunctionCall:
    public SPRVFunctionCallGeneric<SPRVOC_OpFunctionCall, 4> {
public:
  SPRVFunctionCall(SPRVId TheId, SPRVFunction *TheFunction,
      const std::vector<SPRVValue *> &TheArgs, SPRVBasicBlock *BB);
  SPRVFunctionCall():FunctionId(SPRVID_INVALID) {}
  SPRVFunction *getFunction()const {
    return get<SPRVFunction>(FunctionId);
  }
  _SPRV_DEF_ENCDEC4(Type, Id, FunctionId, Args)
  void validate()const;
protected:
  SPRVId FunctionId;
};

class SPRVExtInst: public SPRVFunctionCallGeneric<SPRVOC_OpExtInst, 5> {
public:
  SPRVExtInst(SPRVType *TheType, SPRVId TheId,
      SPRVId TheBuiltinSet, SPRVWord TheEntryPoint,
      const std::vector<SPRVWord> &TheArgs, SPRVBasicBlock *BB)
    :SPRVFunctionCallGeneric(TheType, TheId, TheArgs, BB),
     BuiltinSet(TheBuiltinSet),
     EntryPoint(TheEntryPoint) {
    validate();
  }
  SPRVExtInst(SPRVType *TheType, SPRVId TheId,
      SPRVId TheBuiltinSet, SPRVWord TheEntryPoint,
      const std::vector<SPRVValue *> &TheArgs, SPRVBasicBlock *BB)
    :SPRVFunctionCallGeneric(TheType, TheId, TheArgs, BB),
     BuiltinSet(TheBuiltinSet),
     EntryPoint(TheEntryPoint) {
    validate();
  }
  SPRVExtInst(): BuiltinSet(SPRVWORD_MAX),
      EntryPoint(SPRVWORD_MAX) {}
  SPRVId getBuiltinSet()const {
    return BuiltinSet;
  }
  SPRVWord getEntryPoint()const {
    return EntryPoint;
  }

  void encode(std::ostream &O) const {
    getEncoder(O) << Type << Id << BuiltinSet;
    switch(Module->getBuiltinSet(BuiltinSet)) {
    case SPRVBIS_OpenCL12:
      getEncoder(O) << EntryPointOCL12;
      break;
    case SPRVBIS_OpenCL20:
      getEncoder(O) << EntryPointOCL20;
      break;
    case SPRVBIS_OpenCL21:
      getEncoder(O) << EntryPointOCL21;
      break;
    default:
      assert(0 && "not supported");
      getEncoder(O) << EntryPoint;
    }
    getEncoder(O) << Args;
  }
  void decode(std::istream &I) {
    getDecoder(I) >> Type >> Id >> BuiltinSet;
    switch(Module->getBuiltinSet(BuiltinSet)) {
    case SPRVBIS_OpenCL12:
      getDecoder(I) >> EntryPointOCL12;
      break;
    case SPRVBIS_OpenCL20:
      getDecoder(I) >> EntryPointOCL20;
      break;
    case SPRVBIS_OpenCL21:
      getDecoder(I) >> EntryPointOCL21;
      break;
    default:
      assert(0 && "not supported");
      getDecoder(I) >> EntryPoint;
    }
    getDecoder(I) >> Args;
  }
  void validate()const {
    SPRVFunctionCallGeneric::validate();
    validateBuiltin(BuiltinSet, EntryPoint);
  }
protected:
  SPRVId BuiltinSet;
  union {
    SPRVWord EntryPoint;
    SPRVBuiltinOCL12Kind EntryPointOCL12;
    SPRVBuiltinOCL20Kind EntryPointOCL20;
    SPRVBuiltinOCL21Kind EntryPointOCL21;
  };
};

class SPRVCompositeExtract:public SPRVInstruction {
public:
  const static SPRVOpCode OC = SPRVOC_OpCompositeExtract;
  // Complete constructor
  SPRVCompositeExtract(SPRVType *TheType, SPRVId TheId, SPRVValue *TheComposite,
      const std::vector<SPRVWord>& TheIndices, SPRVBasicBlock *TheBB):
        SPRVInstruction(TheIndices.size() + 4, OC, TheType, TheId, TheBB),
        Composite(TheComposite->getId()), Indices(TheIndices){
    validate();
    assert(TheBB && "Invalid BB");
  }
  // Incomplete constructor
  SPRVCompositeExtract():SPRVInstruction(OC), Composite(SPRVID_INVALID){}

  SPRVValue *getComposite() { return getValue(Composite);}
  const std::vector<SPRVWord>& getIndices()const { return Indices;}
protected:
  void setWordCount(SPRVWord TheWordCount) {
    SPRVEntry::setWordCount(TheWordCount);
    Indices.resize(TheWordCount - 4);
  }
  _SPRV_DEF_ENCDEC4(Type, Id, Composite, Indices)
  // ToDo: validate the result type is consistent with the base type and indices
  // need to trace through the base type for struct types
  void validate()const {
    SPRVInstruction::validate();
    assert(getValueType(Composite)->isTypeArray() ||
        getValueType(Composite)->isTypeStruct() ||
        getValueType(Composite)->isTypeVector());
  }
  SPRVId Composite;
  std::vector<SPRVWord> Indices;
};

class SPRVCompositeInsert:public SPRVInstruction {
public:
  const static SPRVOpCode OC = SPRVOC_OpCompositeInsert;
  const static SPRVWord FixedWordCount = 5;
  // Complete constructor
  SPRVCompositeInsert(SPRVId TheId, SPRVValue *TheObject,
      SPRVValue *TheComposite, const std::vector<SPRVWord>& TheIndices,
      SPRVBasicBlock *TheBB):
        SPRVInstruction(TheIndices.size() + FixedWordCount, OC,
            TheComposite->getType(), TheId, TheBB),
        Object(TheObject->getId()), Composite(TheComposite->getId()),
        Indices(TheIndices){
    validate();
    assert(TheBB && "Invalid BB");
  }
  // Incomplete constructor
  SPRVCompositeInsert():SPRVInstruction(OC), Object(SPRVID_INVALID),
      Composite(SPRVID_INVALID){}

  SPRVValue *getObject() { return getValue(Object);}
  SPRVValue *getComposite() { return getValue(Composite);}
  const std::vector<SPRVWord>& getIndices()const { return Indices;}
protected:
  void setWordCount(SPRVWord TheWordCount) {
    SPRVEntry::setWordCount(TheWordCount);
    Indices.resize(TheWordCount - FixedWordCount);
  }
  _SPRV_DEF_ENCDEC5(Type, Id, Object, Composite, Indices)
  // ToDo: validate the object type is consistent with the base type and indices
  // need to trace through the base type for struct types
  void validate()const {
    SPRVInstruction::validate();
    assert(OpCode == OC);
    assert(WordCount == Indices.size() + FixedWordCount);
    assert(getValueType(Composite)->isTypeArray() ||
        getValueType(Composite)->isTypeStruct() ||
        getValueType(Composite)->isTypeVector());
    assert(Type == getValueType(Composite));
  }
  SPRVId Object;
  SPRVId Composite;
  std::vector<SPRVWord> Indices;
};

class SPRVCopyObject :public SPRVInstruction {
public:
  const static SPRVOpCode OC = SPRVOC_OpCopyObject;

  // Complete constructor
  SPRVCopyObject(SPRVType *TheType, SPRVId TheId, SPRVValue *TheOperand,
    SPRVBasicBlock *TheBB) :
    SPRVInstruction(4, OC, TheType, TheId, TheBB),
    Operand(TheOperand->getId()) {
    validate();
    assert(TheBB && "Invalid BB");
  }
  // Incomplete constructor
  SPRVCopyObject() :SPRVInstruction(OC), Operand(SPRVID_INVALID) {}

  SPRVValue *getOperand() { return getValue(Operand); }

protected:
  _SPRV_DEF_ENCDEC3(Type, Id, Operand)

    void validate()const {
    SPRVInstruction::validate();
  }
  SPRVId Operand;
};


class SPRVCopyMemory :public SPRVInstruction, public SPRVMemoryAccess {
public:
  const static SPRVOpCode OC = SPRVOC_OpCopyMemory;
  const static SPRVWord FixedWords = 3;
  // Complete constructor
  SPRVCopyMemory(SPRVValue *TheTarget, SPRVValue *TheSource,
      const std::vector<SPRVWord> &TheMemoryAccess,
    SPRVBasicBlock *TheBB) :
    SPRVInstruction(FixedWords + TheMemoryAccess.size(), OC, TheBB),
    SPRVMemoryAccess(TheMemoryAccess),
    MemoryAccess(TheMemoryAccess),
    Target(TheTarget->getId()),
    Source(TheSource->getId()) {
    validate();
    assert(TheBB && "Invalid BB");
  }

  // Incomplete constructor
  SPRVCopyMemory() :SPRVInstruction(OC), SPRVMemoryAccess(),
      Target(SPRVID_INVALID),
    Source(SPRVID_INVALID) {
    setHasNoId();
    setHasNoType();
  }

  SPRVValue *getSource() { return getValue(Source); }
  SPRVValue *getTarget() { return getValue(Target); }

protected:
  void setWordCount(SPRVWord TheWordCount) {
    SPRVEntry::setWordCount(TheWordCount);
    MemoryAccess.resize(TheWordCount - FixedWords);
  }

  void encode(std::ostream &O) const {
    getEncoder(O) << Target << Source << MemoryAccess;
  }

  void decode(std::istream &I) {
    getDecoder(I) >> Target >> Source >> MemoryAccess;
    MemoryAccessUpdate(MemoryAccess);
  }

  void validate()const {
    assert((getValueType(Id) == getValueType(Source)) && "Inconsistent type");
    assert(getValueType(Id)->isTypePointer() && "Invalid type");
    assert(!(getValueType(Id)->getPointerElementType()->isTypeVoid()) &&
        "Invalid type");
    SPRVInstruction::validate();
  }

  std::vector<SPRVWord> MemoryAccess;
  SPRVId Target;
  SPRVId Source;
};

class SPRVCopyMemorySized :public SPRVInstruction, public SPRVMemoryAccess {
public:
  const static SPRVOpCode OC = SPRVOC_OpCopyMemorySized;
  const static SPRVWord FixedWords = 4;
  // Complete constructor
  SPRVCopyMemorySized(SPRVValue *TheTarget, SPRVValue *TheSource,
      SPRVValue *TheSize, const std::vector<SPRVWord> &TheMemoryAccess,
      SPRVBasicBlock *TheBB) :
      SPRVInstruction(FixedWords + TheMemoryAccess.size(), OC, TheBB),
      SPRVMemoryAccess(TheMemoryAccess),
      MemoryAccess(TheMemoryAccess),
      Target(TheTarget->getId()),
      Source(TheSource->getId()),
      Size(TheSize->getId()) {
    validate();
    assert(TheBB && "Invalid BB");
  }
  // Incomplete constructor
  SPRVCopyMemorySized() :SPRVInstruction(OC), SPRVMemoryAccess(),
      Target(SPRVID_INVALID), Source(SPRVID_INVALID), Size(0) {
    setHasNoId();
    setHasNoType();
  }

  SPRVValue *getSource() { return getValue(Source); }
  SPRVValue *getTarget() { return getValue(Target); }
  SPRVValue *getSize() { return getValue(Size); }

protected:
  void setWordCount(SPRVWord TheWordCount) {
    SPRVEntry::setWordCount(TheWordCount);
    MemoryAccess.resize(TheWordCount - FixedWords);
  }

  void encode(std::ostream &O) const {
    getEncoder(O) << Target << Source << Size << MemoryAccess;
  }

  void decode(std::istream &I) {
    getDecoder(I) >> Target >> Source >> Size >> MemoryAccess;
    MemoryAccessUpdate(MemoryAccess);
  }

    void validate()const {
    SPRVInstruction::validate();
  }

  std::vector<SPRVWord> MemoryAccess;
  SPRVId Target;
  SPRVId Source;
  SPRVId Size;
};

class SPRVVectorExtractDynamic:public SPRVInstruction {
public:
  const static SPRVOpCode OC = SPRVOC_OpVectorExtractDynamic;
  // Complete constructor
  SPRVVectorExtractDynamic(SPRVId TheId, SPRVValue *TheVector,
      SPRVValue* TheIndex, SPRVBasicBlock *TheBB)
  :SPRVInstruction(5, OC, TheVector->getType()->getVectorComponentType(),
      TheId, TheBB), VectorId(TheVector->getId()),
      IndexId(TheIndex->getId()){
    validate();
    assert(TheBB && "Invalid BB");
  }
  // Incomplete constructor
  SPRVVectorExtractDynamic():SPRVInstruction(OC), VectorId(SPRVID_INVALID),
      IndexId(SPRVID_INVALID){}

  SPRVValue *getVector() { return getValue(VectorId);}
  SPRVValue *getIndex()const { return getValue(IndexId);}
protected:
  _SPRV_DEF_ENCDEC4(Type, Id, VectorId, IndexId)
  void validate()const {
    SPRVInstruction::validate();
    if (getValue(VectorId)->isForward())
      return;
    assert(getValueType(VectorId)->isTypeVector());
  }
  SPRVId VectorId;
  SPRVId IndexId;
};

class SPRVVectorInsertDynamic :public SPRVInstruction {
public:
  const static SPRVOpCode OC = SPRVOC_OpVectorInsertDynamic;
  // Complete constructor
  SPRVVectorInsertDynamic(SPRVId TheId, SPRVValue *TheVector,
      SPRVValue* TheComponent, SPRVValue* TheIndex, SPRVBasicBlock *TheBB)
    :SPRVInstruction(6, OC, TheVector->getType()->getVectorComponentType(),
    TheId, TheBB), VectorId(TheVector->getId()),
    IndexId(TheIndex->getId()), ComponentId(TheComponent->getId()){
    validate();
    assert(TheBB && "Invalid BB");
  }
  // Incomplete constructor
  SPRVVectorInsertDynamic() :SPRVInstruction(OC), VectorId(SPRVID_INVALID),
    IndexId(SPRVID_INVALID), ComponentId(SPRVID_INVALID){}

  SPRVValue *getVector() { return getValue(VectorId); }
  SPRVValue *getIndex()const { return getValue(IndexId); }
  SPRVValue *getComponent() { return getValue(ComponentId); }
protected:
  _SPRV_DEF_ENCDEC5(Type, Id, VectorId, ComponentId, IndexId)
    void validate()const {
    SPRVInstruction::validate();
    if (getValue(VectorId)->isForward())
      return;
    assert(getValueType(VectorId)->isTypeVector());
  }
  SPRVId VectorId;
  SPRVId IndexId;
  SPRVId ComponentId;
};

class SPRVVectorShuffle:public SPRVInstruction {
public:
  const static SPRVOpCode OC = SPRVOC_OpVectorShuffle;
  const static SPRVWord FixedWordCount = 5;
  // Complete constructor
  SPRVVectorShuffle(SPRVId TheId, SPRVType *TheType, SPRVValue *TheVector1,
      SPRVValue *TheVector2, const std::vector<SPRVWord>& TheComponents,
      SPRVBasicBlock *TheBB):
        SPRVInstruction(TheComponents.size() + FixedWordCount, OC, TheType,
            TheId, TheBB),
        Vector1(TheVector1->getId()), Vector2(TheVector2->getId()),
        Components(TheComponents){
    validate();
    assert(TheBB && "Invalid BB");
  }
  // Incomplete constructor
  SPRVVectorShuffle():SPRVInstruction(OC), Vector1(SPRVID_INVALID),
      Vector2(SPRVID_INVALID){}

  SPRVValue *getVector1() { return getValue(Vector1);}
  SPRVValue *getVector2() { return getValue(Vector2);}
  const std::vector<SPRVWord>& getComponents()const { return Components;}
protected:
  void setWordCount(SPRVWord TheWordCount) {
    SPRVEntry::setWordCount(TheWordCount);
    Components.resize(TheWordCount - FixedWordCount);
  }
  _SPRV_DEF_ENCDEC5(Type, Id, Vector1, Vector2, Components)
  void validate()const {
    SPRVInstruction::validate();
    assert(OpCode == OC);
    assert(WordCount == Components.size() + FixedWordCount);
    assert(Type->isTypeVector());
    assert(Type->getVectorComponentType() ==
        getValueType(Vector1)->getVectorComponentType());
    if (getValue(Vector1)->isForward() ||
        getValue(Vector2)->isForward())
      return;
    assert(getValueType(Vector1) == getValueType(Vector2));
    size_t CompCount = Type->getVectorComponentCount();
    assert(Components.size() == CompCount);
    assert(Components.size() > 0 && Components.size() <=
        getValueType(Vector1)->getVectorComponentCount() * 2);
  }
  SPRVId Vector1;
  SPRVId Vector2;
  std::vector<SPRVWord> Components;
};

class SPRVControlBarrier:public SPRVInstruction {
public:
  static const SPRVOpCode OC = SPRVOC_OpControlBarrier;
  // Complete constructor
  SPRVControlBarrier(SPRVExecutionScopeKind TheScope,
      SPRVMemoryScopeKind TheMemScope, SPRVWord TheMemSema,
      SPRVBasicBlock *TheBB)
    :SPRVInstruction(4, OC, TheBB),ExecScope(TheScope),
     MemScope(TheMemScope), MemSema(TheMemSema){
    validate();
    assert(TheBB && "Invalid BB");
  }
  // Incomplete constructor
  SPRVControlBarrier():SPRVInstruction(OC), ExecScope(SPRVES_Count) {
    setHasNoId();
    setHasNoType();
  }
  void setWordCount(SPRVWord TheWordCount) {
    SPRVEntry::setWordCount(TheWordCount);
  }
  SPRVExecutionScopeKind getExecScope() const {
    return ExecScope;
  }
  SPRVMemoryScopeKind getMemScope() const {
    return MemScope;
  }
  bool hasMemSemantic() const {
    return MemSema != 0;
  }
  SPRVWord getMemSemantic() const {
    return MemSema;
  }
protected:
  _SPRV_DEF_ENCDEC3(ExecScope, MemScope, MemSema)
  void validate()const {
    assert(OpCode == OC);
    assert(WordCount == 4);
    SPRVInstruction::validate();
    isValid(ExecScope);
    isValid(MemScope);
  }
  SPRVExecutionScopeKind ExecScope;
  SPRVMemoryScopeKind MemScope;
  SPRVWord MemSema;
};

class SPRVMemoryBarrier:public SPRVInstruction {
public:
  static const SPRVOpCode OC = SPRVOC_OpMemoryBarrier;
  // Complete constructor
  SPRVMemoryBarrier(SPRVExecutionScopeKind TheScope,
      SPRVWord TheMemSemantic, SPRVBasicBlock *TheBB)
    :SPRVInstruction(3, OC, TheBB), ExecScope(TheScope),
     MemSemantic(TheMemSemantic){
    validate();
    assert(TheBB && "Invalid BB");
  }
  // Incomplete constructor
  SPRVMemoryBarrier():SPRVInstruction(OC), ExecScope(SPRVES_Count),
      MemSemantic(SPRVWORD_MAX){
    setHasNoId();
    setHasNoType();
  }
  SPRVExecutionScopeKind getExecScope() const {
    return ExecScope;
  }
  SPRVWord getMemSemantic() const {
    return MemSemantic;
  }
protected:
  _SPRV_DEF_ENCDEC2(ExecScope, MemSemantic)
  void validate()const {
    assert(OpCode == OC);
    assert(WordCount == 3);
    SPRVInstruction::validate();
    isValid(ExecScope);
    isValid(MemSemantic);
  }
  SPRVExecutionScopeKind ExecScope;
  SPRVWord MemSemantic;
};


class SPRVAtomicOperatorGeneric: public SPRVInstruction,
  public SPRVComponentOperands, public SPRVComponentExecutionScope,
  public SPRVComponentMemorySemanticsMask {
public:
  const static SPRVWord FixedWords = 5;
  SPRVAtomicOperatorGeneric(SPRVOpCode TheOC, SPRVType *TheType,
      SPRVId TheId, SPRVExecutionScopeKind TheScope, SPRVWord TheMemSema,
      const std::vector<SPRVValue *> &TheOperands, SPRVBasicBlock *BB):
        SPRVInstruction(TheOperands.size() + FixedWords, TheOC, TheType,
            TheId, BB), SPRVComponentOperands(TheOperands),
        SPRVComponentExecutionScope(TheScope),
        SPRVComponentMemorySemanticsMask(TheMemSema) {
    validate();
    assert(BB && "Invalid BB");
  }
  SPRVAtomicOperatorGeneric(SPRVOpCode TheOC):SPRVInstruction(TheOC){}
  _SPRV_DEF_ENCDEC6(Type, Id, Operands[0], ExecScope, MemSema,
      std::make_pair(Operands.begin() + 1, Operands.end()))
  void setWordCount(SPRVWord TheWordCount) {
    SPRVEntry::setWordCount(TheWordCount);
    Operands.resize(WordCount - FixedWords);
  }
  void validate() {
    SPRVInstruction::validate();
    assert(isValid(ExecScope));
    assert(isValidSPRVMemSemanticsMask(MemSema));
    assert(Operands.size() + FixedWords == WordCount);
    assert(Operands.size() >= 1);
    if (Operands[0]->isForward())
      return;
    assert(Operands[0]->getType()->isTypePointer());
    assert(Operands[0]->getType()->getPointerElementType() == Type);
  }
  virtual std::vector<SPRVValue *> getOperands() {
    return getCompOperands();
  }
};

template<SPRVOpCode OC>
class SPRVAtomicOperator: public SPRVAtomicOperatorGeneric {
public:
  SPRVAtomicOperator(SPRVType *TheType,
    SPRVId TheId, SPRVExecutionScopeKind TheScope, SPRVWord TheMemSema,
    const std::vector<SPRVValue *> &TheOperands, SPRVBasicBlock *BB):
      SPRVAtomicOperatorGeneric(OC, TheType,
      TheId, TheScope, TheMemSema, TheOperands, BB) {}
  SPRVAtomicOperator():SPRVAtomicOperatorGeneric(OC){}
};

#define _SPRV_OP(x) typedef SPRVAtomicOperator<SPRVOC_Op##x> SPRV##x;
_SPRV_OP(AtomicInit)
_SPRV_OP(AtomicLoad)
_SPRV_OP(AtomicStore)
_SPRV_OP(AtomicExchange)
_SPRV_OP(AtomicCompareExchange)
_SPRV_OP(AtomicIIncrement)
_SPRV_OP(AtomicIDecrement)
_SPRV_OP(AtomicIAdd)
_SPRV_OP(AtomicISub)
_SPRV_OP(AtomicUMin)
_SPRV_OP(AtomicUMax)
_SPRV_OP(AtomicIMin)
_SPRV_OP(AtomicIMax)
_SPRV_OP(AtomicAnd)
_SPRV_OP(AtomicOr)
_SPRV_OP(AtomicXor)
#undef _SPRV_OP

class SPRVAsyncGroupCopy:public SPRVInstruction {
public:
  static const SPRVOpCode OC = SPRVOC_OpAsyncGroupCopy;
  static const SPRVWord WC = 9;
  // Complete constructor
  SPRVAsyncGroupCopy(SPRVExecutionScopeKind TheScope, SPRVId TheId,
      SPRVValue *TheDest, SPRVValue *TheSrc, SPRVValue *TheNumElems,
      SPRVValue *TheStride, SPRVValue *TheEvent, SPRVBasicBlock *TheBB)
    :SPRVInstruction(WC, OC, TheEvent->getType(), TheId, TheBB),
     ExecScope(TheScope), Destination(TheDest->getId()),
     Source(TheSrc->getId()), NumElements(TheNumElems->getId()),
     Stride(TheStride->getId()), Event(TheEvent->getId()){
    validate();
    assert(TheBB && "Invalid BB");
  }
  // Incomplete constructor
  SPRVAsyncGroupCopy():SPRVInstruction(OC), ExecScope(SPRVES_Count),
      Destination(SPRVID_INVALID), Source(SPRVID_INVALID),
      NumElements(SPRVID_INVALID), Stride(SPRVID_INVALID),
      Event(SPRVID_INVALID){
  }
  SPRVExecutionScopeKind getExecScope() const {
    return ExecScope;
  }
  SPRVValue *getDestination()const { return getValue(Destination);}
  SPRVValue *getSource()const { return getValue(Source);}
  SPRVValue *getNumElements()const { return getValue(NumElements);}
  SPRVValue *getStride()const { return getValue(Stride);}
  SPRVValue *getEvent()const { return getValue(Event);}
  std::vector<SPRVValue *> getOperands() {
    std::vector<SPRVId> Operands;
    Operands.push_back(Destination);
    Operands.push_back(Source);
    Operands.push_back(NumElements);
    Operands.push_back(Stride);
    Operands.push_back(Event);
    return getValues(Operands);
  }

protected:
  _SPRV_DEF_ENCDEC8(Type, Id, ExecScope, Destination, Source, NumElements,
      Stride, Event)
  void validate()const {
    assert(OpCode == OC);
    assert(WordCount == WC);
    SPRVInstruction::validate();
    isValid(ExecScope);
  }
  SPRVExecutionScopeKind ExecScope;
  SPRVId Destination;
  SPRVId Source;
  SPRVId NumElements;
  SPRVId Stride;
  SPRVId Event;
};

enum SPRVOpKind {
  SPRVOPK_Id,
  SPRVOPK_Literal,
  SPRVOPK_Count
};

template<SPRVOpCode OC = SPRVOC_OpNop,
    bool HasId = true,
    bool IsGroupInst = false>
class SPRVInstTemplate:public SPRVInstruction {
public:
  // Instruction without Id
  SPRVInstTemplate(SPRVOpCode TheOC, const std::vector<SPRVWord> &TheOps,
      SPRVBasicBlock *TheBB)
    :SPRVInstruction(TheOps.size() + 1, TheOC, TheBB),
     Ops(TheOps){
    init();
    validate();
    assert(TheBB && "Invalid BB");
  }
  // Instruction with Id
  SPRVInstTemplate(SPRVOpCode TheOC, SPRVType *TheType, SPRVId TheId,
      const std::vector<SPRVWord> &TheOps, SPRVBasicBlock *TheBB)
    :SPRVInstruction(TheOps.size() + 3, TheOC, TheType, TheId, TheBB),
     Ops(TheOps){
    init();
    validate();
    assert(TheBB && "Invalid BB");
  }
  SPRVInstTemplate():SPRVInstruction(OC){
    init();
  }
  void init(){
    if (!HasId) {
      setHasNoId();
      setHasNoType();
    }
    setGroupInst(IsGroupInst);
  }
  void setWordCount(SPRVWord TheWordCount) {
    SPRVEntry::setWordCount(TheWordCount);
    auto NumOps = WordCount - 1;
    if (hasId())
      --NumOps;
    if (hasType())
      --NumOps;
    Ops.resize(NumOps);
  }

  std::vector<SPRVWord> &getOpWords() {
    return Ops;
  }

  const std::vector<SPRVWord> &getOpWords() const {
    return Ops;
  }

  // Get operands which are values.
  // Drop execution scope literal.
  std::vector<SPRVValue *> getOperands() {
    std::vector<SPRVWord> VOps = Ops;
    if (GroupInst)
      VOps.erase(VOps.begin());
    return getValues(VOps);
  }

  bool isGroupInst() const {
    return GroupInst;
  }

  void setGroupInst(bool groupInst) {
    GroupInst = groupInst;
  }

protected:
  virtual void encode(std::ostream &O) const {
    auto E = getEncoder(O);
    if (hasType())
      E << Type;
    if (hasId())
      E << Id;
    E << Ops;
  }
  virtual void decode(std::istream &I) {
    auto D = getDecoder(I);
    if (hasType())
      D >> Type;
    if (hasId())
      D >> Id;
    D >> Ops;
  }
  std::vector<SPRVWord> Ops;
  bool GroupInst;
};

#define _SPRV_OP(x, y) \
  typedef SPRVInstTemplate<SPRVOC_Op##x, y> SPRV##x;
// CL 2.0 enqueue kernel builtins
_SPRV_OP(EnqueueMarker, true)
_SPRV_OP(EnqueueKernel, true)
_SPRV_OP(GetKernelNDrangeSubGroupCount, true)
_SPRV_OP(GetKernelNDrangeMaxSubGroupSize, true)
_SPRV_OP(GetKernelWorkGroupSize, true)
_SPRV_OP(GetKernelPreferredWorkGroupSizeMultiple, true)
_SPRV_OP(RetainEvent, false)
_SPRV_OP(ReleaseEvent, false)
_SPRV_OP(CreateUserEvent, true)
_SPRV_OP(IsValidEvent, true)
_SPRV_OP(SetUserEventStatus, false)
_SPRV_OP(CaptureEventProfilingInfo, false)
_SPRV_OP(GetDefaultQueue, true)
_SPRV_OP(BuildNDRange, true)
// CL 2.0 pipe builtins
_SPRV_OP(ReadPipe, true)
_SPRV_OP(WritePipe, true)
_SPRV_OP(ReservedReadPipe, true)
_SPRV_OP(ReservedWritePipe, true)
_SPRV_OP(ReserveReadPipePackets, true)
_SPRV_OP(ReserveWritePipePackets, true)
_SPRV_OP(CommitReadPipe, false)
_SPRV_OP(CommitWritePipe, false)
_SPRV_OP(IsValidReserveId, true)
_SPRV_OP(GetNumPipePackets, true)
_SPRV_OP(GetMaxPipePackets, true)
#undef _SPRV_OP

#define _SPRV_OP(x, y) \
  typedef SPRVInstTemplate<SPRVOC_Op##x, y, true> SPRV##x;
// Group instructions
_SPRV_OP(WaitGroupEvents, false)
_SPRV_OP(GroupAll, true)
_SPRV_OP(GroupAny, true)
_SPRV_OP(GroupBroadcast, true)
_SPRV_OP(GroupIAdd, true)
_SPRV_OP(GroupFAdd, true)
_SPRV_OP(GroupFMin, true)
_SPRV_OP(GroupUMin, true)
_SPRV_OP(GroupSMin, true)
_SPRV_OP(GroupFMax, true)
_SPRV_OP(GroupUMax, true)
_SPRV_OP(GroupSMax, true)
_SPRV_OP(GroupReserveReadPipePackets, true)
_SPRV_OP(GroupReserveWritePipePackets, true)
_SPRV_OP(GroupCommitReadPipe, false)
_SPRV_OP(GroupCommitWritePipe, false)
#undef _SPRV_OP

}

#endif // SPRVINSTRUCTION_HPP_
