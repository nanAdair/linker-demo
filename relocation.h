#ifndef _RELOCATION_H
#define _RELOCATION_H 1

#include "section.h"
#include "symbol.h"

typedef struct Relocation{
    struct Relocation *rel_prev;
    struct Relocation *rel_next;
    UINT32     offset;       // relocation offset
    UINT8      type;         // relocation type
    UINT32     link;         // the symbol table it links to
    UINT32     info;         // the section it had effect on
    UINT32     index;        // index in the corresponding symbol table
    UINT32     value;        // 
} Relocation;

struct Section;
struct Symbol;
Relocation *getRel(Elf32_File *);
char *int2str(void *num, unsigned long size, int fixedSize, int extend);
void UpdateGOTForRelocations(Relocation *, struct Symbol *);
void UpdatePLTForRelocations(Relocation *, struct Symbol *);
void ApplyRelocations(Relocation *, struct Section *, struct Section *, struct Symbol *, struct Symbol *);

#endif /* _RELOCATION_H */

