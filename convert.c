/*
 * =====================================================================================
 *
 *       Filename:  convert.c
 *
 *    Description:  basic steps to get binary file data
 *
 *        Version:  1.0
 *        Created:  03/02/2014 10:24:14 AM
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
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include "file.h"
#include "convert.h"

// in: elf file name
// out: the data structure representing a file
Elf32_File *GetBinaryFileData(char *filename)
{
    FILE *file;
    file = fopen(filename, "rb");
    if (file == NULL) {
        printf("error opening file\n");
        printf("%s\n", filename);
        exit(EXIT_FAILURE);
    }
    
    struct stat fs;
    if (stat(filename, &fs) == -1) {
        printf("error reading file meta data\n");
        exit(EXIT_FAILURE);
    }
    
    Elf32_File *elf_file;
    UINT8 *file_data;
    UINT32 file_size;
    
    file_size = fs.st_size;
    
    file_data = (UINT8 *)malloc(file_size);
    if (fread(file_data, 1, file_size, file) != file_size) {
        printf("error reading file data\n");
        exit(EXIT_FAILURE);
    }
    fclose(file);
    
    elf_file = (Elf32_File *)malloc(sizeof(Elf32_File));
    elf_file->elf_file_size = file_size;
    elf_file->elf_file_data = file_data;
    
    FillElfData(elf_file, file_data);
    
    return elf_file;
}

void FillElfData(Elf32_File *elf_file, UINT8 *file_data)
{
    elf_file->elf_file_header = (Elf32_Ehdr *)file_data;
    
    switch (elf_file->elf_file_header->e_type) {
        case ET_REL:
            elf_file->elf_file_type = BINARY_RELOCATABLE_TYPE;
            break;
        case ET_EXEC:
            elf_file->elf_file_type = BINARY_EXECUTABLE_TYPE;
            break;
        case ET_CORE:
            elf_file->elf_file_type = BINARY_CORE_TYPE;
            break;
        case ET_DYN:
            elf_file->elf_file_type = BINARY_SHARED_TYPE;
            break;
        default:
            printf("error The file type we can't handle\n");
            exit(EXIT_FAILURE);
    }
    
    int numberOfSections, i;
    INT16 shstrndx;
    
    numberOfSections = elf_file->elf_file_header->e_shnum;
    elf_file->elf_section_table = (Elf32_Shdr *)(file_data + elf_file->elf_file_header->e_shoff);
    shstrndx = elf_file->elf_file_header->e_shstrndx;
    elf_file->elf_section_strn_table = file_data + (elf_file->elf_section_table + shstrndx)->sh_offset;
    
    for (i = 0; i < numberOfSections; i++) {
        Elf32_Shdr *cur_section;
        cur_section = elf_file->elf_section_table + i;
        
        if (cur_section->sh_type == SHT_SYMTAB) {
            elf_file->elf_sym_table_dr = cur_section;
            elf_file->elf_sym_table = (Elf32_Sym *)(file_data + cur_section->sh_offset);
            elf_file->elf_sym_strn_table 
                = file_data + (elf_file->elf_section_table + elf_file->elf_sym_table_dr->sh_link)->sh_offset;
        }
        
        if (cur_section->sh_type == SHT_DYNSYM) {
            elf_file->elf_dyn_sym_table_dr = cur_section;
            elf_file->elf_dyn_sym_table = (Elf32_Sym *)(file_data + cur_section->sh_offset);
            elf_file->elf_dyn_sym_strn_table
                = file_data + (elf_file->elf_section_table + elf_file->elf_dyn_sym_table_dr->sh_link)->sh_offset;
        }
    }
}
