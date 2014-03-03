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

#define MAGIC (-1)

// Section to be added
#define GOT_SECTION_NAME        ".got"
#define GOT_PLT_SECTION_NAME    ".got.plt"
#define PLT_SECTION_NAME        ".plt"
#define INTERP_SECTION_NAME     ".interp"
#define HASH_SECTION_NAME       ".hash"
#define DYNSYM_SECTION_NAME     ".dynsym"
#define DYNSTR_SECTION_NAME     ".dynstr"
#define GV_SECTION_NAME         ".gnu.version"
#define GNR_SECTION_NAME        ".gnu.version_r"
#define REL_DYN_SECTION_NAME    ".rel.dyn"
#define REL_PLT_SECTION_NAME    ".rel.plt"

typedef struct Section {
    short sec_number;
    struct Section *sec_prev;
    struct Section *sec_next;
    struct Section *sec_mergeto;
    struct Section *sec_relsec;
    UINT8 *sec_data;
    UINT8 *sec_newdata;
    int sec_datasize;
    int sec_newdatasize;
    int sec_delta; // 段合并后在新的段中的偏移
    int sec_address;
    int sec_newaddress;
    char *sec_name;
    int sec_type;
    int sec_flags;
    int sec_link;
    int sec_info;
    int sec_align;
    int sec_entsize;
    int sec_name_offset; // 在段字符串表中的偏移
    int sec_file_offset; // 在文件中的偏移
} Section;

// function declaration
Section *GetSections(Elf32_File *);
void InsertSectionAfterSection(Section *, Section *);
#endif
