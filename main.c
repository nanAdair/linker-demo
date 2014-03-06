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
void showSectionData(Section *);
int main(int argc, char *argv[])
{
    char obj_filename[100], so_filename[100], ld_filename[100], temp_filename[100];
    if (argc < 2) {
        printf("error in command usage\n");
        exit(EXIT_FAILURE);
    }
    
    strcpy(obj_filename, argv[1]);
    strcpy(so_filename, argv[2]);
    strcpy(ld_filename, argv[3]);
    
    Elf32_File *obj_file, *so_file, *temp_file;
    obj_file = GetBinaryFileData(obj_filename);
    so_file = GetBinaryFileData(so_filename);
    

    strcpy(temp_filename, argv[4]);
    temp_file = GetBinaryFileData(temp_filename);
    Section *temp_list;
    temp_list = GetSections(temp_file);
    Section *hash;
    hash = GetSectionByName(temp_list, ".hash");
    /*Section *test;*/
    /*test = GetSectionByName(temp_list, ".hash");*/
    /*showSectionData(test);*/
    
    
    /*showSectionInfo(obj_file->elf_section_table, obj_file->elf_file_header->e_shnum);*/
    /*showSectionInfo(so_file->elf_section_table, so_file->elf_file_header->e_shnum);*/
    
    Section *sec_list;
    sec_list = GetSections(obj_file);
    MergeSection(sec_list);

    Section *so_sec_list;
    so_sec_list = GetSections(so_file);
    
    Symbol *sym_list, *so_sym_list, *dyn_sym_list;
    
    sym_list = GetSymbols(obj_file, sec_list);
    so_sym_list = GetSymbols(so_file, so_sec_list);
    
    dyn_sym_list = MakeDynSymbol(sym_list, so_sym_list);
    
    /*showSection(sec_list);*/
    CreateSections(sec_list);
    
    //.interp
    UpdateInterpSection(sec_list, ld_filename);
    //.dynstr
    UpdateDynstrSection(sec_list, dyn_sym_list, so_filename);
    //.dynsym
    UpdateDynsymSection(sec_list, dyn_sym_list);
    //.gnu.version
    UpdateGVSection(sec_list, dyn_sym_list);
    //.hash
    /*UpdateHashSection(sec_list, dyn_sym_list);*/
    /* TODO: Faked content of hash section */
    UpdateHashSection(sec_list, dyn_sym_list, hash);
    //.gnu.version_r
    UpdateGNRSection(sec_list, dyn_sym_list, so_filename);
    //.plt, .got.plt, rel.plt
    UpdatePLTRelatedSections(sec_list, dyn_sym_list);
    //.got, .rel.dyn
    UpdateGOTRelatedSections(sec_list, dyn_sym_list);
    //.dynamic
    UpdateDynamicSection(sec_list, 1);
    /*UpdateDynamicSection(sec_list, 1);*/
    
    /*showSection(sec_list);*/
    /*Section *test;*/
    /*test = GetSectionByName(sec_list, INTERP_SECTION_NAME);*/
    /*test = GetSectionByName(sec_list, DYNSTR_SECTION_NAME);*/
    /*test = GetSectionByName(sec_list, DYNSYM_SECTION_NAME);*/
    /*test = GetSectionByName(sec_list, GV_SECTION_NAME);*/
    /*test = GetSectionByName(sec_list, GNR_SECTION_NAME);*/
    /*test = GetSectionByName(sec_list, PLT_SECTION_NAME);*/
    /*test = GetSectionByName(sec_list, GOT_PLT_SECTION_NAME);*/
    /*test = GetSectionByName(sec_list, REL_PLT_SECTION_NAME);*/
    /*showSectionData(test);*/
    showSection(sec_list);
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
        printf("%d %s %x\n", cur_section->sec_number, cur_section->sec_name, cur_section->sec_datasize);
        cur_section = cur_section->sec_next;
        i++;
    }
}

void showSectionData(Section *sec)
{
    FILE *output;
    output = fopen("output", "wb");
    
    fwrite(sec->sec_data, sec->sec_datasize, 1, output);
    
    fclose(output);
}

