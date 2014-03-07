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
#include <elf.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include "section.h"
#include "sectionGD.h"

static void FillInterpSection(Section *, char *);
static void FillDynstrSection(Section *, Symbol *, char *);
static void FillDynsymSection(Section *, Symbol *);
static void FillGVSection(Section *, Symbol *);
static void FillHashSection(Section *, Symbol *);
static void FillGNRSection(Section *, Symbol *, Section *, char *);
static void AddDynstrEntryFromName(Section *, Symbol *);
static void AddDynstrEntryFromVersion(Section *, Symbol *);
static void AddDynsymEntry(Section *, Symbol *);
static void AddGVEntry(Section *, Symbol *);
static void AddGNREntry(Section *, Symbol *, Section *, int);
static void AddGOTPLTTop(Section *);
static void AddPLTTop(Section *);
static void AddPLTEntry(Section *, int);
static void AddGOTorGOTPLTEntry(Section *);
static void AddRelPLTEntry(Section *, Symbol *, int);
static void AddRelGOTEntry(Section *, Symbol *, int);
static UINT32 CalculateHash(const UINT8 *);
static int FindOffset(Section *, char *);

static void DeleteSection(Section *);
static Section *MergeTwoSections(Section *, Section *);
static int SkipXSection(UINT8 *name);
static Section *CopySection(Section *);

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
        cur_section->sec_misc = i;
        
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
        new_section->sec_misc = 0;
        new_section->sec_data = NULL;
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

void UpdateDynstrSection(Section *sec_list, Symbol *dynsym_list, char *so_filename)
{
    Section *dynstr_sec;
    dynstr_sec = GetSectionByName(sec_list, DYNSTR_SECTION_NAME);
    
    FillDynstrSection(dynstr_sec, dynsym_list, so_filename);
}

static void FillDynstrSection(Section *dynstr_sec, Symbol *dynsym_list, char *so_filename)
{
    Symbol *cur_sym, *first_sym;
    UINT8 *sec_data;
    int datasize;
    
    sec_data = (UINT8 *)malloc(strlen(so_filename) + 1);
    datasize = strlen(so_filename) + 1;
    strcpy(sec_data, so_filename);
    
    dynstr_sec->sec_data = sec_data;
    dynstr_sec->sec_datasize = datasize;
    
    first_sym = cur_sym = dynsym_list;
    while (cur_sym) {
        AddDynstrEntryFromName(dynstr_sec, cur_sym);
        cur_sym = cur_sym->sym_next;
    }
    
    cur_sym = first_sym;
    // UNFIXED: How to tell a version has ever occured
    // number 0, 1 do not have a version name
    int flags[100] = {1, 1, 0};
    while (cur_sym) {
        int version;
        version = cur_sym->sym_version;
        if (!flags[version] && version != 0) {
            flags[version] = 1;
            AddDynstrEntryFromVersion(dynstr_sec, cur_sym);
        }
        cur_sym = cur_sym->sym_next;
    }
}

static void AddDynstrEntryFromName(Section *dynstr_sec, Symbol *sym)
{
    int datasize, newdatasize;
    datasize = dynstr_sec->sec_datasize;
    int length;
    length = strlen(sym->sym_name) + 1;
    newdatasize = datasize + length;
    
    UINT8 *buffer;
    buffer = (UINT8 *)malloc(newdatasize);
    memcpy(buffer, dynstr_sec->sec_data, datasize);
    strcpy(buffer + datasize, sym->sym_name);
    
    free(dynstr_sec->sec_data);
    dynstr_sec->sec_data = buffer;
    dynstr_sec->sec_datasize = newdatasize;
    
    sym->sym_content->st_name = datasize;
}

static void AddDynstrEntryFromVersion(Section *dynstr_sec, Symbol *sym)
{
    int datasize, newdatasize;
    datasize = dynstr_sec->sec_datasize;
    int length;
    length = strlen(sym->sym_version_name) + 1;
    newdatasize = datasize + length;
    
    UINT8 *buffer;
    buffer = (UINT8 *)malloc(newdatasize);
    memset(buffer, 0, newdatasize);
    memcpy(buffer, dynstr_sec->sec_data, datasize);
    strcpy(buffer + datasize, sym->sym_version_name);
    
    free(dynstr_sec->sec_data);
    dynstr_sec->sec_data = buffer;
    dynstr_sec->sec_datasize = newdatasize;
}

void UpdateDynsymSection(Section *sec_list, Symbol *sym_list)
{
    Section *dynsym;
    dynsym = GetSectionByName(sec_list, DYNSYM_SECTION_NAME);
    
    FillDynsymSection(dynsym, sym_list);
}

// UNFIXED: dynsym.sh_link 
// wait until all the layout settled
static void FillDynsymSection(Section *dynsym, Symbol *sym_list)
{
    Symbol *cur_sym;
    cur_sym = sym_list;
    
    while (cur_sym) {
        AddDynsymEntry(dynsym, cur_sym);
        cur_sym = cur_sym->sym_next;
    }
}

static void AddDynsymEntry(Section *dynsym, Symbol *sym)
{
    int datasize, newdatasize;
    datasize = dynsym->sec_datasize;
    int addition;
    addition = dynsym->sec_entsize;
    newdatasize = datasize + addition;
    
    UINT8 *buffer;
    buffer = (UINT8 *)malloc(newdatasize);
    memset(buffer, 0, newdatasize);
    memcpy(buffer, dynsym->sec_data, datasize);
    memcpy(buffer + datasize, sym->sym_content, addition);
    
    free(dynsym->sec_data);
    dynsym->sec_data = buffer;
    dynsym->sec_datasize = newdatasize;
}

/* TODO: Can't understand the algorithm here now' */
void UpdateHashSection(Section *sec_list, Symbol *sym_list, Section *hash)
{
    Section *hash_sec;
    hash_sec = GetSectionByName(sec_list, HASH_SECTION_NAME);
    
    /*FillHashSection(hash_sec, sym_list);*/
    int datasize = hash->sec_datasize;
    hash_sec->sec_data = (UINT8 *)malloc(datasize);
    memset(hash_sec->sec_data, 0x0, datasize);
    memcpy(hash_sec->sec_data, hash->sec_data, datasize);
    hash_sec->sec_datasize = datasize;
}

/* TODO: Replace the content using a file temporaly */
static void FillHashSection(Section *hash_sec, Symbol *sym_list)
{
    Symbol *cur_sym;
    cur_sym = sym_list;
    
    while (cur_sym) {
        printf("%d\n", CalculateHash(cur_sym->sym_name));
        cur_sym = cur_sym->sym_next;
    }
    
    /*FILE *file;*/
    /*file = fopen("hash", "rb");*/
    /*if (file == NULL) {*/
        /*printf("error opening hash file\n");*/
        /*exit(EXIT_FAILURE);*/
    /*}*/
    
    /*struct stat fs;*/
    /*if (stat("hash", &fs) == -1) {*/
        /*printf("error reading hash file meta data\n");*/
        /*exit(EXIT_FAILURE);*/
    /*}*/
    
    /*UINT32 datasize;*/
    /*datasize = fs.st_size;*/
    
    /*hash_sec->sec_data = (UINT8 *)malloc(datasize);*/
    /*hash_sec->sec_datasize = datasize;*/
    /*if (fread(hash_sec->sec_data, datasize, 1, file) != datasize) {*/
        /*printf("error in reading hash file data\n");*/
        /*exit(EXIT_FAILURE);*/
    /*}*/
    /*fclose(file);*/
}
    
static UINT32 CalculateHash(const UINT8 *name)
{
    UINT32 h = 0, g;
    
    while (*name) {
        h = (h << 4) + *name++;
        if (g = h & 0xf0000000)
            h ^= g >> 24;
        h &= ~g;
    }
    return h;
}

void UpdateGVSection(Section *sec_list, Symbol *sym_list)
{
    Section *gv_sec;
    gv_sec = GetSectionByName(sec_list, GV_SECTION_NAME);
    
    FillGVSection(gv_sec, sym_list);
}

static void FillGVSection(Section *gv_sec, Symbol *sym_list)
{
    Symbol *cur_sym;
    cur_sym = sym_list;
    
    while (cur_sym) {
        AddGVEntry(gv_sec, cur_sym);
        cur_sym = cur_sym->sym_next;
    }
}

static void AddGVEntry(Section *gv_sec, Symbol *sym)
{
    int datasize, newdatasize;
    datasize = gv_sec->sec_datasize;
    UINT16 addition;
    addition = gv_sec->sec_entsize;
    newdatasize = datasize + addition;
    
    UINT8 *buffer;
    buffer = (UINT8 *)malloc(newdatasize);
    memset(buffer, 0, newdatasize);
    memcpy(buffer, gv_sec->sec_data, datasize);
    buffer[datasize] = sym->sym_version;
    
    free(gv_sec->sec_data);
    gv_sec->sec_data = buffer;
    gv_sec->sec_datasize = newdatasize;
}

void UpdateGNRSection(Section *sec_list, Symbol *sym_list, char *so_filename)
{
    Section *gnr_sec, *dynstr;
    gnr_sec = GetSectionByName(sec_list, GNR_SECTION_NAME);
    dynstr = GetSectionByName(sec_list, DYNSTR_SECTION_NAME);
    
    /*int offset;*/
    /*offset = FindOffset(dynstr, so_filename);*/
    /*printf("\n%d\n", offset);*/
    /*printf("\n%d\n", FindOffset(dynstr, "printf"));*/
    FillGNRSection(gnr_sec, sym_list, dynstr, so_filename);

}
/* TODO: UNFIXED: more than two .so files have to be taken cared */
static void FillGNRSection(Section *gnr_sec, Symbol *sym_list, Section *dynstr, char *so_filename)
{
    Symbol *cur_sym;
    cur_sym = sym_list;
    Elf32_Verneed verneed;
    int i, version_number, last, datasize;
    
    int flags[100] = {1, 1, 0};
    version_number = 0;
    
    while (cur_sym) {
        if (flags[cur_sym->sym_version] == 0) {
            version_number++;
            flags[cur_sym->sym_version] = 1;
        }
        cur_sym = cur_sym->sym_next;
    }
    
    verneed.vn_version = 1;
    verneed.vn_cnt = version_number;
    verneed.vn_file = FindOffset(dynstr, so_filename);
    verneed.vn_aux = 0x10;
    verneed.vn_next = 0;
    
    datasize = sizeof(Elf32_Verneed);
    gnr_sec->sec_data = malloc(datasize);
    memset(gnr_sec->sec_data, 0, datasize);
    memcpy(gnr_sec->sec_data, &verneed, datasize);
    gnr_sec->sec_datasize = datasize;
    
    for (i = 2; i < 100; i++)
        flags[i] = 0;
    
    cur_sym = sym_list;
    last = 0;
    while (cur_sym) {
        if (flags[cur_sym->sym_version] == 0) {
            flags[cur_sym->sym_version] = 1;
            last++;
            if (last == verneed.vn_cnt)
                AddGNREntry(gnr_sec, cur_sym, dynstr, 1);
            else
                AddGNREntry(gnr_sec, cur_sym, dynstr, 0);
        }
        cur_sym = cur_sym->sym_next;
    }
}

static void AddGNREntry(Section *gnr_sec, Symbol *sym, Section *dynstr, int flag)
{
    Elf32_Vernaux vernaux;
    UINT8 *version_name;
    UINT16 version;
    
    version_name = sym->sym_version_name;
    version = sym->sym_version;
    
    vernaux.vna_hash = CalculateHash(version_name);
    vernaux.vna_flags = 0;
    vernaux.vna_other = version;
    vernaux.vna_name = FindOffset(dynstr, version_name);
    if (flag)
        vernaux.vna_next = 0;
    else
        vernaux.vna_next = 0x10;
    
    UINT8 *buffer;
    buffer = (UINT8 *)malloc(gnr_sec->sec_datasize + sizeof(Elf32_Vernaux));
    memcpy(buffer, gnr_sec->sec_data, gnr_sec->sec_datasize);
    memcpy(buffer + gnr_sec->sec_datasize, &vernaux, sizeof(Elf32_Vernaux));
    
    free(gnr_sec->sec_data);
    gnr_sec->sec_data = buffer;
    gnr_sec->sec_datasize = gnr_sec->sec_datasize + sizeof(Elf32_Vernaux);
}

static int FindOffset(Section *dynstr, char *str)
{
    UINT8 *data;
    data = dynstr->sec_data;
    int offset = 0;
    while (strcmp(str, data) != 0){
        data++;
        offset++;
    }
    
    return offset;
}

static void AddGOTPLTTop(Section *gotplt)
{
    int datasize, addition;
    UINT8 *buffer;
    
    addition = 0xc;
    buffer = (UINT8 *)malloc(addition);
    memset(buffer, 0x0, addition);
    
    datasize = addition;

    gotplt->sec_data = buffer;
    gotplt->sec_datasize = datasize;
}

void UpdatePLTRelatedSections(Section *sec_list, Symbol *sym_list)
{
    Section *plt, *gotplt, *relplt;
    plt = GetSectionByName(sec_list, PLT_SECTION_NAME);
    gotplt = GetSectionByName(sec_list, GOT_PLT_SECTION_NAME);
    relplt = GetSectionByName(sec_list, REL_PLT_SECTION_NAME);
    
    AddPLTTop(plt);
    AddGOTPLTTop(gotplt);
    
    Symbol *cur_sym;
    cur_sym = sym_list;
    int plt_number = 0;
    
    while (cur_sym) {
        if (cur_sym->sym_sd_type == SYM_PLT) {
            AddPLTEntry(plt, plt_number);
            AddRelPLTEntry(relplt, cur_sym, plt_number);
            AddGOTorGOTPLTEntry(gotplt);
            plt_number += 1;
        }

        cur_sym = cur_sym->sym_next;
    }
}

void UpdateGOTRelatedSections(Section *sec_list, Symbol *sym_list)
{
    Section *got, *reldyn;
    got = GetSectionByName(sec_list, GOT_SECTION_NAME);
    reldyn = GetSectionByName(sec_list, REL_DYN_SECTION_NAME);
    
    Symbol *cur_sym;
    cur_sym = sym_list;
    int got_number = 0;
    
    while (cur_sym) {
        if (cur_sym->sym_sd_type == SYM_GOT) {
            AddGOTorGOTPLTEntry(got);
            AddRelGOTEntry(reldyn, cur_sym, got_number);
            got_number += 1;
        }
        cur_sym = cur_sym->sym_next;
    }
}

static void AddPLTTop(Section *plt)
{
    Instr instr[2];
    int i, offset;
    
    UINT8 *buffer;
    buffer = (UINT8 *)malloc(0x10);
    memset(buffer, 0x0, 0x10);
    
    instr[0].opcode = 0x35ff;
    instr[0].oprand = 0x0;
    instr[1].opcode = 0x25ff;
    instr[1].oprand = 0x0;
    
    offset = 0;
    for (i = 0; i < 2; i++) {
        memcpy(buffer + offset, &(instr[i].opcode), 0x2);
        offset += 2;
        memcpy(buffer + offset, &(instr[i].oprand), 0x4);
        offset += 4;
    }
    
    plt->sec_data = buffer;
    plt->sec_datasize = 0x10;
}

static void AddPLTEntry(Section *plt, int n)
{
    Instr instr[3];
    
    instr[0].opcode = 0x25ff;
    instr[0].oprand = 0x0;
    instr[1].opcode = 0x68;
    instr[1].oprand = n * 0x8;
    instr[2].opcode = 0xe9;
    instr[2].oprand = 0x0;
    
    int i, offset, datasize, newdatasize;
    datasize = plt->sec_datasize;
    newdatasize = datasize + 0x10;
    i = offset = 0;
    
    UINT8 *buffer, *bufferOff;
    buffer = (UINT8 *)malloc(newdatasize);
    memset(buffer, 0x0, 0x10);
    memcpy(buffer, plt->sec_data, datasize);
    bufferOff = buffer + datasize;
    
    memcpy(bufferOff + offset, &(instr[0].opcode), 2);
    offset += 2;
    memcpy(bufferOff + offset, &(instr[0].oprand), 4);
    offset += 4;
    
    /* Trick: write one byte for opcode */
    for (i = 1; i < 3; i++) {
        memcpy(bufferOff + offset, &(instr[i].opcode), 1);
        offset += 1;
        memcpy(bufferOff + offset, &(instr[i].oprand), 4);
        offset += 4;
    }
    
    free(plt->sec_data);
    plt->sec_data = buffer;
    plt->sec_datasize = newdatasize;
}

static void AddGOTorGOTPLTEntry(Section *sec)
{
    int datasize, addition, newdatasize;
    addition = 0x4;
    datasize = sec->sec_datasize;
    newdatasize = datasize + addition;
    
    UINT8 *buffer;
    buffer = (UINT8 *)malloc(newdatasize);
    memset(buffer, 0x0, newdatasize);
    memcpy(buffer, sec->sec_data, datasize);
    
    free(sec->sec_data);
    sec->sec_data = buffer;
    sec->sec_datasize = newdatasize;
}

static void AddRelPLTEntry(Section *relplt, Symbol *sym, int n)
{
    Elf32_Rel cur_rel;
    int datasize, addition, newdatasize;
    
    datasize = relplt->sec_datasize;
    addition = sizeof(Elf32_Rel);
    newdatasize = datasize + addition;
    
    UINT8 *buffer;
    buffer = (UINT8 *)malloc(newdatasize);
    memset(buffer, 0x0, newdatasize);
    memcpy(buffer, relplt->sec_data, datasize);
    
    cur_rel.r_offset = 0xc + 4 * n;
    cur_rel.r_info = (sym->sym_id << 8) + R_386_JMP_SLOT;
    
    memcpy(buffer + datasize, &cur_rel, addition);
    
    free(relplt->sec_data);
    relplt->sec_data = buffer;
    relplt->sec_datasize = newdatasize;
}

static void AddRelGOTEntry(Section *relgot, Symbol *sym, int n)
{
    Elf32_Rel cur_rel;
    int datasize, addition, newdatasize;
    
    datasize = relgot->sec_datasize;
    addition = sizeof(Elf32_Rel);
    newdatasize = datasize + addition;
    
    UINT8 *buffer;
    buffer = (UINT8 *)malloc(newdatasize);
    memset(buffer, 0x0, newdatasize);
    memcpy(buffer, relgot->sec_data, datasize);
    
    cur_rel.r_offset = 4 * n;
    cur_rel.r_info = (sym->sym_id << 8) + R_386_GLOB_DAT;
    
    memcpy(buffer + datasize, &cur_rel, addition);
    
    free(relgot->sec_data);
    relgot->sec_data = buffer;
    relgot->sec_datasize = newdatasize;
}

void UpdateDynamicSection(Section *sec_list, int number)
{
    Section *dynamic;
    dynamic = GetSectionByName(sec_list, DYNAMIC_SECTION_NAME);
    
    UINT8 *buffer;
    int datasize;
    datasize = DYNAMIC_ENTSIZE * (DYNAMIC_NUMBER + number);
    buffer = (UINT8 *)malloc(datasize);
    memset(buffer, 0x0, datasize);
    
    dynamic->sec_data = buffer;
    dynamic->sec_datasize = datasize;
}

Section *MergeSection(Section *sec_list)
{
    Section *cur_sec, *last_sec, *text, *next_sec, *rm_sec, *merge_list;
    merge_list = NULL;
    cur_sec = sec_list;
    text = GetSectionByName(sec_list, TEXT_SECTION_NAME);
    
    int flag = 0;
    while (cur_sec) {
        /*printf("%d %s %s\n", cur_sec->sec_number, cur_sec->sec_name, cur_sec->sec_next->sec_name);*/
        if (cur_sec->sec_flags & SHF_MERGE) {
            /* TODO: modify the comment content */
            if (!strcmp(cur_sec->sec_name,".comment")) {
                /*MergeCommentSection(cur_sec);*/
            }
            else {
                next_sec = cur_sec->sec_next;
                rm_sec = MergeTwoSections(last_sec, cur_sec);
                flag = 1;
                if (merge_list == NULL)
                    merge_list = rm_sec;
                else
                    InsertSectionAfterSection(rm_sec, merge_list);
            }
        }
        
        if ((cur_sec->sec_flags & SHF_EXECINSTR) && !SkipXSection(cur_sec->sec_name)) {
            next_sec = cur_sec->sec_next;
            rm_sec = MergeTwoSections(text, cur_sec);
            flag = 1;
            if (merge_list == NULL)
                merge_list = rm_sec;
            else
                InsertSectionAfterSection(rm_sec, merge_list);
        }

        if (!flag) {
            last_sec = cur_sec;
            cur_sec = cur_sec->sec_next;
        }
        else {
            cur_sec = next_sec;
        }
        flag = 0;
    }
    
    return merge_list;
}

static int SkipXSection(UINT8 *name)
{
    UINT8 *sec_name[] = {
        TEXT_SECTION_NAME,
        INIT_SECTION_NAME,
        FINI_SECTION_NAME
    };
    
    int i;
    for (i = 0; i < 3; i++) {
        if (!strcmp(sec_name[i], name))
            return 1;
    }
    return 0;
}

static Section *MergeTwoSections(Section *target, Section *source)
{
    UINT8 *buffer;
    int addition, num, i, target_datasize, source_datasize, target_newdatasize;
    target_datasize = target->sec_datasize;
    source_datasize = source->sec_datasize;
    
    UINT32 source_align;
    source_align = 1;
    /*source_align <<= source->sec_align;*/
    source_align = source->sec_align;
        
    addition = 0;
    while ((target_datasize + addition) % source_align != 0) {
        addition++;
    }
    target_newdatasize = target_datasize + source_datasize + addition;
    
    buffer = (UINT8 *)malloc(target_newdatasize);
    memset(buffer, 0x0, target_newdatasize);
    memcpy(buffer, target->sec_data, target_datasize);
    memcpy(buffer + target_datasize + addition, source->sec_data, source_datasize);
    
    free(target->sec_data);
    target->sec_data = buffer;
    target->sec_datasize = target_newdatasize;
    
    source->sec_mergeto = target;
    source->sec_delta = target_datasize + addition;
    
    Section *res;
    res = CopySection(source);
    DeleteSection(source);
    
    return res;
}

static void DeleteSection(Section *useless)
{
    Section *head, *tail;
    head = useless->sec_prev;
    tail = useless->sec_next;
    head->sec_next = tail;
    tail->sec_prev = head;
    
    free(useless->sec_data);
    free(useless->sec_name);
    free(useless);
}

static Section *CopySection(Section *sec)
{
    Section *sec_shadow;
    sec_shadow = malloc(sizeof(Section));
    
    sec_shadow->sec_number = sec->sec_number;
    sec_shadow->sec_prev = sec_shadow->sec_next = NULL;
    sec_shadow->sec_mergeto = sec->sec_mergeto;
    sec_shadow->sec_datasize = sec->sec_datasize;
    sec_shadow->sec_data = malloc(sec->sec_datasize);
    memcpy(sec_shadow->sec_data, sec->sec_data, sec->sec_datasize);
    sec_shadow->sec_delta = sec->sec_delta;
    sec_shadow->sec_address = sec->sec_address;
    sec_shadow->sec_name = malloc(strlen(sec->sec_name) + 1);
    strcpy(sec_shadow->sec_name, sec->sec_name);
    sec_shadow->sec_type = sec->sec_type;
    sec_shadow->sec_flags = sec->sec_flags;
    sec_shadow->sec_link = sec->sec_link;
    sec_shadow->sec_info = sec->sec_info;
    sec_shadow->sec_align = sec->sec_align;
    sec_shadow->sec_entsize = sec->sec_entsize;
    sec_shadow->sec_name_offset = sec->sec_name_offset;
    sec_shadow->sec_file_offset = sec->sec_file_offset;
    sec_shadow->sec_misc = sec->sec_misc;
    
    return sec_shadow;
}
