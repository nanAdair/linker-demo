/*
 * =====================================================================================
 *
 *       Filename:  convert.h
 *
 *    Description:  function declaration in the corresponding .c file
 *
 *        Version:  1.0
 *        Created:  03/02/2014 10:43:03 AM
 *       Revision:  none
 *       Compiler:  gcc
 *
 *         Author:  YOUR NAME (), 
 *   Organization:  
 *
 * =====================================================================================
 */

#ifndef CONVERT_H
#define CONVERT_H

Elf32_File *GetBinaryFileData(char *);
void FillElfData(Elf32_File *, UINT8 *);

#endif
