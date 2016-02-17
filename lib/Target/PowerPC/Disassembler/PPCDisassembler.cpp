//===------ PPCDisassembler.cpp - Disassembler for PowerPC ------*- C++ -*-===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

#include "PPC.h"
#include "llvm/MC/MCDisassembler.h"
#include "llvm/MC/MCFixedLenDisassembler.h"
#include "llvm/MC/MCInst.h"
#include "llvm/MC/MCSubtargetInfo.h"
#include "llvm/Support/TargetRegistry.h"

using namespace llvm;

#define DEBUG_TYPE "ppc-disassembler"

typedef MCDisassembler::DecodeStatus DecodeStatus;

namespace {
class PPCDisassembler : public MCDisassembler {
public:
  PPCDisassembler(const MCSubtargetInfo &STI, MCContext &Ctx)
    : MCDisassembler(STI, Ctx) {}
  virtual ~PPCDisassembler() {}

  DecodeStatus getInstruction(MCInst &Instr, uint64_t &Size,
                              ArrayRef<uint8_t> Bytes, uint64_t Address,
                              raw_ostream &VStream,
                              raw_ostream &CStream) const override;
};
} // end anonymous namespace

static MCDisassembler *createPPCDisassembler(const Target &T,
                                             const MCSubtargetInfo &STI,
                                             MCContext &Ctx) {
  return new PPCDisassembler(STI, Ctx);
}

extern "C" void LLVMInitializePowerPCDisassembler() {
  // Register the disassembler for each target.
  TargetRegistry::RegisterMCDisassembler(ThePPC32Target,
                                         createPPCDisassembler);
  TargetRegistry::RegisterMCDisassembler(ThePPC64Target,
                                         createPPCDisassembler);
  TargetRegistry::RegisterMCDisassembler(ThePPC64LETarget,
                                         createPPCDisassembler);
}

// FIXME: These can be generated by TableGen from the existing register
// encoding values!

static const unsigned CRRegs[] = {
  PPC::CR0, PPC::CR1, PPC::CR2, PPC::CR3,
  PPC::CR4, PPC::CR5, PPC::CR6, PPC::CR7
};

static const unsigned CRBITRegs[] = {
  PPC::CR0LT, PPC::CR0GT, PPC::CR0EQ, PPC::CR0UN,
  PPC::CR1LT, PPC::CR1GT, PPC::CR1EQ, PPC::CR1UN,
  PPC::CR2LT, PPC::CR2GT, PPC::CR2EQ, PPC::CR2UN,
  PPC::CR3LT, PPC::CR3GT, PPC::CR3EQ, PPC::CR3UN,
  PPC::CR4LT, PPC::CR4GT, PPC::CR4EQ, PPC::CR4UN,
  PPC::CR5LT, PPC::CR5GT, PPC::CR5EQ, PPC::CR5UN,
  PPC::CR6LT, PPC::CR6GT, PPC::CR6EQ, PPC::CR6UN,
  PPC::CR7LT, PPC::CR7GT, PPC::CR7EQ, PPC::CR7UN
};

static const unsigned FRegs[] = {
  PPC::F0, PPC::F1, PPC::F2, PPC::F3,
  PPC::F4, PPC::F5, PPC::F6, PPC::F7,
  PPC::F8, PPC::F9, PPC::F10, PPC::F11,
  PPC::F12, PPC::F13, PPC::F14, PPC::F15,
  PPC::F16, PPC::F17, PPC::F18, PPC::F19,
  PPC::F20, PPC::F21, PPC::F22, PPC::F23,
  PPC::F24, PPC::F25, PPC::F26, PPC::F27,
  PPC::F28, PPC::F29, PPC::F30, PPC::F31
};

static const unsigned VRegs[] = {
  PPC::V0, PPC::V1, PPC::V2, PPC::V3,
  PPC::V4, PPC::V5, PPC::V6, PPC::V7,
  PPC::V8, PPC::V9, PPC::V10, PPC::V11,
  PPC::V12, PPC::V13, PPC::V14, PPC::V15,
  PPC::V16, PPC::V17, PPC::V18, PPC::V19,
  PPC::V20, PPC::V21, PPC::V22, PPC::V23,
  PPC::V24, PPC::V25, PPC::V26, PPC::V27,
  PPC::V28, PPC::V29, PPC::V30, PPC::V31
};

static const unsigned VSRegs[] = {
  PPC::VSL0, PPC::VSL1, PPC::VSL2, PPC::VSL3,
  PPC::VSL4, PPC::VSL5, PPC::VSL6, PPC::VSL7,
  PPC::VSL8, PPC::VSL9, PPC::VSL10, PPC::VSL11,
  PPC::VSL12, PPC::VSL13, PPC::VSL14, PPC::VSL15,
  PPC::VSL16, PPC::VSL17, PPC::VSL18, PPC::VSL19,
  PPC::VSL20, PPC::VSL21, PPC::VSL22, PPC::VSL23,
  PPC::VSL24, PPC::VSL25, PPC::VSL26, PPC::VSL27,
  PPC::VSL28, PPC::VSL29, PPC::VSL30, PPC::VSL31,

  PPC::VSH0, PPC::VSH1, PPC::VSH2, PPC::VSH3,
  PPC::VSH4, PPC::VSH5, PPC::VSH6, PPC::VSH7,
  PPC::VSH8, PPC::VSH9, PPC::VSH10, PPC::VSH11,
  PPC::VSH12, PPC::VSH13, PPC::VSH14, PPC::VSH15,
  PPC::VSH16, PPC::VSH17, PPC::VSH18, PPC::VSH19,
  PPC::VSH20, PPC::VSH21, PPC::VSH22, PPC::VSH23,
  PPC::VSH24, PPC::VSH25, PPC::VSH26, PPC::VSH27,
  PPC::VSH28, PPC::VSH29, PPC::VSH30, PPC::VSH31
};

static const unsigned VSFRegs[] = {
  PPC::F0, PPC::F1, PPC::F2, PPC::F3,
  PPC::F4, PPC::F5, PPC::F6, PPC::F7,
  PPC::F8, PPC::F9, PPC::F10, PPC::F11,
  PPC::F12, PPC::F13, PPC::F14, PPC::F15,
  PPC::F16, PPC::F17, PPC::F18, PPC::F19,
  PPC::F20, PPC::F21, PPC::F22, PPC::F23,
  PPC::F24, PPC::F25, PPC::F26, PPC::F27,
  PPC::F28, PPC::F29, PPC::F30, PPC::F31,

  PPC::VF0, PPC::VF1, PPC::VF2, PPC::VF3,
  PPC::VF4, PPC::VF5, PPC::VF6, PPC::VF7,
  PPC::VF8, PPC::VF9, PPC::VF10, PPC::VF11,
  PPC::VF12, PPC::VF13, PPC::VF14, PPC::VF15,
  PPC::VF16, PPC::VF17, PPC::VF18, PPC::VF19,
  PPC::VF20, PPC::VF21, PPC::VF22, PPC::VF23,
  PPC::VF24, PPC::VF25, PPC::VF26, PPC::VF27,
  PPC::VF28, PPC::VF29, PPC::VF30, PPC::VF31
};

static const unsigned GPRegs[] = {
  PPC::R0, PPC::R1, PPC::R2, PPC::R3,
  PPC::R4, PPC::R5, PPC::R6, PPC::R7,
  PPC::R8, PPC::R9, PPC::R10, PPC::R11,
  PPC::R12, PPC::R13, PPC::R14, PPC::R15,
  PPC::R16, PPC::R17, PPC::R18, PPC::R19,
  PPC::R20, PPC::R21, PPC::R22, PPC::R23,
  PPC::R24, PPC::R25, PPC::R26, PPC::R27,
  PPC::R28, PPC::R29, PPC::R30, PPC::R31
};

static const unsigned GP0Regs[] = {
  PPC::ZERO, PPC::R1, PPC::R2, PPC::R3,
  PPC::R4, PPC::R5, PPC::R6, PPC::R7,
  PPC::R8, PPC::R9, PPC::R10, PPC::R11,
  PPC::R12, PPC::R13, PPC::R14, PPC::R15,
  PPC::R16, PPC::R17, PPC::R18, PPC::R19,
  PPC::R20, PPC::R21, PPC::R22, PPC::R23,
  PPC::R24, PPC::R25, PPC::R26, PPC::R27,
  PPC::R28, PPC::R29, PPC::R30, PPC::R31
};

static const unsigned G8Regs[] = {
  PPC::X0, PPC::X1, PPC::X2, PPC::X3,
  PPC::X4, PPC::X5, PPC::X6, PPC::X7,
  PPC::X8, PPC::X9, PPC::X10, PPC::X11,
  PPC::X12, PPC::X13, PPC::X14, PPC::X15,
  PPC::X16, PPC::X17, PPC::X18, PPC::X19,
  PPC::X20, PPC::X21, PPC::X22, PPC::X23,
  PPC::X24, PPC::X25, PPC::X26, PPC::X27,
  PPC::X28, PPC::X29, PPC::X30, PPC::X31
};

static const unsigned QFRegs[] = {
  PPC::QF0, PPC::QF1, PPC::QF2, PPC::QF3,
  PPC::QF4, PPC::QF5, PPC::QF6, PPC::QF7,
  PPC::QF8, PPC::QF9, PPC::QF10, PPC::QF11,
  PPC::QF12, PPC::QF13, PPC::QF14, PPC::QF15,
  PPC::QF16, PPC::QF17, PPC::QF18, PPC::QF19,
  PPC::QF20, PPC::QF21, PPC::QF22, PPC::QF23,
  PPC::QF24, PPC::QF25, PPC::QF26, PPC::QF27,
  PPC::QF28, PPC::QF29, PPC::QF30, PPC::QF31
};

template <std::size_t N>
static DecodeStatus decodeRegisterClass(MCInst &Inst, uint64_t RegNo,
                                        const unsigned (&Regs)[N]) {
  assert(RegNo < N && "Invalid register number");
  Inst.addOperand(MCOperand::CreateReg(Regs[RegNo]));
  return MCDisassembler::Success;
}

static DecodeStatus DecodeCRRCRegisterClass(MCInst &Inst, uint64_t RegNo,
                                            uint64_t Address,
                                            const void *Decoder) {
  return decodeRegisterClass(Inst, RegNo, CRRegs);
}

static DecodeStatus DecodeCRBITRCRegisterClass(MCInst &Inst, uint64_t RegNo,
                                            uint64_t Address,
                                            const void *Decoder) {
  return decodeRegisterClass(Inst, RegNo, CRBITRegs);
}

static DecodeStatus DecodeF4RCRegisterClass(MCInst &Inst, uint64_t RegNo,
                                            uint64_t Address,
                                            const void *Decoder) {
  return decodeRegisterClass(Inst, RegNo, FRegs);
}

static DecodeStatus DecodeF8RCRegisterClass(MCInst &Inst, uint64_t RegNo,
                                            uint64_t Address,
                                            const void *Decoder) {
  return decodeRegisterClass(Inst, RegNo, FRegs);
}

static DecodeStatus DecodeVRRCRegisterClass(MCInst &Inst, uint64_t RegNo,
                                            uint64_t Address,
                                            const void *Decoder) {
  return decodeRegisterClass(Inst, RegNo, VRegs);
}

static DecodeStatus DecodeVSRCRegisterClass(MCInst &Inst, uint64_t RegNo,
                                            uint64_t Address,
                                            const void *Decoder) {
  return decodeRegisterClass(Inst, RegNo, VSRegs);
}

static DecodeStatus DecodeVSFRCRegisterClass(MCInst &Inst, uint64_t RegNo,
                                            uint64_t Address,
                                            const void *Decoder) {
  return decodeRegisterClass(Inst, RegNo, VSFRegs);
}

static DecodeStatus DecodeGPRCRegisterClass(MCInst &Inst, uint64_t RegNo,
                                            uint64_t Address,
                                            const void *Decoder) {
  return decodeRegisterClass(Inst, RegNo, GPRegs);
}

static DecodeStatus DecodeGPRC_NOR0RegisterClass(MCInst &Inst, uint64_t RegNo,
                                            uint64_t Address,
                                            const void *Decoder) {
  return decodeRegisterClass(Inst, RegNo, GP0Regs);
}

static DecodeStatus DecodeG8RCRegisterClass(MCInst &Inst, uint64_t RegNo,
                                            uint64_t Address,
                                            const void *Decoder) {
  return decodeRegisterClass(Inst, RegNo, G8Regs);
}

#define DecodePointerLikeRegClass0 DecodeGPRCRegisterClass
#define DecodePointerLikeRegClass1 DecodeGPRC_NOR0RegisterClass

static DecodeStatus DecodeQFRCRegisterClass(MCInst &Inst, uint64_t RegNo,
                                            uint64_t Address,
                                            const void *Decoder) {
  return decodeRegisterClass(Inst, RegNo, QFRegs);
}

#define DecodeQSRCRegisterClass DecodeQFRCRegisterClass
#define DecodeQBRCRegisterClass DecodeQFRCRegisterClass

template<unsigned N>
static DecodeStatus decodeUImmOperand(MCInst &Inst, uint64_t Imm,
                                      int64_t Address, const void *Decoder) {
  assert(isUInt<N>(Imm) && "Invalid immediate");
  Inst.addOperand(MCOperand::CreateImm(Imm));
  return MCDisassembler::Success;
}

template<unsigned N>
static DecodeStatus decodeSImmOperand(MCInst &Inst, uint64_t Imm,
                                      int64_t Address, const void *Decoder) {
  assert(isUInt<N>(Imm) && "Invalid immediate");
  Inst.addOperand(MCOperand::CreateImm(SignExtend64<N>(Imm)));
  return MCDisassembler::Success;
}

static DecodeStatus decodeMemRIOperands(MCInst &Inst, uint64_t Imm,
                                        int64_t Address, const void *Decoder) {
  // Decode the memri field (imm, reg), which has the low 16-bits as the
  // displacement and the next 5 bits as the register #.

  uint64_t Base = Imm >> 16;
  uint64_t Disp = Imm & 0xFFFF;

  assert(Base < 32 && "Invalid base register");

  switch (Inst.getOpcode()) {
  default: break;
  case PPC::LBZU:
  case PPC::LHAU:
  case PPC::LHZU:
  case PPC::LWZU:
  case PPC::LFSU:
  case PPC::LFDU:
    // Add the tied output operand.
    Inst.addOperand(MCOperand::CreateReg(GP0Regs[Base]));
    break;
  case PPC::STBU:
  case PPC::STHU:
  case PPC::STWU:
  case PPC::STFSU:
  case PPC::STFDU:
    Inst.insert(Inst.begin(), MCOperand::CreateReg(GP0Regs[Base]));
    break;
  }

  Inst.addOperand(MCOperand::CreateImm(SignExtend64<16>(Disp)));
  Inst.addOperand(MCOperand::CreateReg(GP0Regs[Base]));
  return MCDisassembler::Success;
}

static DecodeStatus decodeMemRIXOperands(MCInst &Inst, uint64_t Imm,
                                         int64_t Address, const void *Decoder) {
  // Decode the memrix field (imm, reg), which has the low 14-bits as the
  // displacement and the next 5 bits as the register #.

  uint64_t Base = Imm >> 14;
  uint64_t Disp = Imm & 0x3FFF;

  assert(Base < 32 && "Invalid base register");

  if (Inst.getOpcode() == PPC::LDU)
    // Add the tied output operand.
    Inst.addOperand(MCOperand::CreateReg(GP0Regs[Base]));
  else if (Inst.getOpcode() == PPC::STDU)
    Inst.insert(Inst.begin(), MCOperand::CreateReg(GP0Regs[Base]));

  Inst.addOperand(MCOperand::CreateImm(SignExtend64<16>(Disp << 2)));
  Inst.addOperand(MCOperand::CreateReg(GP0Regs[Base]));
  return MCDisassembler::Success;
}

static DecodeStatus decodeCRBitMOperand(MCInst &Inst, uint64_t Imm,
                                        int64_t Address, const void *Decoder) {
  // The cr bit encoding is 0x80 >> cr_reg_num.

  unsigned Zeros = countTrailingZeros(Imm);
  assert(Zeros < 8 && "Invalid CR bit value");

  Inst.addOperand(MCOperand::CreateReg(CRRegs[7 - Zeros]));
  return MCDisassembler::Success;
}

#include "PPCGenDisassemblerTables.inc"

DecodeStatus PPCDisassembler::getInstruction(MCInst &MI, uint64_t &Size,
                                             ArrayRef<uint8_t> Bytes,
                                             uint64_t Address, raw_ostream &OS,
                                             raw_ostream &CS) const {
  // Get the four bytes of the instruction.
  Size = 4;
  if (Bytes.size() < 4) {
    Size = 0;
    return MCDisassembler::Fail;
  }

  // The instruction is big-endian encoded.
  uint32_t Inst =
      (Bytes[0] << 24) | (Bytes[1] << 16) | (Bytes[2] << 8) | (Bytes[3] << 0);

  if ((STI.getFeatureBits() & PPC::FeatureQPX) != 0) {
    DecodeStatus result =
      decodeInstruction(DecoderTableQPX32, MI, Inst, Address, this, STI);
    if (result != MCDisassembler::Fail)
      return result;

    MI.clear();
  }

  return decodeInstruction(DecoderTable32, MI, Inst, Address, this, STI);
}

