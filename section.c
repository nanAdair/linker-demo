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
#include "sectionGD.h"
#include "file.h"

static void FillInterpSection(Section *, char *);
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

Section *GetSectionByName(Section *sec_list, UINT8 *sec_name)
{
    Section *cur_section;
    cur_section = sec_list;
    
    while (cur_section) {
        if (strcmp(cur_section->sec_name, sec_name) == 0)
            return cur_section;

        cur_section = cur_section->sec_next;
    }
    
    printf("error the %s section doesn't exits\n", sec_name);
    exit(EXIT_FAILURE);
}

void CreateSections(Section *sec_list)
{
    Section *tail_sec;
    tail_sec = sec_list;
    
    while (tail_sec->sec_next != NULL)
        tail_sec = tail_sec->sec_next;
    
    int i;
    for (i = 0; i < ADDEDSECTIONNUMBER; i++) {
        Section *new_section;
        new_section = (Section *)malloc(sizeof(Section));
        
        new_section->sec_type = AddedSectionsInfo[i].sh_type;
        new_section->sec_flags = AddedSectionsInfo[i].sh_flags;
        new_section->sec_address = AddedSectionsInfo[i].sh_addr;
        new_section->sec_file_offset = AddedSectionsInfo[i].sh_offset;
        new_section->sec_datasize = AddedSectionsInfo[i].sh_size;
        new_section->sec_link = AddedSectionsInfo[i].sh_link;
        new_section->sec_info = AddedSectionsInfo[i].sh_info;
        new_section->sec_align = AddedSectionsInfo[i].sh_addralign;
        new_section->sec_entsize = AddedSectionsInfo[i].sh_entsize;
        
        new_section->sec_name = (UINT8 *)malloc(strlen(AddedSectionsName[i]) + 1);
        strcpy(new_section->sec_name, AddedSectionsName[i]);
        
        new_section->sec_number = tail_sec->sec_number + 1;
        InsertSectionAfterSection(new_section, tail_sec);
        tail_sec = new_section;
    }
}

void UpdateInterpSection(Section *sec_list, char *ld_path)
{
    Section *interp_sec;
    interp_sec = GetSectionByName(sec_list, INTERP_SECTION_NAME);
    
    FillInterpSection(interp_sec, ld_path);
}

static void FillInterpSection(Section *interp_sec, char *ld_path)
{
    int length;
    length = strlen(ld_path);
    interp_sec->sec_data = (UINT8 *)malloc(length + 1);
    interp_sec->sec_datasize = interp_sec->sec_newdatasize = length + 1;
    strcpy(interp_sec->sec_data, ld_path);
}

/*void FillDynstrSection(Section *dynstr_sec, Symbol *dynsym_list, char *so_file_name)*/
/*{*/
    /*Symbol *cur_sym;k*/
/*}*/
