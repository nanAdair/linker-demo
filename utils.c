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
#include "utilsGD.h"

static void RoundUpSection(Section *, int);
static void UpdateProgPHDR(Elf32_Phdr *phdr);
static void UpdateProgINTERP(Elf32_Phdr *phdr, Section *sec_list);
static void UpdateProgLOAD(Elf32_Phdr *phdr, Section *sec_list);
static void UpdateProgDYNAMIC(Elf32_Phdr *phdr, Section *sec_list);
static void UpdateProgNOTE(Elf32_Phdr *phdr, Section *sec_list);
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
    
    Section *cur_sec, *first_sec, *init, *res_list, *next_sec;
    cur_sec = first_sec = sec_list;
    init = GetSectionByName(sec_list, INIT_SECTION_NAME);
    
    while (cur_sec) {
        /*printf("%s %d\n", cur_sec->sec_name, cur_sec->sec_misc);*/
        if ((cur_sec->sec_misc != 0) && (cur_sec->sec_type == SHT_REL)) {
            next_sec = cur_sec->sec_next;
            DropSection(cur_sec);
            cur_sec = next_sec;
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
        /*if ((cur_sec->sec_misc != 0) && cur_sec->sec_type == SHT_REL) {*/
            /*cur_sec = cur_sec->sec_next;*/
            /*continue;*/
        /*}*/
        
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
        /*list[i]->sec_final_number = i;*/
    }
    
    return res_list;
}

void AllocateAddress(Section *sec_list)
{
    UINT32 base_addr, offset;
    base_addr = 0x8048000;
    offset = 0x114;
    UINT32 addend = 0;
    
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
        
        // as for loading operation, there have to be one more page
        if (cur_sec->sec_flags & SHF_WRITE)
            addend = 0x1000;
        if (cur_sec->sec_flags & SHF_ALLOC)
            cur_sec->sec_address = base_addr + offset + addend;
        else
            cur_sec->sec_address = 0;
        cur_sec->sec_file_offset = offset;
        
        if (strcmp(cur_sec->sec_name, BSS_SECTION_NAME))
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
    /* TODO: TOBE FIXED, the datasize won't be changed
     * instead, write out exact number of 0x0 when write a file
     * The error occured when use datasize as an parameter*/
    /*sec->sec_datasize = newdatasize;*/
}

Section_Table *CreateSectionTable(Section *sec_list)
{
    Section_Table *sec_tab;
    sec_tab = (Section_Table *)malloc(sizeof(Section_Table));
    
    Elf32_Shdr *cur_shdr;
    UINT8 *buffer;
    UINT32 buffer_size, addition, offset;
    buffer = NULL;
    buffer_size = 0;
    addition = sizeof(Elf32_Shdr);
    
    Section *cur_sec, *last_sec;
    cur_sec = sec_list;
    int i = 0;
    
    while (cur_sec) {
        cur_shdr = (Elf32_Shdr *)malloc(addition);
        
        cur_shdr->sh_name = cur_sec->sec_name_offset;
        cur_shdr->sh_type = cur_sec->sec_type;
        cur_shdr->sh_addr = cur_sec->sec_address;
        cur_shdr->sh_flags = cur_sec->sec_flags;
        cur_shdr->sh_offset = cur_sec->sec_file_offset;
        cur_shdr->sh_size = cur_sec->sec_datasize;
        cur_shdr->sh_link = cur_sec->sec_link;
        cur_shdr->sh_info = cur_sec->sec_info;
        cur_shdr->sh_addralign = cur_sec->sec_align;
        cur_shdr->sh_entsize = cur_sec->sec_entsize;
        
        if (buffer == NULL) {
            buffer = (UINT8 *)malloc(addition);
            memset(buffer, 0x0, addition);
            memcpy(buffer, cur_shdr, addition);
            buffer_size = addition;
            free(cur_shdr);
        }
        else {
            UINT8 *newbuffer;
            newbuffer = (UINT8 *)malloc(buffer_size + addition);
            memset(newbuffer, 0x0, buffer_size + addition);
            memcpy(newbuffer, buffer, buffer_size);
            memcpy(newbuffer + buffer_size, cur_shdr, addition);
            
            free(buffer);
            buffer = newbuffer;
            buffer_size += addition;
            free(cur_shdr);
        }
        
        if (!strcmp(cur_sec->sec_name, SHSTRTAB_SECTION_NAME))
            sec_tab->str_index = cur_sec->sec_final_number;

        i++;
        last_sec = cur_sec;
        cur_sec = cur_sec->sec_next;
    }
    
    offset = last_sec->sec_file_offset + last_sec->sec_datasize;
    
    sec_tab->content = buffer;
    sec_tab->offset = offset;
    sec_tab->number = i;
    
    return sec_tab;
}

UINT8 *CreateProgramHeaderTable(Section *sec_list)
{
    // hard-coded here
    int entsize = 0x20;
    
    UINT8 *buffer;
    UINT32 buffer_size;
    
    buffer = NULL;
    buffer_size = 0;
    
    int i, number, addition;
    number = sizeof(Program_Headers) / entsize;
    addition = sizeof(Elf32_Phdr);
    
    for (i = 0; i < number; i++) {
        Elf32_Phdr program_header;
        program_header = Program_Headers[i];
        UINT32 type;
        type = program_header.p_type;
        
        switch (type) {
            case PT_PHDR:
                UpdateProgPHDR(&program_header);
                break;
            case PT_INTERP:
                UpdateProgINTERP(&program_header, sec_list);
                break;
            case PT_LOAD:
                UpdateProgLOAD(&program_header, sec_list);
                break;
            case PT_DYNAMIC:
                UpdateProgDYNAMIC(&program_header, sec_list);
                break;
            case PT_NOTE:
                UpdateProgNOTE(&program_header, sec_list);
                break;
            case PT_GNU_STACK:
                /*UpdateProgGNU_STACK(&program_header, sec_list);*/
                break;
            default:
                printf("error in handling the phdr type %d\n", type);
                break;
        }
        
        if (buffer == NULL) {
            buffer = malloc(addition);
            memset(buffer, 0x0, addition);
            memcpy(buffer, &program_header, addition);
            buffer_size = addition;
        }
        else {
            UINT8 *newbuffer;
            newbuffer = malloc(buffer_size + addition);
            memset(newbuffer, 0x0, addition);
            memcpy(newbuffer, buffer, buffer_size);
            memcpy(newbuffer + buffer_size, &program_header, addition);
            
            free(buffer);
            buffer = newbuffer;
            buffer_size += addition;
        }
    }
    
    return buffer;
}

static void UpdateProgPHDR(Elf32_Phdr *phdr)
{
    UINT32 offset, base_addr;
    offset = sizeof(Elf32_Ehdr);
    base_addr = 0x8048000;
    
    phdr->p_offset = offset;
    phdr->p_vaddr = phdr->p_paddr = base_addr + offset;
    phdr->p_memsz = phdr->p_filesz = sizeof(Program_Headers);
}

static void UpdateProgINTERP(Elf32_Phdr *phdr, Section *sec_list)
{
    Section *interp;
    interp = GetSectionByName(sec_list, INTERP_SECTION_NAME);
    
    UINT32 offset, base_addr;
    offset = interp->sec_file_offset;
    base_addr = 0x8048000;
    
    phdr->p_offset = offset;
    phdr->p_vaddr = phdr->p_paddr = base_addr + offset;
    phdr->p_memsz = phdr->p_filesz = interp->sec_datasize;
}

static void UpdateProgLOAD(Elf32_Phdr *phdr, Section *sec_list)
{
    UINT32 offset, base_addr, filesize, memsize;
    
    if (phdr->p_flags & PF_X) {
        offset = 0;
        base_addr = 0x8048000;
        UINT32 dataBegin, dataEnd;
        dataBegin = 0;
        
        Section *cur_sec;
        cur_sec = sec_list;
        while (cur_sec) {
            if (cur_sec->sec_flags & SHF_WRITE)
                break;
            
            cur_sec = cur_sec->sec_next;
        }
        dataEnd = cur_sec->sec_file_offset;
        
        filesize = dataEnd - dataBegin;
        memsize = filesize;
    }
    
    if (phdr->p_flags & PF_W) {
        UINT32 dataBegin, dataEnd;
        Section *cur_sec;
        cur_sec = sec_list;
        while (cur_sec) {
            if (cur_sec->sec_flags & SHF_WRITE)
                break;
            
            cur_sec = cur_sec->sec_next;
        }
        offset = cur_sec->sec_file_offset;
        base_addr = cur_sec->sec_address;
        dataBegin = cur_sec->sec_file_offset;
        
        while (cur_sec) {
            if (!(cur_sec->sec_next->sec_flags & SHF_WRITE))
                break;
            cur_sec = cur_sec->sec_next;
        }
        dataEnd = cur_sec->sec_file_offset;
        filesize = dataEnd - dataBegin;
        memsize = filesize + cur_sec->sec_datasize;
    }
    
    phdr->p_offset = offset;
    phdr->p_vaddr = phdr->p_paddr = base_addr;
    phdr->p_filesz = filesize;
    phdr->p_memsz = memsize;
}

static void UpdateProgDYNAMIC(Elf32_Phdr *phdr, Section *sec_list)
{
    Section *dynamic;
    dynamic = GetSectionByName(sec_list, DYNAMIC_SECTION_NAME);
    
    phdr->p_offset = dynamic->sec_file_offset;
    phdr->p_vaddr = phdr->p_paddr = dynamic->sec_address;
    phdr->p_filesz = phdr->p_memsz = dynamic->sec_datasize;
}

static void UpdateProgNOTE(Elf32_Phdr *phdr, Section *sec_list)
{
    Section *note;
    note = GetSectionByName(sec_list, NOTE_SECTION_NAME);
    
    phdr->p_offset = note->sec_file_offset;
    phdr->p_vaddr = phdr->p_paddr = note->sec_address;
    phdr->p_filesz = phdr->p_memsz = note->sec_datasize;
}

/*static void UpdateProgGNU_STACK()*/

Elf32_Ehdr *CreateElfHeader(Elf32_Ehdr *ehdr, Section *sec_list, Section_Table *sec_tab)
{
    Elf32_Ehdr *elf_file_header;
    elf_file_header = (Elf32_Ehdr *)malloc(sizeof(Elf32_Ehdr));
    memcpy(elf_file_header, ehdr, sizeof(Elf32_Ehdr));
    
    Section *text;
    text = GetSectionByName(sec_list, TEXT_SECTION_NAME);
    
    elf_file_header->e_type = ET_EXEC;
    elf_file_header->e_entry = text->sec_address;
    elf_file_header->e_phoff = sizeof(Elf32_Ehdr);
    elf_file_header->e_shoff = sec_tab->offset;
    elf_file_header->e_phentsize = 0x20;
    elf_file_header->e_phnum = 7;
    elf_file_header->e_shnum = sec_tab->number;
    elf_file_header->e_shstrndx = sec_tab->str_index;
    
    return elf_file_header;
}

void WriteOut(Elf32_Ehdr *ehdr_data, UINT8 *phdr_data, Section *sec_list, Section_Table *sec_tab)
{
    FILE *output;
    output = fopen("output", "wb");
    UINT32 phdr_size, sec_tab_size;
    phdr_size = ehdr_data->e_phentsize * ehdr_data->e_phnum;
    sec_tab_size = ehdr_data->e_shentsize * ehdr_data->e_shnum;
    
    fwrite(ehdr_data, sizeof(Elf32_Ehdr), 1, output);
    fwrite(phdr_data, phdr_size, 1, output);
    
    Section *cur_sec;
    cur_sec = sec_list;
    UINT32 i, offset;
    UINT8 zero = 0x0;
    
    offset = cur_sec->sec_file_offset;
    i = 0;
    while (cur_sec) {
        if (cur_sec->sec_file_offset != offset) {
            for (i = 0; i < cur_sec->sec_file_offset - offset; i++)
                fwrite(&zero, 1, 1, output);
            offset = cur_sec->sec_file_offset;
        }
        
        if (cur_sec->sec_type == SHT_NOBITS) {
            cur_sec = cur_sec->sec_next;
            continue;
        }
        fwrite(cur_sec->sec_data, cur_sec->sec_datasize, 1, output);
        offset += cur_sec->sec_datasize;
        
        cur_sec = cur_sec->sec_next;
    }
    
    fwrite(sec_tab->content, sec_tab_size, 1, output);
    
    fclose(output);
}
