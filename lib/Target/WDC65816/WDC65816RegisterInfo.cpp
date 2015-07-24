//===-- WDC65816RegisterInfo.cpp - WDC65816 Register Information ----------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
//
// This file contains the WDC65816 implementation of the TargetRegisterInfo class.
//
//===----------------------------------------------------------------------===//

#include "WDC65816RegisterInfo.h"
#include "WDC65816.h"
#include "WDC65816MachineFunctionInfo.h"
#include "WDC65816Subtarget.h"
#include "llvm/ADT/BitVector.h"
#include "llvm/ADT/STLExtras.h"
#include "llvm/CodeGen/MachineFrameInfo.h"
#include "llvm/CodeGen/MachineFunction.h"
#include "llvm/CodeGen/MachineInstrBuilder.h"
#include "llvm/IR/Type.h"
#include "llvm/Support/CommandLine.h"
#include "llvm/Support/ErrorHandling.h"
#include "llvm/Target/TargetInstrInfo.h"

#define GET_REGINFO_TARGET_DESC
#include "WDC65816GenRegisterInfo.inc"

using namespace llvm;

#if 0 // TODO - What is this?
static cl::opt<bool>
ReserveAppRegisters("sparc-reserve-app-registers", cl::Hidden, cl::init(false),
                    cl::desc("Reserve application registers (%g2-%g4)"));
#endif

WDC65816RegisterInfo::WDC65816RegisterInfo(WDC65816Subtarget &st)
: WDC65816GenRegisterInfo(WDC::P), Subtarget(st) {
}

const uint16_t* WDC65816RegisterInfo::getCalleeSavedRegs(const MachineFunction *MF)
const {
    return CSR_NoRegs_SaveList;
}

const uint32_t*
WDC65816RegisterInfo::getCallPreservedMask(CallingConv::ID CC) const {
    return CSR_NoRegs_RegMask;
}

const uint32_t*
WDC65816RegisterInfo::getRTCallPreservedMask(CallingConv::ID CC) const {
    return CSR_NoRegs_RegMask;
}

BitVector WDC65816RegisterInfo::getReservedRegs(const MachineFunction &MF) const {
    BitVector Reserved(getNumRegs());
    // FIXME: G1 reserved for now for large imm generation by frame code.
    Reserved.set(WDC::P);
    Reserved.set(WDC::S);
    Reserved.set(WDC::D);
    Reserved.set(WDC::K);
    Reserved.set(WDC::B);
    Reserved.set(WDC::PC);
    Reserved.set(WDC::dpI32_0);     // This register is used as the frame pointer.
    
    return Reserved;
}


unsigned WDC65816RegisterInfo::getFrameRegister(const MachineFunction &MF) const {
    return WDC::dpI32_0;
}


const TargetRegisterClass*
WDC65816RegisterInfo::getPointerRegClass(const MachineFunction &MF,
                                      unsigned Kind) const {
    // WDC_TODO - this is not a pointer reg actually.  This should end up being the
    // 32-bit direct page registers which are not defined yet.
    return &WDC::IndexRegsRegClass;
}


void
WDC65816RegisterInfo::eliminateFrameIndex(MachineBasicBlock::iterator II,
                                       int SPAdj, unsigned FIOperandNum,
                                       RegScavenger *RS) const {
    // WDC_TODO - Write something here...
}

#if 0 // TODO - How much of this stuff do I need?
static void replaceFI(MachineFunction &MF,
                      MachineBasicBlock::iterator II,
                      MachineInstr &MI,
                      DebugLoc dl,
                      unsigned FIOperandNum, int Offset,
                      unsigned FramePtr)
{
    // Replace frame index with a frame pointer reference.
    if (Offset >= -4096 && Offset <= 4095) {
        // If the offset is small enough to fit in the immediate field, directly
        // encode it.
        MI.getOperand(FIOperandNum).ChangeToRegister(FramePtr, false);
        MI.getOperand(FIOperandNum + 1).ChangeToImmediate(Offset);
        return;
    }
    
    const TargetInstrInfo &TII = *MF.getTarget().getInstrInfo();
    
    // FIXME: it would be better to scavenge a register here instead of
    // reserving G1 all of the time.
    if (Offset >= 0) {
        // Emit nonnegaive immediates with sethi + or.
        // sethi %hi(Offset), %g1
        // add %g1, %fp, %g1
        // Insert G1+%lo(offset) into the user.
        BuildMI(*MI.getParent(), II, dl, TII.get(SP::SETHIi), SP::G1)
        .addImm(HI22(Offset));
        
        
        // Emit G1 = G1 + I6
        BuildMI(*MI.getParent(), II, dl, TII.get(SP::ADDrr), SP::G1).addReg(SP::G1)
        .addReg(FramePtr);
        // Insert: G1+%lo(offset) into the user.
        MI.getOperand(FIOperandNum).ChangeToRegister(SP::G1, false);
        MI.getOperand(FIOperandNum + 1).ChangeToImmediate(LO10(Offset));
        return;
    }
    
    // Emit Negative numbers with sethi + xor
    // sethi %hix(Offset), %g1
    // xor  %g1, %lox(offset), %g1
    // add %g1, %fp, %g1
    // Insert: G1 + 0 into the user.
    BuildMI(*MI.getParent(), II, dl, TII.get(SP::SETHIi), SP::G1)
    .addImm(HIX22(Offset));
    BuildMI(*MI.getParent(), II, dl, TII.get(SP::XORri), SP::G1)
    .addReg(SP::G1).addImm(LOX10(Offset));
    
    BuildMI(*MI.getParent(), II, dl, TII.get(SP::ADDrr), SP::G1).addReg(SP::G1)
    .addReg(FramePtr);
    // Insert: G1+%lo(offset) into the user.
    MI.getOperand(FIOperandNum).ChangeToRegister(SP::G1, false);
    MI.getOperand(FIOperandNum + 1).ChangeToImmediate(0);
}


void
SparcRegisterInfo::eliminateFrameIndex(MachineBasicBlock::iterator II,
                                       int SPAdj, unsigned FIOperandNum,
                                       RegScavenger *RS) const {
    assert(SPAdj == 0 && "Unexpected");
    
    MachineInstr &MI = *II;
    DebugLoc dl = MI.getDebugLoc();
    int FrameIndex = MI.getOperand(FIOperandNum).getIndex();
    
    // Addressable stack objects are accessed using neg. offsets from %fp
    MachineFunction &MF = *MI.getParent()->getParent();
    int64_t Offset = MF.getFrameInfo()->getObjectOffset(FrameIndex) +
    MI.getOperand(FIOperandNum + 1).getImm() +
    Subtarget.getStackPointerBias();
    SparcMachineFunctionInfo *FuncInfo = MF.getInfo<SparcMachineFunctionInfo>();
    unsigned FramePtr = SP::I6;
    if (FuncInfo->isLeafProc()) {
        // Use %sp and adjust offset if needed.
        FramePtr = SP::O6;
        int stackSize = MF.getFrameInfo()->getStackSize();
        Offset += (stackSize) ? Subtarget.getAdjustedFrameSize(stackSize) : 0 ;
    }
    
    if (!Subtarget.isV9() || !Subtarget.hasHardQuad()) {
        if (MI.getOpcode() == SP::STQFri) {
            const TargetInstrInfo &TII = *MF.getTarget().getInstrInfo();
            unsigned SrcReg = MI.getOperand(2).getReg();
            unsigned SrcEvenReg = getSubReg(SrcReg, SP::sub_even64);
            unsigned SrcOddReg  = getSubReg(SrcReg, SP::sub_odd64);
            MachineInstr *StMI =
            BuildMI(*MI.getParent(), II, dl, TII.get(SP::STDFri))
            .addReg(FramePtr).addImm(0).addReg(SrcEvenReg);
            replaceFI(MF, II, *StMI, dl, 0, Offset, FramePtr);
            MI.setDesc(TII.get(SP::STDFri));
            MI.getOperand(2).setReg(SrcOddReg);
            Offset += 8;
        } else if (MI.getOpcode() == SP::LDQFri) {
            const TargetInstrInfo &TII = *MF.getTarget().getInstrInfo();
            unsigned DestReg     = MI.getOperand(0).getReg();
            unsigned DestEvenReg = getSubReg(DestReg, SP::sub_even64);
            unsigned DestOddReg  = getSubReg(DestReg, SP::sub_odd64);
            MachineInstr *StMI =
            BuildMI(*MI.getParent(), II, dl, TII.get(SP::LDDFri), DestEvenReg)
            .addReg(FramePtr).addImm(0);
            replaceFI(MF, II, *StMI, dl, 1, Offset, FramePtr);
            
            MI.setDesc(TII.get(SP::LDDFri));
            MI.getOperand(0).setReg(DestOddReg);
            Offset += 8;
        }
    }
    
    replaceFI(MF, II, MI, dl, FIOperandNum, Offset, FramePtr);
    
}

unsigned SparcRegisterInfo::getFrameRegister(const MachineFunction &MF) const {
    return SP::I6;
}
#endif

