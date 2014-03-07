/*
 * =====================================================================================
 *
 *       Filename:  utils.c
 *
 *    Description:  
 *
 *        Version:  1.0
 *        Created:  03/03/2014 10:38:42 AM
 *       Revision:  none
 *       Compiler:  gcc
 *
 *         Author:  YOUR NAME (), 
 *   Organization:  
 *
 * =====================================================================================
 */
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "utils.h"

static void RoundUpSection(Section *, int);
int GetDynDataByTag(Section *dynamic, int tag)
{
    int i, number;
    number = dynamic->sec_datasize / dynamic->sec_entsize;

    for (i = 0; i < number; i++) {
        Dyn *cur_dyn;
        cur_dyn = (Dyn *)(dynamic->sec_data + dynamic->sec_entsize * i);
        if (cur_dyn->d_tag == tag) {
            return cur_dyn->d_un.d_val;
        }
    }
    
    printf("error %x tag cann't be found\n");
    exit(EXIT_FAILURE);
}

int GetSectionOrderScore(Section *sec)
{
    int n;
    /* New Sections: 0
     * Sections from object file: 50*/
    n = sec->sec_misc ? 50 : 0;
    n += sec->sec_number;
    
    /* NULL section: 0 */
    if (sec->sec_type == SHT_NULL)
        return 0;
    
    /* .interp 1 */
    if (!strcmp(sec->sec_name, INTERP_SECTION_NAME)) {
        n = 1;
        return n;
    }
    
    /*  .note.ABI-tag 2 */
    if (sec->sec_type == SHT_NOTE) {
        n = 2;
        return n;
    }
    
    /*  A: +1000 */
    if (sec->sec_flags == SHF_ALLOC) {
        return 1000 + n;;
    }
    
    /*  AX: +1000 */
    if (sec->sec_flags == (SHF_ALLOC | SHF_EXECINSTR)) {
        return 1000 + n;
    }
    
    /* WA: +2000 */
    if (sec->sec_flags == (SHF_ALLOC | SHF_WRITE)) {
        return 2000 + n;
    }
    
    /* STR: +3000 */
    if (sec->sec_flags == SHF_STRINGS) {
        return 3000 + n;
    }
    
    /* Symbol Table: +7000 */
    if (sec->sec_type == SHT_SYMTAB) {
        return 7000 + n;
    }
    
    /* String Table: +8000 */
    if (sec->sec_type == SHT_STRTAB) {
        return 8000 + n;
    }
    
    return 6000 + n;
}

Section *SortSectionsByWriteOrder(Section *sec_list) 
{
    int i, j, low, number_sections;
    number_sections = 0;
    
    Section *cur_sec, *first_sec, *init, *res_list;
    cur_sec = first_sec = sec_list;
    init = GetSectionByName(sec_list, INIT_SECTION_NAME);
    
    while (cur_sec) {
        if ((cur_sec->sec_misc != 0) && (cur_sec->sec_type == SHT_REL)) {
            cur_sec = cur_sec->sec_next;
            continue;
        }
        
        if (!strcmp(cur_sec->sec_name, PLT_SECTION_NAME)) {
            cur_sec->sec_number = init->sec_number + 1;
            cur_sec->sec_misc = init->sec_misc + 1;
        }
        
        cur_sec->sec_misc = GetSectionOrderScore(cur_sec);
        
        number_sections++;
        cur_sec = cur_sec->sec_next;
    }
    
    Section **list;
    cur_sec = first_sec;
    list = malloc(sizeof(Section *) * number_sections);
    number_sections = 0;
    
    while (cur_sec) {
        if ((cur_sec->sec_misc != 0) && cur_sec->sec_type == SHT_REL) {
            cur_sec = cur_sec->sec_next;
            continue;
        }
        
        list[number_sections++] = cur_sec;
        cur_sec = cur_sec->sec_next;
    }
    
    for (i = 0; i < number_sections; i++) {
        low = i;
        for (j = i + 1; j < number_sections; j++) {
            if (list[j]->sec_misc < list[low]->sec_misc)
                low = j;
        }
        
        if (low != i) {
            Section *temp_sec;
            temp_sec = list[i];
            list[i] = list[low];
            list[low] = temp_sec;
        }
    }
    
    res_list = list[0];
    res_list->sec_next = res_list->sec_prev = NULL;
    
    for (i = 0; i < number_sections; i++) {
        if (i > 0)
            InsertSectionAfterSection(list[i], list[i-1]);
    }
    
    return res_list;
}

void AllocateAddress(Section *sec_list)
{
    UINT32 base_addr, offset;
    base_addr = 0x8048000;
    offset = 0x114;
    
    Section *cur_sec, *last_sec;
    cur_sec = sec_list;
    
    while (cur_sec) {
        /*if (cur_sec->sec_flags & SHF_ALLOC) {*/
        UINT32 align = 1;
        /*align <<= cur_sec->sec_align;*/
        align = cur_sec->sec_align ? cur_sec->sec_align : 1;
        int addition = 0;
        while ((offset + addition) % align != 0)
            addition++;
        if (addition != 0)
            RoundUpSection(last_sec, addition);
        offset += addition;
        /*printf("%d %d %x\n", align, addition, offset);*/
        
        if (cur_sec->sec_flags & SHF_ALLOC)
            cur_sec->sec_address = base_addr + offset;
        else
            cur_sec->sec_address = 0;
        cur_sec->sec_file_offset = offset;
        
        offset += cur_sec->sec_datasize;
        
        last_sec = cur_sec;
        cur_sec = cur_sec->sec_next;
    }
}

static void RoundUpSection(Section *sec, int add)
{
    UINT32 datasize, newdatasize;
    datasize = sec->sec_datasize;
    newdatasize = datasize + add;
    
    UINT8 *buffer;
    buffer = malloc(newdatasize);
    memset(buffer, 0x0, newdatasize);
    memcpy(buffer, sec->sec_data, datasize);
    
    free(sec->sec_data);
    sec->sec_data = buffer;
}
