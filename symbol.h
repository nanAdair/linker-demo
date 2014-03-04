/*
 * =====================================================================================
 *
 *       Filename:  symbol.h
 *
 *    Description:  data structure for symbols
 *
 *        Version:  1.0
 *        Created:  03/02/2014 03:45:41 PM
 *       Revision:  none
 *       Compiler:  gcc
 *
 *         Author:  YOUR NAME (), 
 *   Organization:  
 *
 * =====================================================================================
 */

#ifndef SYMBOL_H
#define SYMBOL_H

#include "elf.h"
#include "type.h"
#include "file.h"
#include "section.h"
#include "version.h"

typedef enum {
    SYM_LOCAL,
    SYM_GOT,
    SYM_PLT
} SYM_SD_TYPE;

typedef struct Symbol {
    struct Symbol *sym_prev;
    struct Symbol *sym_next;
    UINT8 *sym_name;
    Elf32_Sym *sym_content;
    SYM_SD_TYPE sym_sd_type;
    UINT16 sym_version;
    UINT8 *sym_version_name;
} Symbol;

// function declaration
Symbol *GetSymbols(Elf32_File *, Section *);
void InsertSymbolAfterSymbol(Symbol *, Symbol *);

#endif
