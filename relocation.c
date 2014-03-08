#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "relocation.h"

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
                /*ApplyRelocation_32(rel_offset, symbol, section);*/
                break;
            case R_386_PC32:
                /*ApplyRelocation_PC32(rel_offset, symbol, section);*/
                break;
            case R_386_GOTPC:
                /*ApplyRelocation_GOTPC(rel_offset, symbol, section);*/
                break;
            case R_386_GOTOFF:
                /*ApplyRelocation_GOTOFF(rel_offset, symbol, section);*/
                break;
            case R_386_GOT32:
                /*ApplyRelocation_GOT32(rel_offset, symbol, section);*/
                break;
            case R_386_PLT32:
                /*ApplyRelocation_PLT32(rel_offset, symbol, section);*/
                break;
            default:
                printf("error can't handle the relocation type\n");
                exit(EXIT_FAILURE);
                return ;
        }

        cur_rel = cur_rel->rel_next;
    }
}

/*void ApplyRelocation_32(Relocation *rel, Symbol *symbol, Section *section)*/
/*{*/

/*}*/
