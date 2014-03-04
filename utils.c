/*
 * =====================================================================================
 *
 *       Filename:  utils.c
 *
 *    Description:  
 *
 *        Version:  1.0
 *        Created:  03/03/2014 10:38:42 AM
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
#include "utils.h"

int GetDynDataByTag(Section *dynamic, int tag)
{
    int i, number;
    number = dynamic->sec_datasize / dynamic->sec_entsize;

    for (i = 0; i < number; i++) {
        Dyn *cur_dyn;
        cur_dyn = (Dyn *)(dynamic->sec_data + dynamic->sec_entsize * i);
        if (cur_dyn->d_tag == tag) {
            return cur_dyn->d_un.d_val;
        }
    }
    
    printf("error %x tag cann't be found\n");
    exit(EXIT_FAILURE);
}
