/*
 * =====================================================================================
 *
 *       Filename:  symbol.c
 *
 *    Description:  
 *
 *        Version:  1.0
 *        Created:  03/04/2014 10:07:02 AM
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

#include "symbol.h"

static void showSymbolInfo(Symbol *symbol);
static Symbol *CopySymbolData(Symbol *);
static void MarkDynSymbol(Symbol *, Symbol *);

Symbol *GetSymbols(Elf32_File *elf_file, Section *sec_list)
{
    Symbol *cur_symbol, *last_symbol, *first_symbol;
    Elf32_Sym *sym_table;
    Elf32_Shdr *sym_table_dr;
    UINT8 *sym_strn_table;
    Elf32_Sym *dyn_sym_table;
    Elf32_Shdr *dyn_sym_table_dr;
    UINT8 *dyn_sym_strn_table;
    
    UINT16 cur_version_num;
    
    int i, numberOfSymbols;
    Section *gnu_version;
    Vd_item *vd_list;
    vd_list = NULL;
    cur_symbol = last_symbol = first_symbol = NULL;
    
    if (elf_file->elf_file_type == BINARY_RELOCATABLE_TYPE) {
        sym_table = elf_file->elf_sym_table;
        sym_strn_table = elf_file->elf_sym_strn_table;
        sym_table_dr = elf_file->elf_sym_table_dr;
        
        numberOfSymbols = sym_table_dr->sh_size / sym_table_dr->sh_entsize;
    }
    
    if (elf_file->elf_file_type == BINARY_SHARED_TYPE) {
        dyn_sym_table = elf_file->elf_dyn_sym_table;
        dyn_sym_strn_table = elf_file->elf_dyn_sym_strn_table;
        dyn_sym_table_dr = elf_file->elf_dyn_sym_table_dr;
        
        numberOfSymbols = dyn_sym_table_dr->sh_size / dyn_sym_table_dr->sh_entsize;
    }
    
    /*printf("%d \n", numberOfSymbols);*/
    for (i = 0; i < numberOfSymbols; i++) {
        int sym_name_length;
        cur_symbol = (Symbol *)malloc(sizeof(Symbol));
        
        if (elf_file->elf_file_type == BINARY_RELOCATABLE_TYPE) {
            cur_symbol->sym_content = (Elf32_Sym *)malloc(sizeof(Elf32_Sym));
            memcpy(cur_symbol->sym_content, sym_table + i, sym_table_dr->sh_entsize);
            sym_name_length = strlen(sym_strn_table + cur_symbol->sym_content->st_name);
            cur_symbol->sym_name = (UINT8 *)malloc(sym_name_length + 1);
            strcpy(cur_symbol->sym_name, sym_strn_table + cur_symbol->sym_content->st_name);
            
            cur_symbol->sym_sd_type = SYM_LOCAL;
            cur_symbol->sym_version = 0;
            cur_symbol->sym_version_name = NULL;
        }
        
        if (elf_file->elf_file_type == BINARY_SHARED_TYPE) {
            vd_list = GetSymbolVersions(sec_list);
            gnu_version = GetSectionByName(sec_list, GV_SECTION_NAME);
            
            cur_symbol->sym_content = (Elf32_Sym *)malloc(sizeof(Elf32_Sym));
            memcpy(cur_symbol->sym_content, dyn_sym_table + i, dyn_sym_table_dr->sh_entsize);
            sym_name_length = strlen(dyn_sym_strn_table + cur_symbol->sym_content->st_name);
            cur_symbol->sym_name = (UINT8 *)malloc(sym_name_length + 1);
            strcpy(cur_symbol->sym_name, dyn_sym_strn_table + cur_symbol->sym_content->st_name);
            
            cur_symbol->sym_version = GetVersionNumber(gnu_version, i);
            UINT8 *version_name;
            version_name = GetVersionName(vd_list, cur_symbol->sym_version);
            if (version_name) {
                cur_symbol->sym_version_name = (UINT8 *)malloc(strlen(version_name) + 1);
                strcpy(cur_symbol->sym_version_name, version_name);
            }
        }

        if (first_symbol == NULL) {
            first_symbol = cur_symbol;
            last_symbol = cur_symbol;
        }
        else {
            InsertSymbolAfterSymbol(cur_symbol, last_symbol);
            last_symbol = cur_symbol;
        }
        
        /*printf("%d %s\n", i, cur_symbol->sym_name);*/
    }
    
    if (vd_list != NULL)
        FreeVersionsData(vd_list);
    
    /*showSymbolInfo(first_symbol);*/
    return first_symbol;
}

void InsertSymbolAfterSymbol(Symbol *new_symbol, Symbol *symbol)
{
    new_symbol->sym_prev = symbol;
    new_symbol->sym_next = symbol->sym_next;
    symbol->sym_next = new_symbol;
    
    if (new_symbol->sym_next != NULL)
        new_symbol->sym_next->sym_prev = new_symbol;
}

// sym_target: symbols in object file
// sym_source: symbols in .so file
Symbol *MakeDynSymbol(Symbol *sym_target, Symbol *sym_source)
{
    Symbol *cur_symbol, *last_symbol, *first_symbol;
    cur_symbol = last_symbol = first_symbol = NULL;
    
    MarkDynSymbol(sym_target, sym_source);
    
    while (sym_target) {
    /*printf("test\n");*/
        if (sym_target->sym_sd_type == SYM_LOCAL) {
            sym_target = sym_target->sym_next;
            continue;
        }
        
        cur_symbol = CopySymbolData(sym_target);
        
        if (first_symbol == NULL) {
            first_symbol = cur_symbol;
            last_symbol = cur_symbol;
        }
        else {
            InsertSymbolAfterSymbol(cur_symbol, last_symbol);
            last_symbol = cur_symbol;
        }
        sym_target = sym_target->sym_next;
    }
    
    showSymbolInfo(first_symbol);
    return first_symbol;
}

static void MarkDynSymbol(Symbol *sym_target, Symbol *sym_source)
{
    Symbol *cur_target, *cur_source, *first_target;
    first_target = cur_target = sym_target;
    cur_source = sym_source;
    /*Symbol *cur_symbol, *last_symbol, *first_symbol;*/
    /*cur_symbol = last_symbol = first_symbol = NULL;*/
    
    while (cur_source != NULL) {
        while (cur_target != NULL) {
            if (!strcmp(cur_source->sym_name, cur_target->sym_name) && (cur_target->sym_sd_type == SYM_LOCAL)) {
                
                if ((cur_source->sym_content->st_info & 0xf) == STT_FUNC) {
                    cur_target->sym_sd_type = SYM_PLT;
                    cur_target->sym_content->st_info = cur_target->sym_content->st_info & 0xfffffff0 + STT_FUNC;
                    cur_target->sym_version = cur_source->sym_version;
                    cur_target->sym_version_name = (UINT8 *)malloc(strlen(cur_source->sym_version_name) + 1);
                    strcpy(cur_target->sym_version_name, cur_source->sym_version_name);
                }
                else {
                    cur_target->sym_sd_type = SYM_GOT;
                    cur_target->sym_version = cur_source->sym_version;
                    // UNFIXED: Maybe a Bug here
                    if (cur_target->sym_version != 0) {
                        cur_target->sym_version_name = (UINT8 *)malloc(strlen(cur_source->sym_version_name) + 1);
                        strcpy(cur_target->sym_version_name, cur_source->sym_version_name);
                    }
                }
                
                /*cur_symbol = CopySymbolData(cur_target);*/
                
                /*if (first_symbol == NULL) {*/
                    /*first_symbol = cur_symbol;*/
                    /*last_symbol = cur_symbol;*/
                /*}*/
                /*else {*/
                    /*InsertSymbolAfterSymbol(cur_symbol, last_symbol);*/
                    /*last_symbol = cur_symbol;*/
                /*}*/
                break;
            }
            cur_target = cur_target->sym_next;
        }
        cur_source = cur_source->sym_next;
        cur_target = first_target;
    }
    
    /*showSymbolInfo(first_target);*/
    /*printf("test\n");*/
    /*showSymbolInfo(first_symbol);*/
    /*printf("test\n");*/
}

static Symbol *CopySymbolData(Symbol *sym)
{
    Symbol *symbol_shadow;
    
    symbol_shadow = (Symbol *)malloc(sizeof(Symbol));
    symbol_shadow->sym_prev = symbol_shadow->sym_next = NULL;
    symbol_shadow->sym_name = (UINT8 *)malloc(strlen(sym->sym_name) + 1);
    strcpy(symbol_shadow->sym_name, sym->sym_name);
    symbol_shadow->sym_content = (Elf32_Sym *)malloc(sizeof(Elf32_Sym));
    memcpy(symbol_shadow->sym_content, sym->sym_content, sizeof(Elf32_Sym));
    symbol_shadow->sym_sd_type = sym->sym_sd_type;
    symbol_shadow->sym_version = sym->sym_version;
    // UNFIXED: Maybe a Bug here
    if (symbol_shadow->sym_version != 0) {
        symbol_shadow->sym_version_name = (UINT8 *)malloc(strlen(sym->sym_version_name) + 1);
        strcpy(symbol_shadow->sym_version_name, sym->sym_version_name);
    }
    
    return symbol_shadow;
}

static void showSymbolInfo(Symbol *symbol)
{
    Symbol *cur_symbol;
    
    /*printf("Num:     Value    Size    Type   Bind   Ndx   Name\n");*/
    cur_symbol = symbol;
    int i = 0;
    while (cur_symbol != NULL) {
        /*printf("%d ", i);*/
        /*printf("%8x: ", cur_symbol->sym_value);*/
        /*printf("%5d: ", cur_symbol->sym_size);*/
        /*printf("%s ", cur_symbol->sym_name);*/
        /*printf("%d\n", cur_symbol->sym_sd_type);*/
        /*printf("%d ", cur_symbol->sym_version);*/
        /*if (cur_symbol->sym_version_name == NULL)*/
            /*continue;*/
        /*printf("%s\n", cur_symbol->sym_version_name);*/
        /*printf("\n");*/
        /*printf("%d: ", cur_symbol->sym_binding);*/
        /*printf("%d  ", cur_symbol->sym_shndx);*/
        if (cur_symbol->sym_sd_type == SYM_GOT) {
            printf("%d GOT  %s", i, cur_symbol->sym_name);
            printf("\n");
        }
        if (cur_symbol->sym_sd_type == SYM_PLT) {
            printf("%d PLT %s %d %s", i, cur_symbol->sym_name, cur_symbol->sym_version, cur_symbol->sym_version_name);
            printf("\n");
        }
        
        cur_symbol = cur_symbol->sym_next;
        i++;
    }
}
