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
void showSectionData(FILE *, Section *);
void showSectionTable(Section_Table *sec_tab);
char *PrepareStr(char *str);
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
    
    Section *sec_list, *merge_list;
    merge_list = NULL;
    sec_list = GetSections(obj_file);
    merge_list = MergeSection(sec_list);

    Section *so_sec_list;
    so_sec_list = GetSections(so_file);
    
    Relocation *rel_list;
    rel_list = getRel(obj_file);
    
    Symbol *sym_list, *so_sym_list, *dyn_sym_list;
    
    sym_list = GetSymbols(obj_file, sec_list);
    so_sym_list = GetSymbols(so_file, so_sec_list);
    
    dyn_sym_list = MakeDynSymbol(sym_list, so_sym_list, rel_list);
    
    char *filename;
    filename = PrepareStr(so_filename);
    printf("%s\n", filename);

    FILE *output;
    output = fopen("sec_test", "wb");
    
    /*showSection(sec_list);*/
    CreateSections(sec_list);
    
    //.interp
    UpdateInterpSection(sec_list, ld_filename);
    //.dynstr
    UpdateDynstrSection(sec_list, dyn_sym_list, filename);
    /*UpdateDynstrSection(sec_list, dyn_sym_list, so_filename);*/
    //.dynsym
    UpdateDynsymSection(sec_list, dyn_sym_list);
    //.gnu.version
    UpdateGVSection(sec_list, dyn_sym_list);
    //.hash
    /*UpdateHashSection(sec_list, dyn_sym_list);*/
    /* TODO: Faked content of hash section */
    UpdateHashSection(sec_list, dyn_sym_list, hash);
    //.gnu.version_r
    UpdateGNRSection(sec_list, dyn_sym_list, filename);
    /*UpdateGNRSection(sec_list, dyn_sym_list, so_filename);*/
    //.plt, .got.plt, rel.plt
    UpdatePLTRelatedSections(sec_list, dyn_sym_list);
    //.got, .rel.dyn
    UpdateGOTRelatedSections(sec_list, dyn_sym_list);
    //.dynamic
    UpdateDynamicSection(sec_list, 1);
    //.en_frame
    /*UpdateEhframeSection(sec_list);*/
    
    UpdateBSSForSymbols(sym_list, sec_list);
    
    //.shstrtab update after construct the new sec_list and before allocate address
    UpdateShstrSection(sec_list);
    
    /*showSection(sec_list);*/
    sec_list = SortSectionsByWriteOrder(sec_list);
    AllocateAddress(sec_list);
    showSection(sec_list);
    
    UpdateSymbolValue(sym_list, sec_list, merge_list);
    UpdateSymbolValue(dyn_sym_list, sec_list, merge_list);
    
    ApplyRelocations(rel_list, sec_list, merge_list, sym_list, dyn_sym_list);
    printf("test\n");
    
    Section *test;
    test = GetSectionByName(sec_list, DYNSTR_SECTION_NAME);
    showSectionData(output, test);
    fclose(output);

    showSection(sec_list);
    RenewRelGOTSection(sec_list);
    printf("test\n");
    RenewRelPLTSection(sec_list);
    printf("test\n");
    RenewPLTSection(sec_list);
    printf("test\n");
    RenewGOTPLTSection(sec_list);
    printf("test\n");
    RenewSectionInfo(sec_list);
    printf("test\n");
    RenewDynamicSection(sec_list);
    printf("test\n");
    showSection(sec_list);
    printf("test\n");
    /*RenewSymbolInfo(sym_list, dyn_sym_list, sec_list, merge_list);*/
    /* Have to change the corresponding section data */
    
    Section *symtab, *dynsymtab;
    symtab = GetSectionByName(sec_list, SYMTAB_SECTION_NAME);
    dynsymtab = GetSectionByName(sec_list, DYNSYM_SECTION_NAME);
    RenewSymbolSection(sec_list, merge_list, symtab, sym_list);
    RenewSymbolSection(sec_list, merge_list, dynsymtab, dyn_sym_list);
    
    showSection(sec_list);
    
    Section_Table *sec_tab;
    sec_tab = CreateSectionTable(sec_list);
    showSectionTable(sec_tab);

    UINT8 *phdr_data;
    phdr_data = CreateProgramHeaderTable(sec_list);
    
    Elf32_Ehdr *elf_header;
    elf_header = CreateElfHeader(obj_file->elf_file_header, sec_list, sec_tab);
    
    WriteOut(elf_header, phdr_data, sec_list, sec_tab);
    /*printf("section table offset: %d\n", sec_tab->offset);*/
    
    /*showSection(sec_list);*/
    /*Section *test1, *test2;*/
    /*Section *test;*/
    /*test = GetSectionByName(sec_list, INTERP_SECTION_NAME);*/
    /*test = GetSectionByName(sec_list, NOTE_SECTION_NAME);*/
    /*test = GetSectionByName(sec_list, HASH_SECTION_NAME);*/
    /*test = GetSectionByName(sec_list, DYNSYM_SECTION_NAME);*/
    /*test = GetSectionByName(sec_list, DYNSTR_SECTION_NAME);*/
    /*test = GetSectionByName(sec_list, GV_SECTION_NAME);*/
    /*test = GetSectionByName(sec_list, GNR_SECTION_NAME);*/
    /*test = GetSectionByName(sec_list, REL_DYN_SECTION_NAME);*/
    /*test = GetSectionByName(sec_list, REL_PLT_SECTION_NAME);*/
    /*test = GetSectionByName(sec_list, INIT_SECTION_NAME);*/
    /*test = GetSectionByName(sec_list, PLT_SECTION_NAME);*/
    /*test = GetSectionByName(sec_list, TEXT_SECTION_NAME);*/
    /*test = GetSectionByName(sec_list, FINI_SECTION_NAME);*/
    /*test = GetSectionByName(sec_list, RODATA_SECION_NAME);*/
    /*test = GetSectionByName(sec_list, EH_FRAME_SECTION_NAME);*/
    /*test = GetSectionByName(sec_list, DYNAMIC_SECTION_NAME);*/
    /*test = GetSectionByName(sec_list, GOT_SECTION_NAME);*/
    /*test = GetSectionByName(sec_list, GOT_PLT_SECTION_NAME);*/
    /*test = GetSectionByName(sec_list, JCR_SECTION_NAME);*/
    /*test = GetSectionByName(sec_list, DATA_SECTION_NAME);*/
    /*test = GetSectionByName(sec_list, FINI_ARRAY_SECTION_NAME);*/
    /*test = GetSectionByName(sec_list, INIT_ARRAY_SECTION_NAME);*/
    /*test = GetSectionByName(sec_list, COMMENT_SECTION_NAME);*/
    /*test = GetSectionByName(sec_list, SYMTAB_SECTION_NAME);*/
    /*test = GetSectionByName(sec_list, SHSTRTAB_SECTION_NAME);*/
    /*test = GetSectionByName(sec_list, STRTAB_SECTION_NAME);*/
    /*showSectionData(test);*/
    /*showSection(sec_list);*/
    /*showSection(merge_list);*/
    
    /*showSectionData(output, test);*/
    /*showSectionData(output, test2);*/
    /*fclose(output);*/
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
        printf("%d %d %s %x %x %x \n",cur_section->sec_final_number, cur_section->sec_number, cur_section->sec_name, cur_section->sec_datasize, cur_section->sec_address, cur_section->sec_file_offset);
        /*printf("%x\n", cur_section->sec_delta);*/
        /*printf("%d %s\n", cur_section->sec_number, cur_section->sec_name);*/
        cur_section = cur_section->sec_next;
        i++;
    }
}

void showSectionData(FILE *output, Section *sec)
{
    
    fwrite(sec->sec_data, sec->sec_datasize, 1, output);
    
}

void showSectionTable(Section_Table *sec_tab)
{
    int i, number;
    number = sec_tab->number;
    
    printf("id   name    type     flags    addr     offset     size    link    info    align     entsize\n");
    for (i = 0; i < number; i++) {
        Elf32_Shdr *temp;
        temp = (Elf32_Shdr *)(sec_tab->content + 40 * i);
        
        printf("%3d ", i);
        printf("%4d ", temp->sh_name);
        printf("%8x ", temp->sh_type);
        printf("%8d ", temp->sh_flags);
        printf("%8x ", temp->sh_addr);
        printf("%10x ", temp->sh_offset);
        printf("%8x ", temp->sh_size);
        printf("%8d ", temp->sh_link);
        printf("%8d ", temp->sh_info);
        printf("%8d ", temp->sh_addralign);
        printf("%8d ", temp->sh_entsize);
        printf("\n");
    }
    
    FILE *table;
    table = fopen("table", "wb");
    fwrite(sec_tab->content, sec_tab->number * 40, 1, table);
    fclose(table);
}

char *PrepareStr(char *str)
{
    char *res;
    res = malloc(100);
    
    char *temp;
    int i, index ;
    i = 0;
    temp = str;
    while (*temp) {
        if (*temp == '/')
            index = i;
        temp++;
        i++;
    }

    temp = str + index + 1;
    i = 0;
    while (*temp) {
        res[i++] = *temp;
        temp++;
    }
    
    res[i] = '\0';
    
    return res;
}
