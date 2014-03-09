/*
 * =====================================================================================
 *
 *       Filename:  sectionGD.h
 *
 *    Description:  section global data
 *
 *        Version:  1.0
 *        Created:  03/05/2014 10:38:33 AM
 *       Revision:  none
 *       Compiler:  gcc
 *
 *         Author:  YOUR NAME (), 
 *   Organization:  
 *
 * =====================================================================================
 */

#ifndef SECTIONGD_H
#define SECTIONGD_H

#include <elf.h>
#define ADDEDSECTIONNUMBER 12

UINT8 *AddedSectionsName[] = {
    INTERP_SECTION_NAME,
    HASH_SECTION_NAME,
    DYNSYM_SECTION_NAME,
    DYNSTR_SECTION_NAME,
    GV_SECTION_NAME,
    GNR_SECTION_NAME,
    REL_DYN_SECTION_NAME,
    REL_PLT_SECTION_NAME,
    PLT_SECTION_NAME,
    DYNAMIC_SECTION_NAME,
    GOT_SECTION_NAME,
    GOT_PLT_SECTION_NAME
};

Elf32_Shdr AddedSectionsInfo[] = {
    // sh_name, sh_type, sh_flags, sh_addr, sh_offset, sh_size, sh_link, sh_info, sh_addralign, sh_entsize;

    /* .interp */
    {0, SHT_PROGBITS,       SHF_ALLOC,                  0, 0, 0, 0, 0, 1,    0},
    /* .hash  */
    {0, SHT_HASH,           SHF_ALLOC,                  0, 0, 0, 0, 0, 4,    4},
    /* .dynsym */
    {0, SHT_DYNSYM,         SHF_ALLOC,                  0, 0, 0, 0, 0, 4,    0x10},
    /* .dynstr */
    {0, SHT_STRTAB,         SHF_ALLOC,                  0, 0, 0, 0, 0, 1,    0},
    /* .gnu.version */
    {0, SHT_GNU_versym,     SHF_ALLOC,                  0, 0, 0, 0, 0, 2,    2},
    /* .gnu.version_r */
    {0, SHT_GNU_verneed,    SHF_ALLOC,                  0, 0, 0, 0, 0, 4,    0}, 
    /* .rel.dyn */
    {0, SHT_REL,            SHF_ALLOC,                  0, 0, 0, 0, 0, 4,    8},
    /* .rel.plt */
    {0, SHT_REL,            SHF_ALLOC,                  0, 0, 0, 0, 0, 4,    8},
    /* .plt */
    {0, SHT_PROGBITS,       SHF_ALLOC | SHF_EXECINSTR,  0, 0, 0, 0, 0, 0x16, 4},
    /*  .dynamic */
    {0, SHT_DYNAMIC,        SHF_ALLOC | SHF_WRITE,      0, 0, 0, 0, 0, 4,    8},
    /*  .got */
    {0, SHT_PROGBITS,       SHF_ALLOC | SHF_WRITE,      0, 0, 0, 0, 0, 4,    4},
    /*  .got.plt */
    {0, SHT_PROGBITS,       SHF_ALLOC | SHF_WRITE,      0, 0, 0, 0, 0, 4,    4}
};

Elf32_Dyn DynamicSectionInfo[] = {
    {DT_PLTGOT,         0x0},
    {DT_PLTRELSZ,       0x0},
    {DT_JMPREL,         0x0},
    {DT_PLTREL,         0x0},
    {DT_REL,            0x0},
    {DT_RELSZ,          0x0},
    {DT_RELENT,         0x0},
    {DT_DEBUG,          0x0},
    {DT_SYMTAB,         0x0},
    {DT_SYMENT,         0x0},
    {DT_STRTAB,         0x0},
    {DT_STRSZ,          0x0},
    {DT_HASH,           0x0},
    {DT_NEEDED,         0x0},
    {DT_INIT,           0x0},
    {DT_FINI,           0x0},
    {DT_FINI_ARRAY,     0x0},
    {DT_FINI_ARRAYSZ,   0x0},
    {DT_INIT_ARRAY,     0x0},
    {DT_INIT_ARRAYSZ,   0x0},
    {DT_VERSYM,         0x0},
    {DT_VERNEED,        0x0},
    {DT_VERNEEDNUM,     0x0},
    {DT_NULL,           0x0}
};
#endif
