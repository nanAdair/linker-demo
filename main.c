/*
 * =====================================================================================
 *
 *       Filename:  main.c
 *
 *    Description:  
 *
 *        Version:  1.0
 *        Created:  03/02/2014 11:00:41 AM
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
#include <elf.h>

#include "global.h"

void showSectionInfo(Elf32_Shdr *elf_section_table, int numberOfSections);
void showSection(Section *section);
int main(int argc, char *argv[])
{
    char obj_filename[100], so_filename[100], ld_filename[100];
    if (argc < 2) {
        printf("error in command usage\n");
        exit(EXIT_FAILURE);
    }
    
    strcpy(obj_filename, argv[1]);
    strcpy(so_filename, argv[2]);
    /*strcpy(ld_filename, argv[3]);*/
    
    Elf32_File *obj_file, *so_file;
    obj_file = GetBinaryFileData(obj_filename);
    so_file = GetBinaryFileData(so_filename);
    
    /*showSectionInfo(obj_file->elf_section_table, obj_file->elf_file_header->e_shnum);*/
    /*showSectionInfo(so_file->elf_section_table, so_file->elf_file_header->e_shnum);*/
    
    Section *sec_list;
    sec_list = GetSections(obj_file);

    Section *so_sec_list;
    so_sec_list = GetSections(so_file);
    
    GetSymbolVersions(so_sec_list);
    
    /*showSection(sec_list);*/
}

void showSectionInfo(Elf32_Shdr *elf_section_table, int numberOfSections)
{
    int i;
    Elf32_Shdr *cur_section;
    for (i = 0; i < numberOfSections; i++) {
        cur_section = elf_section_table + i;
        /*printf("The section id is %d\n", cur_section->sh_name);*/
        printf("%d The section offset is %6x\n", i, cur_section->sh_offset);
        /*printf("%d\n", i);*/
    }
}

void showSection(Section *section)
{
    Section *cur_section;
    
    cur_section = section;
    int i = 0;
    
    while (cur_section != NULL) {
        printf("%d %s\n", cur_section->sec_number, cur_section->sec_name);
        cur_section = cur_section->sec_next;
        i++;
    }
}
