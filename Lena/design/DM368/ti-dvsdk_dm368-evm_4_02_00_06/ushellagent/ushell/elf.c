/*
 * This source file is elf file for the 'lena' on DM368 platform

 *****************************************************************************
 * Copyright(C) 2015, SinoMartin Corp.
 *----------------------------------------------------------------------------
 * filename	: elf.c
 * function	: elf
 * author	version		date		note
 * feller	1.0		20150928	create         
 *----------------------------------------------------------------------------
*/

#include "typedef.h"

/*************************************************************************************
*                 internal macro defined                                             *
*************************************************************************************/
#define B2L(a, size) (toLE((BYTE*)&a, size))
#define B2L2(a) (B2L(a, 2))
#define B2L4(a) (B2L(a, 4))
#define B2L8(a) (B2L(a, 8))
#define PERR printf("error at line %d\n", __LINE__)
#define MAX_SEC_NUM     	(100)
#define MAX_SEC_NAME_LEN    (30)

/*************************************************************************************
*                 internal datatype defined                                         *
*************************************************************************************/
typedef struct {
    CHAR *name;
    CHAR *addr;
    WORD32 size;
    WORD32  type;
} SYMBOL_ENT;

/************************************************************************************
 *                     local variable                                               *
************************************************************************************/
static SYMBOL_ENT* taSymTbl = NULL; /* SYMBOL_ENT taSymTbl [SYM_NUM];  */
static INT ulSymTblSize;
static INT g_iEndianType;
static CHAR s_aSecNames[MAX_SEC_NUM][MAX_SEC_NAME_LEN] = {{0}};
static BOOLEAN bSymTabInitFlag = FALSE;

/************************************************************************************
 *                     local function declaration                                   *
************************************************************************************/
static INT OpenCurrentELF(CHAR *filename);
#if(CPU_FAMILY==I80X86||CPU_FAMILY==POWERPC||CPU_FAMILY==ARM||CPU_FAMILY==MIPS)
static INT VerifyELFHeader(INT fd,Elf32_Ehdr * pHdr);
static INT BuildSymTable(Elf32_Ehdr *elfHdr, INT fd);
#endif

static BOOLEAN isOurConcernedSection (INT section_index);
static INT toLE(BYTE *p, INT size);
static WORD32 symGetPosByValue(WORDPTR value);

/************************************************************************************
 *                    local function defined                                       *
************************************************************************************/
/*----------------------------------------------------------------------------
 * name		: XOS_Malloc
 * function	: 
 * input 		: no
 * author	version		date		note
 * feller	1.0		20151010      
 *----------------------------------------------------------------------------
*/
BYTE *XOS_Malloc(WORD32 dwSize)
{
	return malloc(dwSize);	
}
/*----------------------------------------------------------------------------
 * name		: XOS_Free
 * function	: 
 * input 		: no
 * author	version		date		note
 * feller	1.0		20151010      
 *----------------------------------------------------------------------------
*/
void XOS_Free(BYTE *pucBuf)
{
	free(pucBuf);	
	return;
}

/*----------------------------------------------------------------------------
 * name		: InitSymbolTable
 * function	: 
 * input 		: no
 * author	version		date		note
 * feller	1.0		20151010      
 *----------------------------------------------------------------------------
*/
WORD32 InitSymbolTable(CHAR *filename)
{
    if(taSymTbl == NULL)
    {
        return OpenCurrentELF(filename);
    }
    else
    {
        return ERROR;
    }
}

/*----------------------------------------------------------------------------
 * name		: OpenCurrentELF
 * function	: 
 * input 		: no
 * author	version		date		note
 * feller	1.0		20151010      
 *----------------------------------------------------------------------------
*/
static INT OpenCurrentELF(CHAR *filename)
{
    INT fd;
    #if(CPU_FAMILY==I80X86||CPU_FAMILY==POWERPC||CPU_FAMILY==ARM||CPU_FAMILY== MIPS)
    Elf32_Ehdr elfHdr;
    #endif
 
    fd = open(filename, S_IRUSR);
    if (fd < 0)
    {
        printf("\nOpenCurrentELF open file error: %s\n",filename);
        return ERROR;
    }

    if (OK != VerifyELFHeader (fd, &elfHdr))
    {
        PERR;
        close(fd);
        return ERROR;
    }

    B2L2 (elfHdr.e_type);
    B2L2 (elfHdr.e_machine);
    #if(CPU_FAMILY==I80X86||CPU_FAMILY==POWERPC||CPU_FAMILY==ARM ||CPU_FAMILY==MIPS)
    B2L4 (elfHdr.e_shoff);
    #endif
  
    B2L2 (elfHdr.e_shentsize);
    B2L2 (elfHdr.e_shnum); 
    B2L2 (elfHdr.e_shstrndx);

    if (OK != BuildSymTable(&elfHdr, fd))
    {
        close(fd);
        return ERROR;
    }
    
    close(fd);
    return OK;
}

#if(CPU_FAMILY==I80X86||CPU_FAMILY==POWERPC||CPU_FAMILY==ARM ||CPU_FAMILY== MIPS)

/*----------------------------------------------------------------------------
 * name		: BuildSymTable
 * function	: 
 * input 		: no
 * author	version		date		note
 * feller	1.0		20151010      
 *----------------------------------------------------------------------------
*/
INT BuildSymTable(Elf32_Ehdr *elfHdr, INT fd)
{
    INT i;
    INT ret;
    INT iSymStrTabOffset = 0;   /* .strtabƫ�� */
    INT iSymStrTabSize = 0;     /* .strtab��С */
    INT iSymTabOffset = 0;      /* .symtabƫ�� */
    INT iSymTabSize = 0;         /* .symtab��С */
    INT iSymTabEntSize = 0;    /* .symtab��entry�Ĵ�С */

    Elf32_Shdr secHdr;                /* section header �ṹ */
    Elf32_Shdr shSecNameStrTab; /* .shstrtab section header */

    CHAR *pSecNameStrTab = NULL;  /* .shstrtab */
    CHAR *pSymStrTab = NULL;      /* .strtab */
    CHAR *pSymTab = NULL;            /* .symtab */

    /* �ҵ�.shstrtab section header */
    ret = lseek(fd, elfHdr->e_shoff + sizeof(Elf32_Shdr)*(elfHdr->e_shstrndx),SEEK_SET);
    if( -1 == ret )
    {
		return ERROR;
    }
    ret=read(fd, &shSecNameStrTab, sizeof(Elf32_Shdr));
    if ( -1 == ret || 0 == ret )
    {
        return ERROR;
    }    
    B2L4 (shSecNameStrTab.sh_name);
    B2L4 (shSecNameStrTab.sh_type);
    B2L4 (shSecNameStrTab.sh_offset);
    B2L4 (shSecNameStrTab.sh_addr);
    B2L4 (shSecNameStrTab.sh_addralign);
    B2L4 (shSecNameStrTab.sh_size);
    B2L4 (shSecNameStrTab.sh_entsize);

    /* ����.shstrtab section header�õ�.shstrtab section��ƫ�ƺʹ�С */
    /* �Ӷ���.shstrtab section����pSecNameStrTab */
    pSecNameStrTab = (CHAR*)XOS_Malloc(shSecNameStrTab.sh_size);
    if (NULL == pSecNameStrTab)
    {
        return ERROR;
    }
    ret = lseek(fd, shSecNameStrTab.sh_offset, SEEK_SET);
    if( -1 == ret )
    {
		return ERROR;
    }
    ret = read(fd, pSecNameStrTab, shSecNameStrTab.sh_size);
	if ( -1 == ret || 0 == ret )
    {
        return ERROR;
    } 
    /* ��������section header���õ�.strtab��.symtab��ƫ�ƺʹ�С */
    /* ������section�����ƶ�������.shstrtab section���ҵ� */
    ret = lseek(fd, elfHdr->e_shoff, SEEK_SET);
    if( -1 == ret )
    {
		return ERROR;
    }
    for (i = 0; i < elfHdr->e_shnum; i++)
    {
        ret = read(fd, &secHdr, sizeof(secHdr));
		if ( -1 == ret || 0 == ret )
        {
            return ERROR;
        } 
        B2L4 (secHdr.sh_name);
        B2L4 (secHdr.sh_type);
        B2L4 (secHdr.sh_offset);
        B2L4 (secHdr.sh_addr);
        B2L4 (secHdr.sh_addralign);
        B2L4 (secHdr.sh_size);
        B2L4 (secHdr.sh_entsize);
        
        /* ��¼���� */
       strncpy (&s_aSecNames[i][0], &pSecNameStrTab[secHdr.sh_name],MAX_SEC_NAME_LEN);

        /* �ҵ�.strtab section��ƫ�ƺʹ�С */
        if ((SHT_STRTAB == secHdr.sh_type) && (0 == strncmp(".strtab", &pSecNameStrTab[secHdr.sh_name], 7)))
        {
            iSymStrTabOffset = secHdr.sh_offset; /* �����ĵ�һ���ֽ����ļ�ͷ֮���ƫ�� */
            iSymStrTabSize = secHdr.sh_size;      /* �����ĳ��ȣ��ֽ����� */
            continue;
        }
        /* �ҵ�.symtab section��ƫ�ƺʹ�С */
        if ((SHT_SYMTAB == secHdr.sh_type) && (0 == strncmp(".symtab", &pSecNameStrTab[secHdr.sh_name], 7)))
        {
            iSymTabOffset = secHdr.sh_offset;  /* �����ĵ�һ���ֽ����ļ�ͷ֮���ƫ�� */
            iSymTabSize = secHdr.sh_size;       /* �����ĳ��ȣ��ֽ����� */
            iSymTabEntSize = secHdr.sh_entsize; /* ĳЩ�����а����̶���С����Ŀ������ű���������������˳�Ա����ÿ������ĳ����ֽ����� */
            continue;
        }
    }

    /* ����.strtab section��pSymStrTable */
    pSymStrTab = (CHAR*)XOS_Malloc(iSymStrTabSize);
    if (NULL == pSymStrTab)
    {
        XOS_Free((BYTE *)pSecNameStrTab);
        return ERROR;
    }
    ret = lseek(fd, iSymStrTabOffset, SEEK_SET);
    if( -1 == ret )
    {
		return ERROR;
    }
    ret=read(fd, pSymStrTab, iSymStrTabSize);
	if ( -1 == ret || 0 == ret)
    {
        return ERROR;
    }
    /* ����.symtab section��pSymTab */
    pSymTab = (CHAR*)XOS_Malloc(iSymTabSize);
    if (NULL == pSymTab)
    {
        XOS_Free((BYTE *)pSecNameStrTab);
        XOS_Free((BYTE *)pSymStrTab);
        return ERROR;
    }
    ret=lseek(fd, iSymTabOffset, SEEK_SET);
    if( -1 == ret )
    {
		return ERROR;
    }
    ret=read(fd, pSymTab, iSymTabSize);
	if ( -1 == ret || 0 == ret )
    {
        return ERROR;
    }
    /* ���������ű���ȡ���ţ� */
    INT iSymTabEntryNum = iSymTabSize / iSymTabEntSize;  /* .symtab��entry������ */

    /* ����ѭ�����У�ָ��.symtab�е�ǰ�����entry */
    Elf32_Sym *pSymEnt = (Elf32_Sym*)pSymTab;             

    /* ����FUNCTION��OBJECT���ŵ����� */
    ulSymTblSize = 0;  
    for (i = 0; i < iSymTabEntryNum; i++)
    {
        if  ((STT_FUNC == ELF32_ST_TYPE(pSymEnt->st_info) || (STT_OBJECT == ELF32_ST_TYPE(pSymEnt->st_info))) &&
            isOurConcernedSection(pSymEnt->st_shndx) &&
            0 != pSymEnt->st_shndx)
        {
            ulSymTblSize++;
        }
        pSymEnt++;
    }


    /* ������Ҫ��ȡ�ķ��������������ŷ��ŵ��ڴ� */
    taSymTbl = (SYMBOL_ENT*)XOS_Malloc(ulSymTblSize *sizeof(SYMBOL_ENT));
    if (NULL == taSymTbl)
    {
        XOS_Free((BYTE *)pSecNameStrTab);
        XOS_Free((BYTE *)pSymStrTab);
        XOS_Free((BYTE *)pSymTab);
        return ERROR;
    }
    SYMBOL_ENT *pCurMemForSym = taSymTbl;  /* ָ���������ķ��ű�ռ䣬�������շ��� */

    /* ��ȡ���ŵ�����õ��ڴ���ȥ */
    pSymEnt = (Elf32_Sym*)pSymTab;          /* ѭ������ָ��.symtab�е�ǰ���ŵ�ָ�� */
    for (i = 0; i < iSymTabEntryNum; i++)
    {
        if  ((STT_FUNC == ELF32_ST_TYPE(pSymEnt->st_info) || (STT_OBJECT == ELF32_ST_TYPE(pSymEnt->st_info))) &&
              isOurConcernedSection(pSymEnt->st_shndx))

        {
            pCurMemForSym->name = &pSymStrTab[pSymEnt->st_name];
            pCurMemForSym->addr = (CHAR*)pSymEnt->st_value;
            pCurMemForSym->type = ELF32_ST_TYPE(pSymEnt->st_info);
            pCurMemForSym->size = pSymEnt->st_size;
            pCurMemForSym++;
        }
        pSymEnt++;
    }/* end for loop */


    /* �ͷŶ��ڴ� */
    if(pSecNameStrTab)
    {
        XOS_Free((BYTE *)pSecNameStrTab);  
    }

    if(pSymStrTab)
    {
        /* �����ַ�����ʵ�ʴ��λ�ã������ͷ� */
        /* XOS_Free(pSymStrTab); */      
    }

    if(pSymTab)
    {
        XOS_Free((BYTE *)pSymTab);       
    }    
    bSymTabInitFlag = TRUE;
    return OK;
}
#endif
/*----------------------------------------------------------------------------
 * name		: BspSymFindByName
 * function	: 
 * input 		: no
 * author	version		date		note
 * feller	1.0		20151010      
 *----------------------------------------------------------------------------
*/
WORD32 BspSymFindByName(CHAR *name, WORDPTR *pValue, WORD32 *size, WORD32 *pType)
{
    WORD32 num;
    WORD32 i;
    num = ulSymTblSize;
    if(taSymTbl == NULL)
    {
        return  ERROR;
    }

    for (i = 0; i < num; i++)
    {
        if (strcmp(name, taSymTbl[i].name) == 0)
        {
            break;
        }
    }

    if (i >= num)
    {
        return  ERROR;
    }
    
    if (pValue)
    {
        *pValue = (WORDPTR)(taSymTbl[i].addr);
    }
    if (pType)
    {
        *pType = (WORD32)(taSymTbl[i].type);
    }
    if (size)
    {
        *size = (WORD32)(taSymTbl[i].size);
    }
    return OK;
}

/*----------------------------------------------------------------------------
 * name		: toLE
 * function	: 
 * input 		: no
 * author	version		date		note
 * feller	1.0		20151010      
 *----------------------------------------------------------------------------
*/
static INT toLE(BYTE *p, INT size)
{
    WORD32 i;
    WORD16 s;
    if (g_iEndianType == ELFDATA2MSB)
    {
        if (4 == size)
        {
            i = (p[0] << 24) + (p[1] << 16) + (p[2] << 8) + p[3];
            *(WORD32*)p = i;
        }
        else if (2 == size)
        {
            s = (p[0] << 8) + p[1];
            *(WORD16*)p = s;
        }
    }
    return 0;
}

#if(CPU_FAMILY==I80X86||CPU_FAMILY==POWERPC||CPU_FAMILY==ARM||CPU_FAMILY==MIPS)
/*----------------------------------------------------------------------------
 * name		: VerifyELFHeader
 * function	: 
 * input 		: no
 * author	version		date		note
 * feller	1.0		20151010      
 *----------------------------------------------------------------------------
*/
static INT VerifyELFHeader(INT fd, Elf32_Ehdr *pHdr)
{
    INT iReadRv;
    INT iHdrSize;
    iHdrSize = sizeof(*pHdr);
    iReadRv = read(fd, (CHAR*)pHdr, iHdrSize);
    if (iReadRv != iHdrSize)
    {
        printf("iReadRv = %d\n", iReadRv);
        printf("Erroneous header read\n");
        return (ERROR);
    }
    /* Is it an ELF file? */
    if (strncmp((CHAR*)pHdr->e_ident, (CHAR*)ELFMAG, SELFMAG) != 0)
    {
        return (ERROR);
    }
    if (ELFDATA2MSB == pHdr->e_ident[EI_DATA])
    {
        g_iEndianType = ELFDATA2MSB;
    }
    else if (ELFDATA2LSB == pHdr->e_ident[EI_DATA])
    {
        g_iEndianType = ELFDATA2LSB;
    }
    else
    {
        PERR;
        printf("unknown enidan type\n");
        return (ERROR);
    }
    return OK;
}
#endif

/*----------------------------------------------------------------------------
 * name		: isOurConcernedSection
 * function	: 
 * input 		: no
 * author	version		date		note
 * feller	1.0		20151010      
 *----------------------------------------------------------------------------
*/
static BOOLEAN isOurConcernedSection (INT section_index)
{
    if(section_index >= MAX_SEC_NUM || section_index < 0)
    {
        return FALSE;
    }

    /* .text: ����ͳ��� */
    /* .data: �ѳ�ʼ������ */
    /* .bss: δ��ʼ������ */
    /* .sdata: �ѳ�ʼ��small���ݣ���С��CPU��ϵ�й� */
    /* .sbss: δ��ʼ��small���ݣ���С��CPU��ϵ�й� */
    /* .tdata: �ѳ�ʼ��tls���ݣ�ARM */
    /* .tbss: δ��ʼ��tls���ݣ�ARM */
    if(strcmp(s_aSecNames[section_index],".text") == 0 ||
       strcmp(s_aSecNames[section_index],".data") == 0 ||
       strcmp(s_aSecNames[section_index],".bss") == 0 ||
       strcmp(s_aSecNames[section_index],".sbss") == 0 ||
       strcmp(s_aSecNames[section_index],".sdata") == 0 ||
       strcmp(s_aSecNames[section_index],".tbss") == 0 ||
       strcmp(s_aSecNames[section_index],".tdata") == 0)
    {
        return TRUE;
    }

    return FALSE;
}

/*----------------------------------------------------------------------------
 * name		: partition
 * function	: 
 * input 		: no
 * author	version		date		note
 * feller	1.0		20151010      
 *----------------------------------------------------------------------------
*/
static WORD32 partition(SYMBOL_ENT arr[],WORD32 low,WORD32 high)
{
    SYMBOL_ENT pivot = arr[low];
    while (low < high)
    {
        while ((low<high) && (arr[high].addr >= pivot.addr))
        {
            high--;
        }
        arr[low] = arr[high];

        while ((low<high) && (arr[low].addr <= pivot.addr))
        {
            low++;
        }
        arr[high] = arr[low];
    }
    arr[low] = pivot;
    return low;
}

/*----------------------------------------------------------------------------
 * name		: quicksort
 * function	: 
 * input 		: no
 * author	version		date		note
 * feller	1.0		20151010      
 *----------------------------------------------------------------------------
*/
static VOID quicksort(SYMBOL_ENT arr[],WORD32 low,WORD32 high)
{
    WORD32 pivotindex;
    if (low < high)
    {
        pivotindex = partition(arr,low,high);
        quicksort(arr,low,pivotindex-1);
        quicksort(arr,pivotindex+1,high);
    }
	return;
}


