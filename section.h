/*
 * =====================================================================================
 *
 *       Filename:  section.h
 *
 *    Description:  data structure for section
 *
 *        Version:  1.0
 *        Created:  03/02/2014 11:29:48 AM
 *       Revision:  none
 *       Compiler:  gcc
 *
 *         Author:  YOUR NAME (), 
 *   Organization:  
 *
 * =====================================================================================
 */
#ifndef SECTION_H
#define SECTION_H

#include "type.h"
#include "file.h"
#include "symbol.h"
#include "instr.h"

#define MAGIC (-1)

// Section to be added
#define INTERP_SECTION_NAME     ".interp"
#define HASH_SECTION_NAME       ".hash"
#define DYNSYM_SECTION_NAME     ".dynsym"
#define DYNSTR_SECTION_NAME     ".dynstr"
#define GV_SECTION_NAME         ".gnu.version"
#define GNR_SECTION_NAME        ".gnu.version_r"
#define REL_DYN_SECTION_NAME    ".rel.dyn"
#define REL_PLT_SECTION_NAME    ".rel.plt"
#define PLT_SECTION_NAME        ".plt"
#define DYNAMIC_SECTION_NAME    ".dynamic"
#define GOT_SECTION_NAME        ".got"
#define GOT_PLT_SECTION_NAME    ".got.plt"

#define TEXT_SECTION_NAME       ".text"
#define INIT_SECTION_NAME       ".init"
#define FINI_SECTION_NAME       ".fini"
#define INIT_ARRAY_SECTION_NAME ".init_array"
#define FINI_ARRAY_SECTION_NAME ".fini_array"

#define BSS_SECTION_NAME        ".bss"
#define RODATA_SECION_NAME      ".rodata"
#define EH_FRAME_SECTION_NAME   ".eh_frame"
#define JCR_SECTION_NAME        ".jcr"
#define DATA_SECTION_NAME       ".data"
#define COMMENT_SECTION_NAME    ".comment"
#define SYMTAB_SECTION_NAME     ".symtab"
#define STRTAB_SECTION_NAME     ".strtab"

#define SHSTRTAB_SECTION_NAME   ".shstrtab"
#define NOTE_SECTION_NAME       ".note.ABI-tag"

#define DYNAMIC_NUMBER 28
#define DYNAMIC_ENTSIZE 8

typedef struct Section {
    UINT16 sec_number;
    UINT16 sec_final_number;
    struct Section *sec_prev;
    struct Section *sec_next;
    struct Section *sec_mergeto;
    struct Section *sec_relsec;
    UINT8 *sec_data;
    UINT8 *sec_newdata;
    UINT32 sec_datasize;
    UINT32 sec_newdatasize;
    UINT32 sec_delta; // 段合并后在新的段中的偏移
    UINT32 sec_address;
    UINT32 sec_newaddress;
    UINT8 *sec_name;
    UINT32 sec_type; 
    UINT32 sec_flags;
    UINT32 sec_link;
    UINT32 sec_info;
    UINT32 sec_align;
    UINT32 sec_entsize;
    UINT32 sec_name_offset; // 在段字符串表中的偏移
    UINT32 sec_file_offset; // 在文件中的偏移
    UINT32 sec_misc; // 用来保存段的优先级
} Section;

struct Symbol;
// function declaration
Section *GetSections(Elf32_File *);
Section *GetSectionByIndex(Section *, UINT16);
Section *GetSectionByName(Section *, UINT8 *);
void InsertSectionAfterSection(Section *, Section *);
void CreateSections(Section *);
void UpdateInterpSection(Section *, char *);
void UpdateDynstrSection(Section *, struct Symbol *, char *);
void UpdateShstrSection(Section *);
void UpdateDynsymSection(Section *, struct Symbol *);
void UpdateGVSection(Section *, struct Symbol *);
void UpdateHashSectionNew(Section *, struct Symbol *);
void UpdateHashSection(Section *, struct Symbol *, Section *);
void UpdateGNRSection(Section *, struct Symbol *, char *);
void UpdatePLTRelatedSections(Section *, struct Symbol *);
void UpdateGOTRelatedSections(Section *, struct Symbol *);
void UpdateDynamicSection(Section *, int);
void UpdateEhframeSection(Section *sec_list);

void RenewRelGOTSection(Section *);
void RenewRelPLTSection(Section *);
void RenewPLTSection(Section *);
void RenewGOTPLTSection(Section *);
void RenewSectionInfo(Section *);
void RenewDynamicSection(Section *);
void RenewSymbolSection(Section *sec_list, Section *merge_list, Section *sec, struct Symbol *);

//void AddDynstrEntryFromName(Section *, struct Symbol *);
//void AddDynsymEntry(Section *, struct Symbol *);
//void AddPLTEntry(Section *, int);
//void AddGOTorGOTPLTEntry(Section *);
//void AddRelPLTEntry(Section *, struct Symbol *, int);
//void AddRelGOTEntry(Section *, struct Symbol *, int);
//int FindOffset(Section *, char *);

Section *MergeSection(Section *);
void DropSection(Section *);
#endif
