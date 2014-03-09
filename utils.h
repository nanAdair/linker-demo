/*
 * =====================================================================================
 *
 *       Filename:  utils.h
 *
 *    Description:  
 *
 *        Version:  1.0
 *        Created:  03/03/2014 10:32:06 AM
 *       Revision:  none
 *       Compiler:  gcc
 *
 *         Author:  YOUR NAME (), 
 *   Organization:  
 *
 * =====================================================================================
 */

#ifndef UTILS_H
#define UTILS_H

#include "type.h"
#include "section.h"

typedef struct Dyn {
    INT32 d_tag;
    union {
        UINT32 d_val;
        UINT32 d_ptr;
    } d_un;
} Dyn;

typedef struct {
    UINT8 *content;
    UINT32 offset;
    UINT32 number;
    UINT32 str_index;
} Section_Table;

struct Section;
int GetDynDataByTag(struct Section *, int);
struct Section *SortSectionsByWriteOrder(struct Section *);
void AllocateAddress(struct Section *);
Section_Table *CreateSectionTable(struct Section *);
UINT8 *CreateProgramHeaderTable(struct Section *);
Elf32_Ehdr *CreateElfHeader(Elf32_Ehdr *ehdr, struct Section *sec_list, Section_Table *sec_tab);
void WriteOut(Elf32_Ehdr *ehdr_data, UINT8 *phdr_data, struct Section *sec_list, Section_Table *sec_tab);
#endif
