/*
 * =====================================================================================
 *
 *       Filename:  section.c
 *
 *    Description:  
 *
 *        Version:  1.0
 *        Created:  03/02/2014 02:40:57 PM
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
#include "section.h"
#include "file.h"

Section *GetSections(Elf32_File *elf_file)
{
    Section *cur_section, *last_section, *first_section;
    Elf32_Shdr *section_table;
    UINT8 *section_strn_table, *file_data;
    int i, numberOfSections;
    
    section_table = elf_file->elf_section_table;
    section_strn_table = elf_file->elf_section_strn_table;
    file_data = elf_file->elf_file_data;
    numberOfSections = elf_file->elf_file_header->e_shnum;
    
    cur_section = last_section = first_section = NULL;
    
    for (i = 0; i < numberOfSections; i++) {
        Elf32_Shdr *cur_sec_dr;
        cur_sec_dr = section_table + i;
        
        cur_section = (Section *)malloc(sizeof(Section));
        
        cur_section->sec_number = i;
        cur_section->sec_data = (UINT8 *)malloc(cur_sec_dr->sh_size);
        memcpy(cur_section->sec_data, file_data + cur_sec_dr->sh_offset, cur_sec_dr->sh_size);
        cur_section->sec_datasize = cur_section->sec_newdatasize = cur_sec_dr->sh_size;
        cur_section->sec_address = cur_sec_dr->sh_addr;
        cur_section->sec_type = cur_sec_dr->sh_type;
        cur_section->sec_flags = cur_sec_dr->sh_flags;
        cur_section->sec_link = cur_sec_dr->sh_link;
        cur_section->sec_info = cur_sec_dr->sh_info;
        cur_section->sec_align = cur_sec_dr->sh_addralign;
        cur_section->sec_entsize = cur_sec_dr->sh_entsize;
        cur_section->sec_name_offset = cur_sec_dr->sh_name;
        cur_section->sec_file_offset = cur_sec_dr->sh_offset;
        
        int name_length = strlen(section_strn_table + cur_section->sec_name_offset);
        cur_section->sec_name = (UINT8 *)malloc(name_length + 1);
        strcpy(cur_section->sec_name, section_strn_table + cur_section->sec_name_offset);
        
        /*if (last_section == NULL) {*/
            /*first_section = cur_section;*/
            /*cur_section->sec_prev = NULL;*/
            /*cur_section->sec_next = NULL;*/
            /*last_section = cur_section;*/
        /*}*/
        /*else {*/
            /*last_section->sec_next = cur_section;*/
            /*cur_section->sec_prev = last_section;*/
            /*cur_section->sec_next = NULL;*/
            /*last_section = cur_section;*/
        /*}*/
        
        if (first_section == NULL) {
            first_section = cur_section;
            last_section = cur_section;
        }
        else {
            InsertSectionAfterSection(cur_section, last_section);
            last_section = cur_section;
        }
    }
    
    return first_section;
}

// new_section is inserted after section
void InsertSectionAfterSection(Section *new_section, Section *section)
{
    new_section->sec_prev = section;
    new_section->sec_next = section->sec_next;
    section->sec_next = new_section;
    
    if (new_section->sec_next != NULL) {
        new_section->sec_next->sec_prev = new_section;
    }
}
