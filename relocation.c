#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "relocation.h"

static void ApplyRelocation_32(UINT32 rel_offset, Symbol *symbol, Section *section, Symbol *dynsym_list);
static void ApplyRelocation_PC32(UINT32 rel_offset, Symbol *symbol, Section *section, Symbol *dynsym_list, Section *sec_list);
static void ApplyRelocation_GOTPC(UINT32 rel_offset, Symbol *symbol, Section *section);
static void ApplyRelocation_GOTOFF(UINT32 rel_offset, Symbol *symbol, Section *section, Section *);
static void ApplyRelocation_GOT32(UINT32 rel_offset, Symbol *symbol, Section *section, Symbol *dynsym_list);
static void ApplyRelocation_PLT32(UINT32 rel_offset, Symbol *symbol, Section *section, Symbol *dynsym_list, Section *sec_list);
static int Relocation_32_GOT(Symbol *symbol, Symbol *dynsym_list);
static int Relocation_PC32_PLT(Symbol *symbol, Symbol *dynsym_list);

Relocation* getRel(Elf32_File *elf_file){
    Relocation *first, *cur;
    first = cur = NULL;
    int i = 0, j, rel_entry_num;

    for (; i < elf_file->rel_num; i++){
	rel_entry_num = (elf_file->elf_section_table + elf_file->rel_index[i])->sh_size / sizeof(Elf32_Rel);

	Elf32_Rel *rel_table = (Elf32_Rel *)(elf_file->elf_file_data + (elf_file->elf_section_table + elf_file->rel_index[i])->sh_offset);
	for (j = 0; j < rel_entry_num; j++){
	    Relocation *temp = (Relocation *)malloc(sizeof(Relocation));

	    temp->link = (elf_file->elf_section_table + elf_file->rel_index[i])->sh_link;
	    temp->info = (elf_file->elf_section_table + elf_file->rel_index[i])->sh_info;

	    temp->offset = rel_table[j].r_offset;
	    temp->index = rel_table[j].r_info >> 8;
	    temp->type = rel_table[j].r_info & 0xff;

	    temp->rel_prev = cur;
	    if (cur)
		cur->rel_next = temp;
	    else
		first = temp;
	    cur = temp;
	}
    }
    cur = first;
    /*while (cur){*/
	/*printf("%s %s %s\n", int2str(&cur->offset, sizeof(INT32), 1, 0),*/
		   /*int2str(&cur->index, sizeof(INT32), 1, 0),*/
		   /*int2str(&cur->type, sizeof(INT8), 1, 0));*/
	/*cur = cur->rel_next;*/
    /*}*/
    /*printf("%s\n", "this is the end");*/
    return first;
}

char *int2str(void *num, unsigned long size, int fixedSize, int extend){
    int i = 0, j = 0, delete_zero = 1;
    char result_reverse[10];

    unsigned long temp = *((unsigned long*)num);
    if (num < 0)
	num += 0xffffffff + 1;

    for (; i < size * 2; i++){
	if ((temp % 16) >= 10)
	    result_reverse[i] = (temp % 16) - 10 + 'a';
	else
	    result_reverse[i] = (temp % 16) + '0';
	temp /= 16;
    }
    result_reverse[i] = '\0';

    char *result = malloc(strlen(result_reverse));
    for (i = i - 1; i >=0; i--){
	if (!fixedSize && result_reverse[i] == '0' && delete_zero)
	    continue;
	result[j++] = result_reverse[i];
	delete_zero = 0;
    }
    if (j == 0)
	result[j++] = '0';
    result[j] = '\0';

    return result;
}

void UpdateGOTForRelocations(Relocation *rel_list, Symbol *sym_list)
{
    Relocation *cur_rel;
    cur_rel = rel_list->rel_next;
    
    while (cur_rel) {
        if (cur_rel->type == R_386_GOT32) {
            UINT32 sym_index;
            sym_index = cur_rel->index;
            Symbol *symbol;
            symbol = GetSymbolByIndex(sym_list, sym_index);

            if (symbol->sym_content->st_shndx != SHN_UNDEF) {
                cur_rel = cur_rel->rel_next;
                continue;
            }
            if (!(symbol->sym_sd_type & SYM_GOT)) {
                if (symbol->sym_sd_type == SYM_LOCAL)
                    symbol->sym_sd_type ^= SYM_LOCAL;
                symbol->sym_sd_type |= SYM_GOT;
            }
        }
        cur_rel = cur_rel->rel_next;
    }
}

void UpdatePLTForRelocations(Relocation *rel_list, Symbol *sym_list)
{
    Relocation *cur_rel;
    cur_rel = rel_list->rel_next;
    
    while (cur_rel) {
        if (cur_rel->type == R_386_PLT32) {
            UINT32 sym_index;
            sym_index = cur_rel->index;
            Symbol *symbol;
            symbol = GetSymbolByIndex(sym_list, sym_index);
            
            if (symbol->sym_content->st_shndx != SHN_UNDEF) {
                cur_rel = cur_rel->rel_next;
                continue;
            }

            if (!(symbol->sym_sd_type & SYM_PLT)) {
                if (symbol->sym_sd_type == SYM_LOCAL)
                    symbol->sym_sd_type ^= SYM_LOCAL;
                symbol->sym_sd_type |= SYM_PLT;
            }
        }
        cur_rel = cur_rel->rel_next;
    }
}

void ApplyRelocations(Relocation *rel_list, Section *sec_list, Section *merge_list, Symbol *sym_list, Symbol *dynsym_list)
{
    Relocation *cur_rel;
    cur_rel = rel_list;
    UINT8 rel_type;
    UINT32 rel_offset, sym_index, sec_index, rel_value;
    
    while (cur_rel) {
        rel_type = cur_rel->type;
        rel_offset = cur_rel->offset;
        sym_index = cur_rel->index;
        sec_index = cur_rel->info;
        
        Symbol *symbol;
        Section *temp, *section;
        
        symbol = GetSymbolByIndex(sym_list, sym_index);
        temp = GetSectionByIndex(sec_list, sec_index);
        if (temp) {
            section = temp;
        }
        else {
            temp = GetSectionByIndex(merge_list, sec_index);
            section = temp->sec_mergeto;
            rel_offset += temp->sec_delta;
            
            /* modify the content of relocations in merged section, useful when
             * writing obfuscation codes */
            cur_rel->offset = rel_offset;
            cur_rel->info = section->sec_number;
        }
        
        switch (rel_type) {
            case R_386_NONE:
                break;
            case R_386_32:
                ApplyRelocation_32(rel_offset, symbol, section, dynsym_list);
                break;
            case R_386_PC32:
                ApplyRelocation_PC32(rel_offset, symbol, section, dynsym_list, sec_list);
                break;
            case R_386_GOTPC:
                ApplyRelocation_GOTPC(rel_offset, symbol, section);
                break;
            case R_386_GOTOFF:
                ApplyRelocation_GOTOFF(rel_offset, symbol, section, sec_list);
                break;
            case R_386_GOT32:
                ApplyRelocation_GOT32(rel_offset, symbol, section, dynsym_list);
                break;
            case R_386_PLT32:
                ApplyRelocation_PLT32(rel_offset, symbol, section, dynsym_list, sec_list);
                break;
            default:
                printf("error can't handle the relocation type\n");
                exit(EXIT_FAILURE);
                return ;
        }

        cur_rel = cur_rel->rel_next;
    }
}

static void ApplyRelocation_32(UINT32 rel_offset, Symbol *symbol, Section *section, Symbol *dynsym_list)
{
    int addend, value;
    int *buffer;
    buffer = (int *)malloc(sizeof(int));
    memcpy(buffer, section->sec_data + rel_offset, sizeof(int));
    addend = *buffer;
    free(buffer);
    value = 0;
    
    if (symbol->sym_sd_type & SYM_GOT) {
        /* TODO: UNFIEXD Maybe a bug, the method is for GOT32 */
        /*value = Relocation_32_GOT(symbol, dynsym_list);*/
    }
    else {
        value = symbol->sym_content->st_value + addend;
    }
    memcpy(section->sec_data + rel_offset, &value, sizeof(int));
}

/* sum: GOT number
 * index: symbol index in got start from 1
 * return value: -4 * (sum - index + 1)*/
static int Relocation_32_GOT(Symbol *symbol, Symbol *dynsym_list)
{
    Symbol *cur_sym;
    cur_sym = dynsym_list;
    int index, sum;
    sum = 0;
    
    while (cur_sym) {
        if (cur_sym->sym_sd_type & SYM_GOT)
            sum++;
        if (!strcmp(symbol->sym_name, cur_sym->sym_name)) {
            index = sum;
        }

        cur_sym = cur_sym->sym_next;
    }
    
    return (-4) * (sum - index + 1);
}

static void ApplyRelocation_PC32(UINT32 rel_offset, Symbol *symbol, Section *section, Symbol *dynsym_list, Section *sec_list)
{
    int addend, value, rel_address, plt_address;
    
    // Get the addend value
    int *buffer;
    buffer = (int *)malloc(sizeof(int));
    memcpy(buffer, section->sec_data + rel_offset, sizeof(int));
    addend = *buffer;
    free(buffer);
    
    // P
    rel_address = section->sec_address + rel_offset;
    value = 0;
    
    /* Don't handle the outside symbol's address, so caculate the address here */
    if (symbol->sym_sd_type & SYM_PLT) {
        Section *plt;
        plt = GetSectionByName(sec_list, PLT_SECTION_NAME);
        plt_address = plt->sec_address;
    
        int sym_address;
        sym_address = Relocation_PC32_PLT(symbol, dynsym_list) + plt_address;
        /*printf("%x %s\n", sym_address, symbol->sym_name);*/
        value = sym_address + addend - rel_address;
    }
    else{
        value = (int)symbol->sym_content->st_value + addend - rel_address;
        /*printf("%x %x %s\n", symbol->sym_content->st_value, value, symbol->sym_name);*/
    }
    
    memcpy(section->sec_data + rel_offset, &value, sizeof(int));
}

/* return the offset of the symbol in plt */
static int Relocation_PC32_PLT(Symbol *symbol, Symbol *dynsym_list)
{
    int i, index;
    i = 0;
    
    Symbol *cur_sym;
    cur_sym = dynsym_list;
    
    while (cur_sym) {
        if (cur_sym->sym_sd_type & SYM_PLT)
            i++;
        if (!strcmp(symbol->sym_name, cur_sym->sym_name)) {
            index = i;
            break;
        }
        
        cur_sym = cur_sym->sym_next;
    }

    return 16 * index;
}

/* .got.plt address - the relocation's instr's address (not a pc-changing instr)
 * eg: after get_pc_thunk instruction, there is a instr to find the .got.plt table
 * address which is used for finding the symbol address in got later*/
static void ApplyRelocation_GOTPC(UINT32 rel_offset, Symbol *symbol, Section *section)
{
    int addend, value, rel_address;
    
    int *buffer;
    buffer = (int *)malloc(sizeof(int));
    memcpy(buffer, section->sec_data + rel_offset, sizeof(int));
    addend = *buffer;
    free(buffer);
    
    rel_address = section->sec_address + rel_offset;
    value = (int)symbol->sym_content->st_value + addend - rel_address;
    
    memcpy(section->sec_data + rel_offset, &value, sizeof(int));
}

/* Take the .got.plt table address as an base to find the symbol's address */
static void ApplyRelocation_GOTOFF(UINT32 rel_offset, Symbol *symbol, Section *section, Section *sec_list)
{
    int value, gotplt_address;
    
    Section *gotplt;
    gotplt = GetSectionByName(sec_list, GOT_PLT_SECTION_NAME);
    gotplt_address = gotplt->sec_address;
    
    value = (int)symbol->sym_content->st_value - gotplt_address;
    
    memcpy(section->sec_data + rel_offset, &value, sizeof(int));
}

/* find the symbol's address compared to .got.plt table address
 * the symbol is in got*/
static void ApplyRelocation_GOT32(UINT32 rel_offset, Symbol *symbol, Section *section, Symbol *dynsym_list)
{
    int value;
    
    value = Relocation_32_GOT(symbol, dynsym_list);
    
    memcpy(section->sec_data + rel_offset, &value, sizeof(int));
}

/* The same as PC32
 * because PC32 have to deal with the symbol in PLT*/
static void ApplyRelocation_PLT32(UINT32 rel_offset, Symbol *symbol, Section *section, Symbol *dynsym_list, Section *sec_list)
{
    ApplyRelocation_PC32(rel_offset, symbol, section, dynsym_list, sec_list);
}
