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

#include "type.h"

typedef enum {
    SYM_LOCAL,
    SYM_GOT,
    SYM_PLT
} SYM_SD_TYPE;

typedef struct Symbol {
    struct Symbol *sym_prev;
    struct Symbol *sym_next;
    UINT8 *sym_name;
    int sym_value;
    int sym_size;
    short sym_type;
    short sym_binding;
    short sym_shndx;
    UINT8 *sym_data;
    SYM_SD_TYPE sym_sd_type;
    short sym_version;
    UINT8 *sym_version_name;
} Symbol;
