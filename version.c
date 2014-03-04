/*
 * =====================================================================================
 *
 *       Filename:  version.c
 *
 *    Description:  
 *
 *        Version:  1.0
 *        Created:  03/03/2014 10:06:55 AM
 *       Revision:  none
 *       Compiler:  gcc
 *
 *         Author:  YOUR NAME (), 
 *   Organization:  
 *
 * =====================================================================================
 */
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "version.h"

static void showVersion(Vd_item *vd_list);

Vd_item *GetSymbolVersions(Section *sec_list)
{
    Section *version_def, *dynstr, *dynamic;
    
    version_def = GetSectionByName(sec_list, ".gnu.version_d");
    dynstr = GetSectionByName(sec_list, ".dynstr");
    dynamic = GetSectionByName(sec_list, ".dynamic");
    
    int numberOfVersions;
    numberOfVersions = GetDynDataByTag(dynamic, DT_VERDEFNUM);
    
    Vd_item *vd_list;
    
    vd_list = FillVersionData(version_def, numberOfVersions, dynstr);
    
    return vd_list;
}

Vd_item *FillVersionData(Section *vd, int number, Section *dynstr)
{
    Version_Def *cur_version;
    Vd_item *first_item, *cur_item, *last_item;
    first_item = cur_item = last_item = NULL;

    int i, offset;
    offset = 0;
    for (i = 0; i < number; i++) {
        cur_version = (Version_Def *)malloc(sizeof(Version_Def));
        cur_item = (Vd_item *)malloc(sizeof(Vd_item));
        if (last_item == NULL) 
            offset = 0;
        else 
            offset += last_item->vd_content->vd_offset_next;
        cur_version = (Version_Def *)(vd->sec_data + offset);
        
        cur_item->vd_content = cur_version;
        
        int offset_aux;
        offset_aux = cur_version->vd_aux;
        Vd_Aux *cur_aux;
        cur_aux = (Vd_Aux *)(vd->sec_data + offset + offset_aux);
        
        cur_item->vd_version_name = GetVersionName(cur_aux, dynstr);
        
        if (first_item == NULL) {
            first_item = cur_item;
            cur_item->vd_prev = NULL;
            cur_item->vd_next = NULL;
            last_item = cur_item;
        }
        else {
            last_item->vd_next = cur_item;
            cur_item->vd_prev = last_item;
            cur_item->vd_next = NULL;
            last_item = cur_item;
        }
    }
    
    showVersion(first_item);
    
    return first_item;
}

UINT8 *GetVersionName(Vd_Aux *verdaux, Section *dynstr)
{
    UINT8 *str;
    int length;
    length = strlen(dynstr->sec_data + verdaux->vda_name);
    str = malloc(length + 1);
    strcpy(str, dynstr->sec_data + verdaux->vda_name);
    
    return str;
}

void FreeVersionsData(Vd_item *vd_list)
{
    Vd_item *cur_item, *last_item;
    
    while (cur_item) {
        free(cur_item->vd_content);
        free(cur_item->vd_version_name);
        if (last_item != NULL)
            free(last_item);
        cur_item = cur_item->vd_next;
        last_item = cur_item;
    }
    
    free(last_item);
}

static void showVersion(Vd_item *vd_list)
{
    Vd_item *cur_item;
    cur_item = vd_list;
    
    while (cur_item != NULL) {
        printf("%d %s\n", cur_item->vd_content->vd_ndx, cur_item->vd_version_name);

        cur_item = cur_item->vd_next;
    }
}
