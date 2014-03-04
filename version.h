/*
 * =====================================================================================
 *
 *       Filename:  version.h
 *
 *    Description:  get the symbol version data from a shared file
 *
 *        Version:  1.0
 *        Created:  03/03/2014 10:02:36 AM
 *       Revision:  none
 *       Compiler:  gcc
 *
 *         Author:  YOUR NAME (), 
 *   Organization:  
 *
 * =====================================================================================
 */

#ifndef VERSION_H
#define VERSION_H

#include "type.h"
#include "section.h"
#include "utils.h"

typedef struct Version_Def {
    UINT16 vd_version;
    UINT16 vd_flags;
    UINT16 vd_ndx;
    UINT16 vd_cnt;
    UINT32 vd_hash;
    UINT32 vd_aux;
    UINT32 vd_offset_next;
} Version_Def;

typedef struct Vd_item{
    struct Vd_item *vd_prev;
    struct Vd_item *vd_next;
    Version_Def *vd_content;
    UINT8 *vd_version_name;
} Vd_item;

typedef struct Vd_Aux {
    UINT32 vda_name;
    UINT32 vda_next;
} Vd_Aux;

// functions declaration
Vd_item *GetSymbolVersions(Section *);
Vd_item *FillVersionData(Section *, int, Section *);
UINT8 *GetVersionName(Vd_Aux *, Section *);
#endif
