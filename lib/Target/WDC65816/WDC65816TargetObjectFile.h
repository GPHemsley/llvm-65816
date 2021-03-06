//===-- WDC65816TargetObjectFile.h - WDC65816 Object Info -------------*- C++ -*-===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

#ifndef LLVM_TARGET_WDC65816_TARGETOBJECTFILE_H
#define LLVM_TARGET_WDC65816_TARGETOBJECTFILE_H

#include "WDC65816Section.h"
#include "llvm/Target/TargetLoweringObjectFile.h"
#include <string>

namespace llvm {
    class GlobalVariable;
    class Module;
    
    class WDC65816TargetObjectFile : public TargetLoweringObjectFile {
        
    public:
        WDC65816TargetObjectFile() {
            TextSection = 0;
            DataSection = 0;
            BSSSection = 0;
            ReadOnlySection = 0;
            
            StaticCtorSection = 0;
            StaticDtorSection = 0;
            LSDASection = 0;
            EHFrameSection = 0;
            DwarfAbbrevSection = 0;
            DwarfInfoSection = 0;
            DwarfLineSection = 0;
            DwarfFrameSection = 0;
            DwarfPubTypesSection = 0;
            DwarfDebugInlineSection = 0;
            DwarfStrSection = 0;
            DwarfLocSection = 0;
            DwarfARangesSection = 0;
            DwarfRangesSection = 0;
            DwarfMacroInfoSection = 0;
        }
        
        virtual ~WDC65816TargetObjectFile();
        
        virtual void Initialize(MCContext &ctx, const TargetMachine &TM) {
            TargetLoweringObjectFile::Initialize(ctx, TM);
            TextSection = new WDC65816Section(MCSection::SV_ELF, SectionKind::getText());
            DataSection = new WDC65816Section(MCSection::SV_ELF, SectionKind::getDataRel());
            BSSSection = new WDC65816Section(MCSection::SV_ELF, SectionKind::getBSS());
            ReadOnlySection = new WDC65816Section(MCSection::SV_ELF, SectionKind::getReadOnly());
            
            StaticCtorSection = new WDC65816Section(MCSection::SV_ELF, SectionKind::getMetadata());
            StaticDtorSection = new WDC65816Section(MCSection::SV_ELF, SectionKind::getMetadata());
            LSDASection = new WDC65816Section(MCSection::SV_ELF, SectionKind::getMetadata());
            EHFrameSection = new WDC65816Section(MCSection::SV_ELF, SectionKind::getMetadata());
            DwarfAbbrevSection = new WDC65816Section(MCSection::SV_ELF, SectionKind::getMetadata());
            DwarfInfoSection = new WDC65816Section(MCSection::SV_ELF, SectionKind::getMetadata());
            DwarfLineSection = new WDC65816Section(MCSection::SV_ELF, SectionKind::getMetadata());
            DwarfFrameSection = new WDC65816Section(MCSection::SV_ELF, SectionKind::getMetadata());
            DwarfPubTypesSection = new WDC65816Section(MCSection::SV_ELF, SectionKind::getMetadata());
            DwarfDebugInlineSection = new WDC65816Section(MCSection::SV_ELF, SectionKind::getMetadata());
            DwarfStrSection = new WDC65816Section(MCSection::SV_ELF, SectionKind::getMetadata());
            DwarfLocSection = new WDC65816Section(MCSection::SV_ELF, SectionKind::getMetadata());
            DwarfARangesSection = new WDC65816Section(MCSection::SV_ELF, SectionKind::getMetadata());
            DwarfRangesSection = new WDC65816Section(MCSection::SV_ELF, SectionKind::getMetadata());
            DwarfMacroInfoSection = new WDC65816Section(MCSection::SV_ELF, SectionKind::getMetadata());
        }
        
        virtual const MCSection *getSectionForConstant(SectionKind Kind) const {
            return ReadOnlySection;
        }
        
        virtual const MCSection *
        getExplicitSectionGlobal(const GlobalValue *GV, SectionKind Kind,
                                 Mangler *Mang, const TargetMachine &TM) const {
            return DataSection;
        }
        
    };
    
} // end namespace llvm

#endif
