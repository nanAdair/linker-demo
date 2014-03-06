/*
 * =====================================================================================
 *
 *       Filename:  instr.h
 *
 *    Description:  
 *
 *        Version:  1.0
 *        Created:  03/06/2014 03:00:04 PM
 *       Revision:  none
 *       Compiler:  gcc
 *
 *         Author:  YOUR NAME (), 
 *   Organization:  
 *
 * =====================================================================================
 */

#ifndef INSTRH
#define INSTR_H

#include "type.h"

typedef struct Instr {
    UINT16 opcode;
    UINT32 oprand;
} Instr;

#endif
