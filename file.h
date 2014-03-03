/*
 * =====================================================================================
 *
 *       Filename:  file.h
 *
 *    Description:  the declaration  of basic data structure for obj file
 *
 *        Version:  1.0
 *        Created:  03/02/2014 10:10:29 AM
 *       Revision:  none
 *       Compiler:  gcc
 *
 *         Author:  YOUR NAME (), 
 *   Organization:  
 *
 * =====================================================================================
 */

#ifndef FILE_H
#define FILE_H

#include <elf.h>
#include "type.h"

typedef enum {
    BINARY_CORE_TYPE,
    BINARY_SHARED_TYPE,
    BINARY_RELOCATABLE_TYPE,
    BINARY_EXECUTABLE_TYPE
} BINARY_TYPE;

// Defined for the elf file
typedef struct Elf32_File {
    Elf32_Ehdr *elf_file_header;
    
    Elf32_Shdr *elf_section_table;
    UINT8 *elf_section_strn_table;

    Elf32_Shdr *elf_sym_table_dr;
    UINT8 *elf_sym_strn_table;
    Elf32_Sym *elf_sym_table;
    
    Elf32_Shdr *elf_dyn_sym_table_dr;
    UINT8 *elf_dyn_sym_strn_table;
    Elf32_Sym *elf_dyn_sym_table;
    
    UINT8 *elf_file_data;
    UINT32 elf_file_size;
    BINARY_TYPE elf_file_type;
} Elf32_File;

#endif
