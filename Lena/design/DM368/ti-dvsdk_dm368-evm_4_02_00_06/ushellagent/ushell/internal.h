#ifndef _INTERNAL_H_
#define _INTERNAL_H_

extern unsigned long InitSymbolTable(char *filename);
extern void PrintSymTable();
extern int BspSymFindByName(char *name, void *pValue, int *size, int *pType);

#endif