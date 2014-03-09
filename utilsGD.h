/*
 * =====================================================================================
 *
 *       Filename:  utilsGD.h
 *
 *    Description:  
 *
 *        Version:  1.0
 *        Created:  03/09/2014 08:26:19 PM
 *       Revision:  none
 *       Compiler:  gcc
 *
 *         Author:  YOUR NAME (), 
 *   Organization:  
 *
 * =====================================================================================
 */

#ifndef UTILSGD_H
#define UTILSGD_H

#include <elf.h>

Elf32_Phdr Program_Headers[] = {
    {PT_PHDR,       0,  0,  0,  0,  0,  PF_R | PF_X,   0x4},
    {PT_INTERP,     0,  0,  0,  0,  0,  PF_R,          0x1},
    {PT_LOAD,       0,  0,  0,  0,  0,  PF_R | PF_X,   0x1000},
    {PT_LOAD,       0,  0,  0,  0,  0,  PF_R | PF_W,   0x1000},
    {PT_DYNAMIC,    0,  0,  0,  0,  0,  PF_R | PF_W,   0x4},
    {PT_NOTE,       0,  0,  0,  0,  0,  PF_R,          0x4},
    {PT_GNU_STACK,  0,  0,  0,  0,  0,  PF_R | PF_W,   0x10}
};
#endif
