/*
 * =====================================================================================
 *
 *       Filename:  utils.h
 *
 *    Description:  
 *
 *        Version:  1.0
 *        Created:  03/03/2014 10:32:06 AM
 *       Revision:  none
 *       Compiler:  gcc
 *
 *         Author:  YOUR NAME (), 
 *   Organization:  
 *
 * =====================================================================================
 */

#ifndef UTILS_H
#define UTILS_H

#include "type.h"
#include "section.h"

typedef struct Dyn {
    INT32 d_tag;
    union {
        UINT32 d_val;
        UINT32 d_ptr;
    } d_un;
} Dyn;

int GetDynDataByTag(Section *, int);

#endif
