/* /% C %/ */
/***********************************************************************
 * cint (C/C++ interpreter)
 ************************************************************************
 * Source file ifunc.c
 ************************************************************************
 * Description:
 *  interpret function and new style compiled function
 ************************************************************************
 * Copyright(c) 1995~1999  Masaharu Goto (MXJ02154@niftyserve.or.jp)
 *
 * Permission to use, copy, modify and distribute this software and its 
 * documentation for any purpose is hereby granted without fee,
 * provided that the above copyright notice appear in all copies and
 * that both that copyright notice and this permission notice appear
 * in supporting documentation.  The author makes no
 * representations about the suitability of this software for any
 * purpose.  It is provided "as is" without express or implied warranty.
 ************************************************************************/

#include "common.h"

#ifndef G__OLDIMPLEMENTATION1103
extern int G__const_noerror;
#endif

#ifndef G__OLDIMPLEMENTATION1167
/***********************************************************************
 * G__reftypeparam()
***********************************************************************/
void G__reftypeparam(p_ifunc,ifn,libp)
struct G__ifunc_table *p_ifunc;
int ifn;
struct G__param *libp;
{
  int itemp;
  for(itemp=0;itemp<p_ifunc->para_nu[ifn];itemp++) {
    if(G__PARAREFERENCE==p_ifunc->para_reftype[ifn][itemp] &&
       p_ifunc->para_type[ifn][itemp]!=libp->para[itemp].type) {
      switch(p_ifunc->para_type[ifn][itemp]) {
      case 'c': 
	libp->para[itemp].ref = (long)G__Charref(&libp->para[itemp]);
	break;
      case 's':
	libp->para[itemp].ref = (long)G__Shortref(&libp->para[itemp]);
	break;
      case 'i':
	libp->para[itemp].ref = (long)G__Intref(&libp->para[itemp]);
	break;
      case 'l':
	libp->para[itemp].ref = (long)G__Longref(&libp->para[itemp]);
	break;
      case 'b':
	libp->para[itemp].ref = (long)G__UCharref(&libp->para[itemp]);
	break;
      case 'r':
	libp->para[itemp].ref = (long)G__UShortref(&libp->para[itemp]);
	break;
      case 'h':
	libp->para[itemp].ref = (long)G__UIntref(&libp->para[itemp]);
	break;
      case 'k':
	libp->para[itemp].ref = (long)G__ULongref(&libp->para[itemp]);
	break;
      case 'f':
	libp->para[itemp].ref = (long)G__Floatref(&libp->para[itemp]);
	break;
      case 'd':
	libp->para[itemp].ref = (long)G__Doubleref(&libp->para[itemp]);
	break;
      }
    }
  }
}
#endif

#ifndef G__OLDIMPLEMENTATION1003
/***********************************************************************
 * G__warn_refpromotion
***********************************************************************/
static void G__warn_refpromotion(p_ifunc,ifn,itemp,libp) 
struct G__ifunc_table *p_ifunc; 
int ifn;
int itemp;
struct G__param *libp; /* argument buffer */
{
  if(G__PARAREFERENCE==p_ifunc->para_reftype[ifn][itemp] &&
     'u'!=p_ifunc->para_type[ifn][itemp] &&
     p_ifunc->para_type[ifn][itemp]!=libp->para[itemp].type &&
     0!=libp->para[itemp].obj.i &&
     G__VARIABLE==p_ifunc->para_isconst[ifn][itemp]) {
#ifdef G__OLDIMPLEMENTATION1167
    fprintf(G__serr,"Warning: implicit type conversion of non-const reference arg %d",itemp);
    G__printlinenum();
#endif
  }
}
#endif


#ifdef G__ASM_WHOLEFUNC
/***********************************************************************
* G__asm_freebytecode()
***********************************************************************/
void G__free_bytecode(bytecode)
struct G__bytecodefunc *bytecode;
{
  if(bytecode) {
    if(bytecode->asm_name) free((void*)bytecode->asm_name);
    if(bytecode->pstack) free((void*)bytecode->pstack);
    if(bytecode->pinst) free((void*)bytecode->pinst);
    if(bytecode->var) {
      G__destroy(bytecode->var,G__BYTECODELOCAL_VAR) ;
      free((void*)bytecode->var);
    }
    free((void*)bytecode);
  }
}

/***********************************************************************
* G__asm_storebytecodefunc()
***********************************************************************/
void G__asm_storebytecodefunc(ifunc,ifn,var,pstack,sp,pinst,instsize)
struct G__ifunc_table *ifunc;
int ifn;
struct G__var_array *var;
G__value *pstack;
int sp;
long *pinst;
int instsize;
{
  struct G__bytecodefunc *bytecode;

  /* check if the function is already compiled, replace old one */
  if(ifunc->pentry[ifn]->bytecode) {
    G__genericerror("Internal error: G__asm_storebytecodefunc duplicate");
  }

  /* allocate bytecode buffer */
  bytecode = (struct G__bytecodefunc*)malloc(sizeof(struct G__bytecodefunc));
  ifunc->pentry[ifn]->bytecode = bytecode;

  /* store function ID */
  bytecode->ifunc = ifunc;
  bytecode->ifn = ifn;

  /* copy local variable table */
  bytecode->var = var;
  bytecode->varsize = G__struct.size[G__tagdefining];

  /* copy instruction */
  bytecode->pinst = (long*)malloc(sizeof(long)*instsize+8);
  memcpy(bytecode->pinst,pinst,sizeof(long)*instsize+1);
  bytecode->instsize = instsize;

  /* copy constant data stack */
  bytecode->stacksize = G__MAXSTACK-sp;
  bytecode->pstack = (G__value*)malloc(sizeof(G__value)*bytecode->stacksize);
  memcpy((void*)bytecode->pstack,(void*)(&pstack[sp])
	 ,sizeof(G__value)*bytecode->stacksize);

  /* copy compiled and library function name buffer */
#ifndef G__OLDIMPLEMENTATION513
  if(0==G__asm_name_p) {
    free(G__asm_name);
    bytecode->asm_name = (char*)NULL;
  }
  else {
    bytecode->asm_name = G__asm_name;
  }
#else
  bytecode->asm_name = (char*)malloc(G__asm_name_p+4);
  memcpy(bytecode->asm_name,G__asm_name,G__asm_name_p+2);
#endif

  /* store pointer to function */
  ifunc->pentry[ifn]->tp2f = (void*)bytecode;
}

/**************************************************************************
* G__exec_bytecode()
*
*
**************************************************************************/
int G__exec_bytecode(result7,funcname,libp,hash)
G__value *result7; /* result buffer */
char *funcname; /* contains struct G__bytecode* */
struct G__param *libp; /* argument buffer */
int hash; /* not use */
{
  int i;
  struct G__bytecodefunc *bytecode;
  G__value asm_stack_g[G__MAXSTACK]; /* data stack */
  long *store_asm_inst;
  G__value *store_asm_stack;
  char *store_asm_name;
  int store_asm_name_p;
  struct G__param *store_asm_param;
  int store_asm_exec;
  int store_asm_noverflow;
  int store_asm_cp;
  int store_asm_dt;
  int store_asm_index; /* maybe unneccessary */
  int store_tagnum;
  long localmem;
#ifndef G__OLDIMPLEMENTATION507
  struct G__input_file store_ifile;
#endif
#ifndef G__OLDIMPLEMENTATION517
  int store_exec_memberfunc;
#endif
#ifndef G__OLDIMPLEMENTATION518
  long store_memberfunc_struct_offset;
  int store_memberfunc_tagnum;
#endif
#ifndef G__OLDIMPLEMENTATION542
#define G__LOCALBUFSIZE 32
  char localbuf[G__LOCALBUFSIZE];
#endif
#ifndef G__OLDIMPLEMENTATION1016
  struct G__var_array *var;
#endif

  /* use funcname as bytecode struct */
  bytecode = (struct G__bytecodefunc*)funcname;
#ifndef G__OLDIMPLEMENTATION1016
  var = bytecode->var;
#endif

#ifdef G__ASM_DBG
  if(G__asm_dbg||G__dispsource) {
    if(bytecode->ifunc->tagnum>=0) 
      fprintf(G__serr,"Running bytecode function %s::%s inst=%lx->%lx stack=%lx->%lx\n"
	      ,G__struct.name[bytecode->ifunc->tagnum]
	      ,bytecode->ifunc->funcname[bytecode->ifn]
	      ,(long)G__asm_inst,(long)bytecode->pinst
	      ,(long)G__asm_stack,(long)asm_stack_g);
    else
      fprintf(G__serr,"Running bytecode function %s inst=%lx->%lx stack=%lx->%lx\n"
	      ,bytecode->ifunc->funcname[bytecode->ifn]
	      ,(long)G__asm_inst,(long)bytecode->pinst
	      ,(long)G__asm_stack,(long)asm_stack_g);
  }
#endif

  /* Push loop compilation environment */
  store_asm_inst = G__asm_inst;
  store_asm_stack = G__asm_stack;
  store_asm_name = G__asm_name;
  store_asm_name_p = G__asm_name_p;
  store_asm_param  = G__asm_param ;
  store_asm_exec  = G__asm_exec ;
  store_asm_noverflow  = G__asm_noverflow ;
  store_asm_cp  = G__asm_cp ;
  store_asm_dt  = G__asm_dt ;
  store_asm_index  = G__asm_index ;
  store_tagnum = G__tagnum;
#ifndef G__OLDIMPLEMENTATION507
  store_ifile = G__ifile;
#endif
#ifndef G__OLDIMPLEMENTATION517
  store_exec_memberfunc = G__exec_memberfunc;
#endif
#ifndef G__OLDIMPLEMENTATION518
  store_memberfunc_struct_offset=G__memberfunc_struct_offset;
  store_memberfunc_tagnum=G__memberfunc_tagnum;
#endif

  /* set new bytecode environment */
  G__asm_inst = bytecode->pinst;
  G__asm_stack = asm_stack_g;
  G__asm_name = bytecode->asm_name;
  G__asm_name_p = 0;
  G__tagnum = bytecode->var->tagnum;
#ifndef G__OLDIMPLEMENTATION507
  G__asm_noverflow = 0; /* bug fix */
  G__ifile.filenum = bytecode->ifunc->pentry[bytecode->ifn]->filenum;
  G__ifile.line_number = bytecode->ifunc->pentry[bytecode->ifn]->line_number;
  strcpy(G__ifile.name,G__srcfile[G__ifile.filenum].filename);
#endif
#ifndef G__OLDIMPLEMENTATION517
  if(bytecode->ifunc->tagnum>=0) G__exec_memberfunc = 1;
  else                           G__exec_memberfunc = 0;
#endif
#ifndef G__OLDIMPLEMENTATION518
  G__memberfunc_struct_offset=G__store_struct_offset;
  G__memberfunc_tagnum=G__tagnum;
#endif

  /* copy constant buffer */
  memcpy((void*)(&asm_stack_g[G__MAXSTACK-bytecode->stacksize])
	 ,(void*)bytecode->pstack,bytecode->stacksize*sizeof(G__value));

  /* copy arguments to stack in reverse order */
  for(i=0;i<libp->paran;i++) {
    int j=libp->paran-i-1;
    G__asm_stack[j] = libp->para[i];
#ifndef G__OLDIMPLEMENTATION1167
    if(0==G__asm_stack[j].ref || 
       (G__PARAREFERENCE==var->reftype[i]&&var->type[i]!=libp->para[i].type)){
      if(var) {
	switch(var->type[i]) {
	case 'f':
	  G__asm_stack[j].ref=(long)G__Floatref(&libp->para[i]);
	  break;
	case 'd':
	  G__asm_stack[j].ref=(long)G__Doubleref(&libp->para[i]);
	  break;
	case 'c':
	  G__asm_stack[j].ref=(long)G__Charref(&libp->para[i]);
	  break;
	case 's':
	  G__asm_stack[j].ref=(long)G__Shortref(&libp->para[i]);
	  break;
	case 'i':
	  G__asm_stack[j].ref=(long)G__Intref(&libp->para[i]);
	  break;
	case 'l':
	  G__asm_stack[j].ref=(long)G__Longref(&libp->para[i]);
	  break;
	case 'b':
	  G__asm_stack[j].ref=(long)G__UCharref(&libp->para[i]);
	  break;
	case 'r':
	  G__asm_stack[j].ref=(long)G__UShortref(&libp->para[i]);
	  break;
	case 'h':
	  G__asm_stack[j].ref=(long)G__UIntref(&libp->para[i]);
	  break;
	case 'k':
	  G__asm_stack[j].ref=(long)G__ULongref(&libp->para[i]);
	  break;
	case 'u':
	  G__asm_stack[j].ref=libp->para[i].obj.i;
	  break;
	default:
	  G__asm_stack[j].ref=(long)(&libp->para[i].obj.i);
	  break;
	}
	if(i>=var->allvar) var=var->next;
      }
    }
#else
    if(0==G__asm_stack[j].ref) {
#ifndef G__OLDIMPLEMENTATION1016
      if(var) {
	switch(var->type[i]) {
	case 'f':
	  G__Mfloat(libp->para[i]);
	  G__asm_stack[j].ref=(long)(&libp->para[i].obj.fl);
	  break;
	case 'd':
	  G__asm_stack[j].ref=(long)(&libp->para[i].obj.d);
	  break;
	case 'c':
	  G__Mchar(libp->para[i]);
	  G__asm_stack[j].ref=(long)(&libp->para[i].obj.ch);
	  break;
	case 's':
	  G__Mshort(libp->para[i]);
	  G__asm_stack[j].ref=(long)(&libp->para[i].obj.sh);
	  break;
	case 'i':
	  G__Mint(libp->para[i]);
	  G__asm_stack[j].ref=(long)(&libp->para[i].obj.in);
	  break;
	case 'l':
	  G__asm_stack[j].ref=(long)(&libp->para[i].obj.i);
	  break;
	case 'b':
	  G__Muchar(libp->para[i]);
	  G__asm_stack[j].ref=(long)(&libp->para[i].obj.uch);
	  break;
	case 'r':
	  G__Mushort(libp->para[i]);
	  G__asm_stack[j].ref=(long)(&libp->para[i].obj.ush);
	  break;
	case 'h':
	  /* G__Muint(libp->para[i]); */
	  G__asm_stack[j].ref=(long)(&libp->para[i].obj.i);
	  break;
	case 'k':
	  G__asm_stack[j].ref=(long)(&libp->para[i].obj.i);
	  break;
	case 'u':
	  G__asm_stack[j].ref=libp->para[i].obj.i;
	  break;
	default:
	  G__asm_stack[j].ref=(long)(&libp->para[i].obj.i);
	  break;
	}
	if(i>=var->allvar) var=var->next;
      }
#else
      G__asm_stack[j].ref=(long)(&libp->para[i].obj);
#endif
    }
#endif
  }

  /* allocate local memory */
#ifndef G__OLDIMPLEMENTATION542
  if(bytecode->varsize>G__LOCALBUFSIZE) 
    localmem = (long)malloc(bytecode->varsize);
  else 
    localmem=(long)localbuf;
#else
  localmem = (long)malloc(bytecode->varsize);
#endif

#if defined(G__DUMPFILE) 
  if(G__dumpfile!=NULL) {
    int ipara;
    char resultx[G__ONELINE];
    for(ipara=0;ipara<G__dumpspace;ipara++) fprintf(G__dumpfile," ");
    fprintf(G__dumpfile,"%s(",bytecode->ifunc->funcname[bytecode->ifn]);
    for(ipara=1;ipara<= libp->paran;ipara++) {
      if(ipara!=1) fprintf(G__dumpfile,",");
      G__valuemonitor(libp->para[ipara-1],resultx);
      fprintf(G__dumpfile,"%s",resultx);
    }
    fprintf(G__dumpfile,");/*%s %d (bc)*/\n" ,G__ifile.name,G__ifile.line_number);
    G__dumpspace += 3;
    
  }
#endif

  /* run bytecode function */
  ++bytecode->ifunc->busy[bytecode->ifn];
  G__exec_asm(/*start*/0,/*stack*/libp->paran,result7,localmem);
  --bytecode->ifunc->busy[bytecode->ifn];
#ifdef G__ASM_DBG
  if(G__asm_dbg) {
    char temp[G__ONELINE];
    fprintf(G__serr,"returns %s\n",G__valuemonitor(*result7,temp));
  }
#endif
  if(G__RETURN_IMMEDIATE>=G__return) G__return=G__RETURN_NON;
  
#ifndef G__OLDIMPLEMENTATION1259
  result7->isconst = bytecode->ifunc->isconst[bytecode->ifn];
#endif

  /* free local memory */
#ifndef G__OLDIMPLEMENTATION542
  if(bytecode->varsize>G__LOCALBUFSIZE) free((void*)localmem);
#else
  free((void*)localmem);
#endif

#ifdef G__ASM_DBG
  if(G__asm_dbg||G__dispsource) {
    if(bytecode->ifunc->tagnum>=0) 
      fprintf(G__serr,"Exit bytecode function %s::%s restore inst=%lx stack=%lx\n"
	      ,G__struct.name[bytecode->ifunc->tagnum]
	      ,bytecode->ifunc->funcname[bytecode->ifn]
	      ,(long)store_asm_inst,(long)store_asm_stack);
    else
      fprintf(G__serr,"Exit bytecode function %s restore inst=%lx stack=%lx\n"
	      ,bytecode->ifunc->funcname[bytecode->ifn]
	      ,(long)store_asm_inst,(long)store_asm_stack);
  }
#endif
#ifdef G__DUMPFILE
  if(G__dumpfile!=NULL) {
    int ipara;
    char resultx[G__ONELINE];
    G__dumpspace -= 3;
    for(ipara=0;ipara<G__dumpspace;ipara++) fprintf(G__dumpfile," ");
    G__valuemonitor(*result7,resultx);
    fprintf(G__dumpfile ,"/* return(bc) %s()=%s*/\n" 
	    ,bytecode->ifunc->funcname[bytecode->ifn],resultx);
  }
#endif

  /* restore bytecode environment */
  G__asm_inst = store_asm_inst;
  G__asm_stack = store_asm_stack;
  G__asm_name = store_asm_name;
  G__asm_name_p = store_asm_name_p;
  G__asm_param  = store_asm_param ;
  G__asm_exec  = store_asm_exec ;
  G__asm_noverflow  = store_asm_noverflow ;
  G__asm_cp  = store_asm_cp ;
  G__asm_dt  = store_asm_dt ;
  G__asm_index  = store_asm_index ;
  G__tagnum = store_tagnum;
#ifndef G__OLDIMPLEMENTATION507
  G__ifile = store_ifile;
#endif
#ifndef G__OLDIMPLEMENTATION517
  G__exec_memberfunc = store_exec_memberfunc;
#endif
#ifndef G__OLDIMPLEMENTATION518
  G__memberfunc_struct_offset=store_memberfunc_struct_offset;
  G__memberfunc_tagnum=store_memberfunc_tagnum;
#endif

  return(0);
}

#ifndef G__OLDIMPLEMENTATION523
/***********************************************************************
* G__noclassargument()
*  stops bytecode compilation if class object is passed as argument
***********************************************************************/
int G__noclassargument(ifunc,iexist)
struct G__ifunc_table *ifunc;
int iexist;
{
  int i;
  for(i=0;i<ifunc->para_nu[iexist];i++) {
    if('u'==ifunc->para_type[iexist][i] &&
       G__PARAREFERENCE!=ifunc->para_reftype[iexist][i]) {
      /* return false if class/struct object and non-reference type arg */
      return(0);
    }
  }
  return(1);
}
#endif

/***********************************************************************
* G__compile_bytecode()
*
***********************************************************************/
int G__compile_bytecode(ifunc,iexist)
struct G__ifunc_table *ifunc;
int iexist;
{
  G__value buf;
  struct G__param para; /* This one is only dummy */
  struct G__input_file store_ifile;
  int store_prerun=G__prerun;
  int store_asm_index = G__asm_index;
  int store_no_exec = G__no_exec;
  int store_asm_exec = G__asm_exec;
  int store_tagdefining = G__tagdefining;
  int store_asm_noverflow = G__asm_noverflow;
  int funcstatus;
#ifndef G__OLDIMPLEMENTATION831
  long store_globalvarpointer = G__globalvarpointer;
#endif
  char funcname[G__ONELINE];

  if(
#ifndef G__OLDIMPLEMENTATION1164
     G__xrefflag ||
#endif
     (
     ifunc->pentry[iexist]->size<G__ASM_BYTECODE_FUNC_LIMIT
     && 0==G__def_struct_member 
#ifndef G__OLDIMPLEMENTATION588
     && ('u'!=ifunc->type[iexist]||G__PARAREFERENCE==ifunc->reftype[iexist])
#endif
     && (0==ifunc->para_nu[iexist] ||
	 (ifunc->ansi[iexist] && G__noclassargument(ifunc,iexist)))
      )
      ) {

    G__tagdefining = G__MAXSTRUCT-1;
    G__struct.type[G__tagdefining] = 's';
    G__struct.size[G__tagdefining] = 0;
    G__no_exec = 0;
    G__prerun = 0;
    G__asm_exec = 1;
    G__asm_wholefunction = G__ASM_FUNC_COMPILE;
    G__asm_noverflow = 0;
    store_ifile = G__ifile;
    G__asm_index = iexist;
    ++G__templevel;
    strcpy(funcname,ifunc->funcname[iexist]);
    if(-1==ifunc->tagnum) funcstatus = G__TRYNORMAL;
    else                  funcstatus = G__CALLMEMFUNC;
#ifndef G__OLDIMPLEMENTATION842
    G__init_jumptable_bytecode();
#endif
    G__interpret_func(&buf,funcname,&para
		      ,ifunc->hash[iexist] ,ifunc
		      ,G__EXACT,funcstatus);
#ifndef G__OLDIMPLEMENTATION842
    G__init_jumptable_bytecode();
#endif
    --G__templevel;
    G__tagdefining = store_tagdefining;
    G__asm_exec = store_asm_exec;
    G__no_exec = store_no_exec;
    G__prerun = store_prerun;
    G__asm_index = store_asm_index;
    G__asm_wholefunction = G__ASM_FUNC_NOP;
    G__ifile = store_ifile;
    G__asm_noverflow = store_asm_noverflow;
#ifndef G__OLDIMPLEMENTATION831
    G__globalvarpointer = store_globalvarpointer;
#endif
  }
#ifndef G__OLDIMPLEMENTATION841
  else if(G__asm_dbg) {
    fprintf(G__serr,"!!!bytecode compilation %s not tried either because\n"
	    ,ifunc->funcname[iexist]);
    fprintf(G__serr,"    function is longer than %d lines\n"
	    ,G__ASM_BYTECODE_FUNC_LIMIT);
    fprintf(G__serr,"    function returns class object or reference type\n");
    fprintf(G__serr,"    function is K&R style\n");
    G__printlinenum();
  }
#endif
    
  if(ifunc->pentry[iexist]->bytecode) {
#ifndef G__OLDIMPLEMENTATION1164
    if(0==G__xrefflag) 
      ifunc->pentry[iexist]->bytecodestatus = G__BYTECODE_SUCCESS;
    else
      ifunc->pentry[iexist]->bytecodestatus = G__BYTECODE_ANALYSIS;
#else
    ifunc->pentry[iexist]->bytecodestatus = G__BYTECODE_SUCCESS;
#endif
  }
  else if(0==G__def_struct_member)
    ifunc->pentry[iexist]->bytecodestatus = G__BYTECODE_FAILURE;

  return(ifunc->pentry[iexist]->bytecodestatus);
}

/***********************************************************************
*
***********************************************************************/
#ifndef G__OLDIMPLEMENTATION842
#define G__MAXGOTOLABEL 30

struct G__gotolabel {
 int pc;
 char *label;
};
static int G__ngoto  = 0 ;
static int G__nlabel = 0 ;
static struct G__gotolabel G__gototable[G__MAXGOTOLABEL];
static struct G__gotolabel G__labeltable[G__MAXGOTOLABEL];

/***********************************************************************
* G__free_gotolabel()
***********************************************************************/
static void G__free_gotolabel(pgotolabel,pn)
struct G__gotolabel *pgotolabel;
int *pn;
{
  while(*pn>0) {
    --(*pn);
    free((char*)pgotolabel[*pn].label);
  }
}

/***********************************************************************
* G__init_jumptable_bytecode()
*
***********************************************************************/
void G__init_jumptable_bytecode()
{
  G__free_gotolabel(G__labeltable,&G__nlabel);
  G__free_gotolabel(G__gototable,&G__ngoto);
}

/***********************************************************************
* G__add_label_bytecode()
*
***********************************************************************/
void G__add_label_bytecode(label)
char* label;
{
  if(G__nlabel<G__MAXGOTOLABEL) {
    int len=strlen(label);
    if(len) {
      G__labeltable[G__nlabel].pc = G__asm_cp;
      label[len-1] = 0;
      G__labeltable[G__nlabel].label = (char*)malloc(strlen(label)+1);
      strcpy(G__labeltable[G__nlabel].label,label);
      ++G__nlabel;
    }
  }
  else {
    G__abortbytecode();
  }
}

/***********************************************************************
* G__add_jump_bytecode()
*
***********************************************************************/
void G__add_jump_bytecode(label)
char* label; 
{
  if(G__ngoto<G__MAXGOTOLABEL) {
    int len=strlen(label);
    if(len) {
      G__gototable[G__ngoto].pc = G__asm_cp+1;
      G__asm_inst[G__asm_cp]=G__JMP;
      G__inc_cp_asm(2,0);
      G__gototable[G__ngoto].label = (char*)malloc(strlen(label)+1);
      strcpy(G__gototable[G__ngoto].label,label);
      ++G__ngoto;
    }
  }
  else {
    G__abortbytecode();
  }
}

/***********************************************************************
* G__resolve_jumptable_bytecode()
*
***********************************************************************/
void G__resolve_jumptable_bytecode()
{
  if(G__asm_noverflow) {
    int i,j;
    for(j=0;j<G__nlabel;j++) {
      for(i=0;i<G__ngoto;i++) {
	if(strcmp(G__gototable[i].label,G__labeltable[j].label)==0) {
	  G__asm_inst[G__gototable[i].pc] = G__labeltable[j].pc;
	}
      }
    }
  }
  G__init_jumptable_bytecode();
}
#endif

#endif /* G__ASM_WHOLEFUNC */

/***********************************************************************
* G__istypename()
*
* true if fundamental type, class, struct, typedef, template class name
***********************************************************************/
int G__istypename(temp)
char *temp;
{
  /* char *p; */
  /* char buf[G__MAXNAME*2]; */
  if('\0'==temp[0]) return(0);
  if(strcmp(temp,"int")==0||
     strcmp(temp,"short")==0||
     strcmp(temp,"char")==0||
     strcmp(temp,"long")==0||
     strcmp(temp,"float")==0||
     strcmp(temp,"double")==0||
#ifndef G__OLDIMPLEMENTATION742
     (strncmp(temp,"unsigned",8)==0 && 
      (strcmp(temp,"unsigned")==0||
       strcmp(temp,"unsignedchar")==0||
       strcmp(temp,"unsignedshort")==0||
       strcmp(temp,"unsignedint")==0||
       strcmp(temp,"unsignedlong")==0))||
#else
     strcmp(temp,"unsigned")==0||
#endif
     strcmp(temp,"signed")==0||
     strcmp(temp,"const")==0||
     strcmp(temp,"void")==0||
     strcmp(temp,"FILE")==0||
     strcmp(temp,"class")==0||
     strcmp(temp,"struct")==0||
     strcmp(temp,"union")==0||
     strcmp(temp,"enum")==0||
     -1!=G__defined_typename(temp)||
     -1!=G__defined_tagname(temp,2)||
     G__defined_templateclass(temp)) {
    return(1);
  }

  if(G__fpundeftype) return(1);

  return(0);
}


/***********************************************************************
* void G__make_ifunctable(funcheader)
*
* Called by
*   G__exec_statement()
*   G__define_var()
*   G__define_var()
*   G__define_var()
*
*  No change will be needed to support ANSI function prototype
* G__interpret_func() should be changed.
*
***********************************************************************/
void G__make_ifunctable(funcheader)
char *funcheader;   /* funcheader = 'funcname(' */
{
  int /* ifn=0, */ iin=0;
  int cin='\0';
  char paraname[G__ONELINE];
  int func_now;
  int iexist;
  struct G__ifunc_table *ifunc;
  char store_type;
  int store_tagnum,store_typenum;
  int isparam;
  int store_access;
  int paranu;
  int dobody=0;
#ifdef G__FRIEND
  struct G__friendtag *friendtag;
#endif
#ifdef G__NEWINHERIT
  int basen;
  struct G__inheritance *baseclass;
#endif
  int isvoid=0;
  
  /*****************************************************
   * to get type of function parameter
   *****************************************************/
  int iin2;
  fpos_t temppos;
  int store_line_number;  /* bug fix 3 mar 1993 */
  
  int store_def_struct_member;
  struct G__ifunc_table *store_ifunc;
  
  
  /* system check */
  G__ASSERT(G__prerun);
  
  store_ifunc = G__p_ifunc;
  if(G__def_struct_member && G__def_tagnum != -1) {
    /* no need for incremental setup */
    G__p_ifunc = G__struct.memfunc[G__def_tagnum] ;
  }
  
  /* Store ifunc to check if same function already exists */
  ifunc = G__p_ifunc;
  
  /* Get to the last page of interpreted function list */
  while(G__p_ifunc->next) G__p_ifunc = G__p_ifunc->next;
  
  
  /* set funcname to G__p_ifunc */
  G__func_now=G__p_ifunc->allifunc;
  G__func_page=G__p_ifunc->page;
  func_now = G__func_now;


  if('*'==funcheader[0]) {
    if('*'==funcheader[1]) {
#ifndef G__OLDIMPLEMENTATION853
      int numstar=2;
      while('*'==funcheader[numstar]) ++numstar;
#endif
#ifndef G__OLDIMPLEMENTATION853
      if(strlen(funcheader+2)>G__MAXNAME-1) {
	fprintf(G__serr
		,"Limitation: Function name length overflow strlen(%s)>%d"
		,funcheader+2,G__MAXNAME-1);
	G__genericerror((char*)NULL);
	funcheader[G__MAXNAME+1]=0;
      }
#endif
#ifndef G__OLDIMPLEMENTATION853
      strcpy(G__p_ifunc->funcname[func_now],funcheader+numstar);
#else
      strcpy(G__p_ifunc->funcname[func_now],funcheader+2);
#endif
      if(isupper(G__var_type)) {
	switch(G__reftype) {
	case G__PARANORMAL:
	  G__reftype=G__PARAP2P2P;
	  break;
	default:
	  G__reftype += 2;
	  break;
	}
      }
      else {
	switch(G__reftype) {
	case G__PARANORMAL:
	  G__reftype=G__PARAP2P;
	  break;
	case G__PARAP2P:
	  G__reftype=G__PARAP2P2P;
	  break;
	default:
	  G__reftype += 1;
	  break;
	}
      }
#ifndef G__OLDIMPLEMENTATION853
      G__reftype += numstar-2 ;
#endif
    }
    else {
#ifndef G__OLDIMPLEMENTATION853
      if(strlen(funcheader+1)>G__MAXNAME-1) {
	fprintf(G__serr
		,"Limitation: Function name length overflow strlen(%s)>%d"
		,funcheader+1,G__MAXNAME-1);
	G__genericerror((char*)NULL);
	funcheader[G__MAXNAME]=0;
      }
#endif
      strcpy(G__p_ifunc->funcname[func_now],funcheader+1);
      if(isupper(G__var_type)) {
	switch(G__reftype) {
	case G__PARANORMAL:
	  G__reftype=G__PARAP2P;
	  break;
	case G__PARAP2P:
	  G__reftype=G__PARAP2P2P;
	  break;
	default:
	  G__reftype += 1;
	  break;
	}
      }
    }
    G__var_type = toupper(G__var_type);
  }
  else {
    char *pt1;
#ifndef G__OLDIMPLEMENTATION853
    if(strlen(funcheader)>G__MAXNAME-1) {
      fprintf(G__serr
	      ,"Limitation: Function name length overflow strlen(%s)>%d"
	      ,funcheader,G__MAXNAME-1);
      G__genericerror((char*)NULL);
      funcheader[G__MAXNAME-1]=0;
    }
#endif
    strcpy(G__p_ifunc->funcname[func_now],funcheader);
    if((char*)NULL!=(pt1=strstr(funcheader,">>")) && 
       (char*)NULL!=strchr(funcheader,'<')) {
      char *pt2;
      pt2 = strstr(G__p_ifunc->funcname[func_now],">>");
      ++pt2;
      *pt2 = ' ';
      ++pt2;
      strcpy(pt2,pt1+1);
    }
  }
  G__p_ifunc->funcname[func_now][strlen(G__p_ifunc->funcname[func_now])-1]
    ='\0';
  G__hash(G__p_ifunc->funcname[func_now],G__p_ifunc->hash[func_now],iin2);

  G__p_ifunc->para_name[func_now][0]=(char*)NULL;


  /*************************************************************
   * check if the function is operator()(), if so, regenerate
   * hash value
   *************************************************************/
  if(G__HASH_OPERATOR==G__p_ifunc->hash[func_now] &&
     strcmp(G__p_ifunc->funcname[func_now],"operator")==0) {
    strcpy(G__p_ifunc->funcname[func_now],"operator()");
    G__p_ifunc->hash[func_now] += ('('+')');
  }

  fgetpos(G__ifile.fp,&G__p_ifunc->entry[func_now].pos);
  G__p_ifunc->entry[func_now].p = (void*)G__ifile.fp;
  G__p_ifunc->entry[func_now].line_number = G__ifile.line_number;
  G__p_ifunc->entry[func_now].filenum = G__ifile.filenum;
#ifdef G__TRUEP2F
  G__p_ifunc->entry[func_now].tp2f = (void*)G__p_ifunc->funcname[func_now];
#endif
#ifdef G__ASM_FUNC
  G__p_ifunc->entry[func_now].size = 0;
#endif
#ifdef G__ASM_WHOLEFUNC
  G__p_ifunc->entry[func_now].bytecode = (struct G__bytecodefunc*)NULL;
#ifndef G__OLDIMPLEMENTATION507
  G__p_ifunc->entry[func_now].bytecodestatus = G__BYTECODE_NOTYET;
#endif
#endif
  G__p_ifunc->pentry[func_now] = &G__p_ifunc->entry[func_now];
  G__p_ifunc->globalcomp[func_now]=G__globalcomp;
#ifdef G__FRIEND
  if(-1==G__friendtagnum) {
    G__p_ifunc->friendtag[func_now]=(struct G__friendtag*)NULL;
  }
  else {
    G__p_ifunc->friendtag[func_now]
      =(struct G__friendtag*)malloc(sizeof(struct G__friendtag));
    G__p_ifunc->friendtag[func_now]->next =(struct G__friendtag*)NULL;
    G__p_ifunc->friendtag[func_now]->tagnum=G__friendtagnum;
  }
#endif

  
  /*************************************************************
   * set type struct and typedef information to G__ifile
   *************************************************************/
  if(G__def_struct_member && G__def_tagnum != -1 && 
#ifdef G__OLDIMPLEMENTATION505
    /* illegular handling not to instaitiate temp object for return */
     'i'== G__var_type &&
#endif
     strcmp(G__struct.name[G__def_tagnum],G__p_ifunc->funcname[func_now])==0){
    /* constructor */
#ifndef G__OLDIMPLEMENTATION505
    /* illegular handling not to instaitiate temp object for return */
    G__p_ifunc->type[func_now] = 'i'; 
#else
    G__p_ifunc->type[func_now] = G__var_type;
#endif
    G__p_ifunc->p_tagtable[func_now] = G__def_tagnum;
    G__p_ifunc->p_typetable[func_now] = G__typenum;
#ifndef G__OLDIMPLEMENTATION1238
    G__struct.isctor[G__def_tagnum] = 1;
#endif
  }
  else {
    G__p_ifunc->type[func_now] = G__var_type;
    G__p_ifunc->p_tagtable[func_now] = G__tagnum;
    G__p_ifunc->p_typetable[func_now] = G__typenum;
  }
  
  G__p_ifunc->reftype[func_now]=G__reftype;
  G__p_ifunc->isconst[func_now]=G__constvar;
#ifndef G__OLDIMPLEMENTATION1250
  G__p_ifunc->isexplicit[func_now]=G__isexplicit;
  G__isexplicit = 0;
#endif

#ifndef G__OLDIMPLEMENTATION802
  G__reftype=G__PARANORMAL;
  /*
  G__constvar=0;
  G__var_type='p';
  G__tagnum = -1;
  G__typenum = -1;
  */
#endif
  
  if(funcheader[0]=='~') {
    /* return type is void if destructor */
    G__p_ifunc->type[func_now] = 'y';
    G__p_ifunc->p_tagtable[func_now] = -1;
    G__p_ifunc->p_typetable[func_now] = -1;
  }

#ifndef G__NEWINHERIT
  G__p_ifunc->isinherit[func_now] = 0;
#endif
  
  /*************************************************************
   * member access control
   *************************************************************/
  if(G__def_struct_member) G__p_ifunc->access[func_now]=G__access;
  else                     G__p_ifunc->access[func_now]=G__PUBLIC;
  G__p_ifunc->staticalloc[func_now] = G__static_alloc;
  
  /*************************************************************
   * initiazlize baseoffset
   *************************************************************/
#ifndef G__NEWINHERIT
  G__p_ifunc->baseoffset[func_now] = 0;
  if(-1 != G__def_tagnum) G__p_ifunc->basetagnum[func_now] = G__def_tagnum;
  else G__p_ifunc->basetagnum[func_now] = G__tagdefining;
#endif
  G__p_ifunc->isvirtual[func_now] = G__virtual;
  G__p_ifunc->ispurevirtual[func_now] = 0;
  
  /* for virtual function, allocate virtual identity member.
   * Set offset of the virtual identity member to 
   * G__struct.virtual_offset[G__p_ifunc->basetagnum[func_now]].
   */
#ifdef G__NEWINHERIT
  if(G__virtual && 
     -1==G__struct.virtual_offset[G__tagdefining]) {
#else
  if(G__virtual && 
     -1==G__struct.virtual_offset[G__p_ifunc->basetagnum[func_now]]) {
#endif
    
    store_tagnum = G__tagnum;
    store_typenum = G__typenum;
    store_type=G__var_type;
    G__tagnum = -1;
    G__typenum = -1;
    G__var_type = 'l';
    store_access=G__access;
#ifdef G__DEBUG2
    G__access=G__PUBLIC;
#else
    G__access=G__PRIVATE;
#endif
    G__letvariable("G__virtualinfo",G__null,&G__global,G__p_local);
    G__access=store_access;
    G__var_type=store_type;
    G__tagnum = store_tagnum;
    G__typenum = store_typenum;
    
#ifdef G__NEWINHERIT
    G__struct.virtual_offset[G__tagdefining]
      = G__struct.size[G__tagdefining]-G__LONGALLOC;
#else
    G__struct.virtual_offset[G__p_ifunc->basetagnum[func_now]]
      = G__struct.size[G__p_ifunc->basetagnum[func_now]]-G__LONGALLOC;
#endif
  }
  G__virtual=0; /* this position is not very best */

#ifdef G__FONS_COMMENT
  G__p_ifunc->comment[func_now].p.com = (char*)NULL;
  G__p_ifunc->comment[func_now].filenum = -1;
#endif
  
  /*************************************************************
   * initialize busy flag
   *************************************************************/
  G__p_ifunc->busy[func_now] = 0;
  
  /*************************************************************
   * store C++ or C 
   *************************************************************/
  G__p_ifunc->iscpp[func_now] = (char)G__iscpp;
  
  /*****************************************************
   * to get type of function parameter
   *****************************************************/
  
  /* remember current file position
   *   func(   int   a   ,  double   b )
   *        ^
   *  if this is an ANSI stype header, the file will be rewinded
   */
  fgetpos(G__ifile.fp,&temppos);
  store_line_number = G__ifile.line_number;  /* bub fix 3 mar 1993 */
  
  
  /* Skip parameter field  'param,param,,,)'  until ')' is found */
  
  /* check if the header is written in ANSI format or not 
   *   type func(param,param);
   *             ^
   *   func(   int   a   ,  double   b )
   *         -  -  - - - -
   */

  isparam=0;
  cin=G__fgetname_template(paraname,"<*&,)=");
#ifndef G__PHILIPPE8
  if (strlen(paraname) && isspace(cin)) {
    /* There was an argument and the parsing was stopped by a white
     * space rather than on of ",)*&<=", it is possible that 
     * we have a namespace followed by '::' in which case we have
     * to grab more before stopping!
    */
    int namespace_tagnum;
    char more[G__LONGLINE];
    
    namespace_tagnum = G__defined_tagname(paraname,2);
    while ( ( ( (namespace_tagnum!=-1)
                && (G__struct.type[namespace_tagnum]=='n') )
              || (strcmp("std",paraname)==0)
              || (paraname[strlen(paraname)-1]==':') )
            && isspace(cin) ) {
      cin = G__fgetname(more,"<*&,)=");
      strcat(paraname,more);
      namespace_tagnum = G__defined_tagname(paraname,2);
    }
  }
#endif           

  if(paraname[0]) {
    if(strcmp("void",paraname)==0) {
      if(isspace(cin)) cin = G__fgetspace();
      switch(cin) {
      case ',':
      case ')':
	G__p_ifunc->ansi[func_now]=1;
	isvoid=1;
	break;
      case '*':
      case '(':
	G__p_ifunc->ansi[func_now]=1;
	isvoid=0;
	break;
      default:
	G__genericerror("Syntax error");
	G__p_ifunc->ansi[func_now]=0;
	isvoid=1;
	break;
      }
    }
    else if(G__istypename(paraname) || strchr(paraname,'[')) {
      G__p_ifunc->ansi[func_now]=1;
      isvoid=0;
    }
    else {
#ifndef G__OLDIMPLEMENTATION976
      if(G__def_struct_member) G__genericerror("Syntax error");
#endif
#ifndef G__OLDIMPLEMENTATION1122
      if(G__globalcomp<G__NOLINK&&!G__nonansi_func
#ifdef G__ROOT
	 && strncmp(funcheader,"ClassDef",8)!=0
#endif
	 ) {
	fprintf(G__serr,"Warning: Unknown type %s in function argyment"
		,paraname);
	G__printlinenum();
      }
#endif
      G__p_ifunc->ansi[func_now]=0;
      isvoid=0;
    }
  }
  else {
    if(G__def_struct_member || G__iscpp) G__p_ifunc->ansi[func_now]=1;
    else                     G__p_ifunc->ansi[func_now]=0;
    isvoid=1;
  }
  if(')'!=cin) cin = G__fignorestream(")");

  G__static_alloc=0;

  
  /*****************************************************
   * to get type of function parameter
   *****************************************************/
  /****************************************************************
   * If ANSI style header, rewind file position to 
   *       func(int a ,double b )   ANSI
   *            ^
   * and check type of paramaters and store it into G__ifunc
   ****************************************************************/
#ifndef G__OLDIMPLEMENTATION832
  if(G__p_ifunc->ansi[func_now]) {
#else
  if(G__p_ifunc->ansi[func_now]==1) {
#endif
    
    if(isvoid) {
      G__p_ifunc->para_nu[func_now]=0;
      G__p_ifunc->para_def[func_now][0]=(char*)NULL;
      G__p_ifunc->para_default[func_now][0]=(G__value*)NULL;
    }
    else {
      if(G__dispsource) G__disp_mask=1000;
      fsetpos(G__ifile.fp,&temppos);
      G__ifile.line_number = store_line_number; 
      G__readansiproto(G__p_ifunc,func_now);
      cin=')';
      if(G__dispsource) G__disp_mask=0;
    }
  } /* end of reading ANSI parameter list */
  
  /****************************************************************
   * K&R style function header 
   *
   *
   ****************************************************************/
  else {
    if(isvoid) G__p_ifunc->para_nu[func_now] = 0;
    else       G__p_ifunc->para_nu[func_now] = -1;
  } /* end of reading K&R parameter list */
  
  /* 
   * Set G__no_exec to skip ifunc body 
   * This statement can be placed after endif.
   */
  G__no_exec = 1;
  
  /* skip space character after 
   *   func(param)      int a; {
   *              ^             */

  if(G__isfuncreturnp2f) {
    /* function returning pointer to function
     *   type (*func(param1))(param2)  { } or ;
     *                      ^ -----> ^   */
    cin=G__fignorestream(")");
    cin=G__fignorestream("(");
    cin=G__fignorestream(")");
  }
  
  cin=G__fgetstream_template(paraname,",;{");
  
  /****************************************************************
   * if header ignore following headers
   * else read func body
   ****************************************************************/
  G__mparen=0;
  if((paraname[0]=='\0'
#ifndef G__OLDIMPLEMETATION817
      ||((strncmp(paraname,"throw",5)==0
	 ||strncmp(paraname,"const throw",11)==0)&&0==strchr(paraname,'='))
#endif
      ) &&((cin==',')||(cin==';'))) {
    /* this is ANSI style func proto without param name */
    if(isparam) {
      fsetpos(G__ifile.fp,&temppos);
      G__ifile.line_number = store_line_number; 
      G__readansiproto(G__p_ifunc,func_now);
      cin = G__fignorestream(",;");
    }
    if(cin==',') {
      /* ignore other prototypes */
      G__fignorestream(";");
      if(G__globalcomp<G__NOLINK)
	G__genericerror("Limitation: Items in header must be separately specified");
    }
    /* entry fp = NULL means this is header */
    G__p_ifunc->entry[func_now].p=(void*)NULL;
    G__p_ifunc->entry[func_now].line_number = -1;
    G__p_ifunc->ispurevirtual[func_now]=0;
#ifndef G__PHILIPPE0
    /* Key the class comment off of Dictionary rather than ClassDef
     * because ClassDef is removed by a preprocessor */
    if(G__fons_comment && G__def_struct_member &&
       (strncmp(G__p_ifunc->funcname[func_now],"Dictionary",10)==0 
	|| strncmp(G__p_ifunc->funcname[func_now],"Dictionary(",11)==0
#ifndef G__OLDIMPLEMENTATION1298
	|| strncmp(G__p_ifunc->funcname[func_now],"Dictionary",10)==0 
	|| strncmp(G__p_ifunc->funcname[func_now],"Dictionary(",11)==0
#endif
       )) {
      G__fsetcomment(&G__struct.comment[G__tagdefining]);
    }
#endif
  }

  else if(strncmp(paraname,"=",1)==0 ||
	  strncmp(paraname,"const =",7)==0 ||
	  strncmp(paraname,"const=",6)==0
#ifndef G__OLDIMPLEMETATION817
	  ||((strncmp(paraname,"throw",5)==0
	      ||strncmp(paraname,"const throw",11)==0)
	     &&0!=strchr(paraname,'='))
#endif
	  ) {
    char *p;
    p=strchr(paraname,'=');
    if(0!=G__int(G__getexpr(p+1))) {
      G__genericerror("Error: invalid pure virtual function initializer");
    }
    /* this is ANSI style func proto without param name */
    if(isparam) {
      fsetpos(G__ifile.fp,&temppos);
      G__ifile.line_number = store_line_number; 
      G__readansiproto(G__p_ifunc,func_now);
      cin = G__fignorestream(",;");
    }
    if(cin==',') {
      /* ignore other prototypes */
      G__fignorestream(";");
      if(G__globalcomp<G__NOLINK)
	G__genericerror(
        "Limitation: Items in header must be separately specified");
    }
    G__p_ifunc->ansi[func_now]=1;
    /* entry fp = NULL means this is header */
    G__p_ifunc->entry[func_now].p=(void*)NULL;
    G__p_ifunc->entry[func_now].line_number = -1;
    G__p_ifunc->ispurevirtual[func_now]=1;
    if(G__tagdefining>=0) ++G__struct.isabstract[G__tagdefining];
#ifndef G__OLDIMPLEMENTATION1232
    if('~'==G__p_ifunc->funcname[func_now][0]) {
      fprintf(G__serr,"Warning: Pure virtual destructor may cause problem. Define as 'virtual %s() { }'"
	      ,G__p_ifunc->funcname[func_now]
	      );
      G__printlinenum();
    }
#endif
    if(0==strncmp(paraname,"const",5))
      G__p_ifunc->isconst[func_now]|=G__CONSTFUNC;
  }

  else if(strcmp(paraname,"const")==0 ||
	  strcmp(paraname,"const ")==0) {
    /* this is ANSI style func proto without param name */
    if(isparam) {
      fsetpos(G__ifile.fp,&temppos);
      G__ifile.line_number = store_line_number; 
      G__readansiproto(G__p_ifunc,func_now);
      cin = G__fignorestream(",;{");
    }
    if(cin==',') {/* ignore other prototypes */
      G__fignorestream(";");
      if(G__globalcomp<G__NOLINK)
	G__genericerror(
            "Limitation: Items in header must be separately specified");
    }
    if('{'==cin) {/* it is possible that this is a function body. */
      fseek(G__ifile.fp,-1,SEEK_CUR);
      if(G__dispsource) G__disp_mask=1;
      dobody=1;
    }
    else {/* entry fp = NULL means this is header */
      G__p_ifunc->entry[func_now].p=(void*)NULL;
      G__p_ifunc->entry[func_now].line_number = -1;
    }
    G__p_ifunc->ispurevirtual[func_now]=0;
    G__p_ifunc->ansi[func_now]=1;
    G__p_ifunc->isconst[func_now]|=G__CONSTFUNC;
  }


  else if(G__def_struct_member && 
	  ('}'==cin || (';'==cin && '\0'!=paraname[0]))) {
    /* Function macro as member declaration */
    /* restore file position
     *   func(   int   a   ,  double   b )
     *        ^  <------------------------+
     */
    fsetpos(G__ifile.fp,&temppos);
    G__ifile.line_number = store_line_number; 

    if(G__dispsource) G__disp_mask=1000;
    strcpy(paraname,funcheader);
    cin = G__fgetstream(paraname+strlen(paraname),")");
    iin = strlen(paraname);
    paraname[iin]=')';
    paraname[iin+1]='\0';
    if(G__dispsource) G__disp_mask=0;

    G__no_exec = 0; /* must be set to 1 again after return */
    G__func_now = -1;
    G__p_ifunc = store_ifunc ;

    G__execfuncmacro(paraname,&iin);
    if(!iin) {
      G__genericerror("Error: unrecognized language construct");
    }
#if defined(G__FONS_COMMENT) && defined(G__FONS_ROOTSPECIAL)
    else if(G__fons_comment && G__def_struct_member && 
	    (strncmp(paraname,"ClassDef",8)==0 ||
	     strncmp(paraname,"ClassDef(",9)==0 ||
	     strncmp(paraname,"ClassDefT(",10)==0)) {
      G__fsetcomment(&G__struct.comment[G__tagdefining]);
    }
#endif
    
    return; 
  }

  else { 
    /* Body of the function, skip until 
     * 'func(param)  type param;  { '
     *                             ^
     * and rewind file to just before the '{' 
     */
    if('\0'==paraname[0] && isparam) {
      /* Strange case
       *   type f(type) { }; 
       *          ^ <--  ^   */
      fsetpos(G__ifile.fp,&temppos);
      G__ifile.line_number = store_line_number; 
      G__readansiproto(G__p_ifunc,func_now);
      cin = G__fignorestream("{");
    }
    if(G__HASH_MAIN==G__p_ifunc->hash[func_now] &&
       strcmp(G__p_ifunc->funcname[func_now],"main")==0
#ifndef G__OLDIMPLEMENTATION971
       && -1==G__def_tagnum
#endif
       ) {
      G__ismain=G__MAINEXIST ;
    }
    /* following part is needed to detect inline new/delete in header */
    if(G__CPPLINK==G__globalcomp) {
      if(strcmp(G__p_ifunc->funcname[func_now],"operator new")==0&&
#ifndef G__OLDIMPLEMENTATION680
	 2==G__p_ifunc->para_nu[func_now] &&
#endif
	 0==(G__is_operator_newdelete&G__MASK_OPERATOR_NEW))
	G__is_operator_newdelete |= G__IS_OPERATOR_NEW;
      if(strcmp(G__p_ifunc->funcname[func_now],"operator delete")==0&&
	 0==(G__is_operator_newdelete&G__MASK_OPERATOR_DELETE))
	G__is_operator_newdelete |= G__IS_OPERATOR_DELETE;
    }
    if(':'==paraname[0] && 0==G__p_ifunc->ansi[func_now]) 
      G__p_ifunc->ansi[func_now]=1;
    if(cin!='{') G__fignorestream("{");
    fseek(G__ifile.fp,-1,SEEK_CUR);
    if(G__dispsource) G__disp_mask=1;
    
    /* skip body of the function surrounded by '{' '}'.
     * G__exec_statement() does the job */
    G__p_ifunc->ispurevirtual[func_now]=0;
    
    dobody=1;
  }

  if(G__nonansi_func) G__p_ifunc->ansi[func_now]=0;

#ifdef G__DETECT_NEWDEL
  /****************************************************************
   * operator new(size_t,void*) , operator delete(void*) detection
   * This is only needed for Linux g++
   ****************************************************************/
  if(G__CPPLINK==G__globalcomp) {
    if(strcmp(G__p_ifunc->funcname[func_now],"operator new")==0&&
       2==G__p_ifunc->para_nu[func_now] &&
       0==(G__is_operator_newdelete&G__MASK_OPERATOR_NEW))
      G__is_operator_newdelete |= G__IS_OPERATOR_NEW;
    if(strcmp(G__p_ifunc->funcname[func_now],"operator delete")==0&&
       0==(G__is_operator_newdelete&G__MASK_OPERATOR_DELETE))
      G__is_operator_newdelete |= G__IS_OPERATOR_DELETE;
  }
#endif


  /****************************************************************
   * Set constructor,copy constructor, destructor, operator= flags
   ****************************************************************/
  if(G__def_struct_member && G__def_tagnum != -1) {
    if('~'==G__p_ifunc->funcname[func_now][0]) {
      /* Destructor */
      G__struct.funcs[G__def_tagnum] |= G__HAS_DESTRUCTOR;
    }
    else if(strcmp(G__struct.name[G__def_tagnum]
		   ,G__p_ifunc->funcname[func_now])==0) {
      if(0==G__p_ifunc->para_nu[func_now] || 
	 G__p_ifunc->para_default[func_now][0]) {
	/* Default constructor */
	G__struct.funcs[G__def_tagnum] |= G__HAS_DEFAULTCONSTRUCTOR;
      }
      else if((1==G__p_ifunc->para_nu[func_now] || 
	       G__p_ifunc->para_default[func_now][1])&&
	      G__def_tagnum==G__p_ifunc->para_p_tagtable[func_now][0]&&
	      G__p_ifunc->para_reftype[func_now][0]) {
	/* Copy constructor */
	G__struct.funcs[G__def_tagnum] |= G__HAS_COPYCONSTRUCTOR;
      }
    }
    else if(strcmp("operator=",G__p_ifunc->funcname[func_now])==0) {
      /* operator= */
	G__struct.funcs[G__def_tagnum] |= G__HAS_ASSIGNMENTOPERATOR;
    }
  }

  /****************************************************************
   * if same function already exists       copy entry
   * else if body exists or ansi header    increment ifunc
   ****************************************************************/
  ifunc=G__ifunc_exist(G__p_ifunc,func_now ,ifunc,&iexist);

  if(G__ifile.filenum<G__nfile) {

  if(ifunc) {
#ifdef G__FRIEND
    if(G__p_ifunc->friendtag[func_now]) {
      if(ifunc->friendtag[iexist]) {
	friendtag=ifunc->friendtag[iexist];
	while(friendtag->next) friendtag=friendtag->next;
	friendtag->next = G__p_ifunc->friendtag[func_now];
      }
      else {
	ifunc->friendtag[iexist]=G__p_ifunc->friendtag[func_now];
      }
    }
#endif
    if(((FILE*)G__p_ifunc->entry[func_now].p!=(FILE*)NULL)
       /* C++ precompiled member function must not be overridden  */
       && (0==G__def_struct_member || 
	   G__CPPLINK!=G__struct.iscpplink[G__def_tagnum])) {
      ifunc->ansi[iexist]=G__p_ifunc->ansi[func_now];
      if(-1==G__p_ifunc->para_nu[func_now]) paranu=0;
      else paranu=ifunc->para_nu[iexist];
#ifndef G__OLDIMPLEMENTATION509
      if(0==ifunc->ansi[iexist]) 
	ifunc->para_nu[iexist] = G__p_ifunc->para_nu[func_now];
#else
      /* if(0==ifunc->ansi[iexist]) ifunc->para_nu[iexist] = -1; */
#endif
      ifunc->type[iexist]=G__p_ifunc->type[func_now];
      ifunc->p_tagtable[iexist]=G__p_ifunc->p_tagtable[func_now];
      ifunc->p_typetable[iexist]=G__p_ifunc->p_typetable[func_now];
      ifunc->reftype[iexist]=G__p_ifunc->reftype[func_now];
      ifunc->isconst[iexist]|=G__p_ifunc->isconst[func_now];
#ifndef G__OLDIMPLEMENTATION1250
      ifunc->isexplicit[iexist]|=G__p_ifunc->isexplicit[func_now];
#endif
      for(iin=0;iin<paranu;iin++) {
	ifunc->para_reftype[iexist][iin]
	  =G__p_ifunc->para_reftype[func_now][iin];
	ifunc->para_p_typetable[iexist][iin]
	  =G__p_ifunc->para_p_typetable[func_now][iin];
	if(G__p_ifunc->para_default[func_now][iin]) {
	  G__genericerror("Error: Duplicate default parameter definition");
	  free((void*)G__p_ifunc->para_default[func_now][iin]);
	  free((void*)G__p_ifunc->para_def[func_now][iin]);
	}
	G__p_ifunc->para_default[func_now][iin]=(G__value*)NULL;
	G__p_ifunc->para_def[func_now][iin]=(char*)NULL;
	if(ifunc->para_name[iexist][iin]) {
	  if(G__p_ifunc->para_name[func_now][iin]) {
	    free((void*)G__p_ifunc->para_name[func_now][iin]);
	    G__p_ifunc->para_name[func_now][iin]=(char*)NULL;
	  }
	}
	else {
	  ifunc->para_name[iexist][iin]=G__p_ifunc->para_name[func_now][iin];
	  G__p_ifunc->para_name[func_now][iin]=(char*)NULL;
	}
      }
      ifunc->entry[iexist]=G__p_ifunc->entry[func_now];
#ifndef G__OLDIMPLEMENTATION768
#ifndef G__PHILIPPE6
      /* The copy in previous get the wrong tp2f ... let's restore it */
      ifunc->entry[iexist].tp2f = (void*)ifunc->funcname[iexist];
#else
      G__p_ifunc->entry[iexist].tp2f = (void*)G__p_ifunc->funcname[iexist];
#endif
#endif
      ifunc->pentry[iexist]= &ifunc->entry[iexist];
      if(1==ifunc->ispurevirtual[iexist]) {
	ifunc->ispurevirtual[iexist]=G__p_ifunc->ispurevirtual[func_now];
	if(G__tagdefining>=0) --G__struct.isabstract[G__tagdefining];
      }
      else if(1==G__p_ifunc->ispurevirtual[func_now]) {
	ifunc->ispurevirtual[iexist]=G__p_ifunc->ispurevirtual[func_now];
      }
    } /* of if(G__p_ifunc->entry[func_now].p) */
    else {
      /* Entry not used, must free allocated default argument buffer */
      if(1==G__p_ifunc->ispurevirtual[func_now]) {
	if(G__tagdefining>=0) --G__struct.isabstract[G__tagdefining];
      }
      paranu=G__p_ifunc->para_nu[func_now];
      for(iin=0;iin<paranu;iin++) {
	if(G__p_ifunc->para_name[func_now][iin]) {
	  free((void*)G__p_ifunc->para_name[func_now][iin]);
	  G__p_ifunc->para_name[func_now][iin]=(char*)NULL;
	}
	if(G__p_ifunc->para_default[func_now][iin] && 
	   (&G__default_parameter)!=G__p_ifunc->para_default[func_now][iin]) {
	  free((void*)G__p_ifunc->para_default[func_now][iin]);
	  G__p_ifunc->para_default[func_now][iin]=(G__value*)NULL;
	  free((void*)G__p_ifunc->para_def[func_now][iin]);
	  G__p_ifunc->para_def[func_now][iin]=(char*)NULL;
	}
      }
    }
    G__func_page=ifunc->page;
    G__func_now = iexist;
    G__p_ifunc=ifunc;
  } /* of if(ifunc) */
  else if(G__p_ifunc->entry[func_now].p || G__p_ifunc->ansi[func_now] ||
	  G__nonansi_func || 
	  G__globalcomp<G__NOLINK || G__p_ifunc->friendtag[func_now]) {
    /* increment allifunc */
    ++G__p_ifunc->allifunc;
    
    /* Allocate and initialize function table list if needed */
    if(G__p_ifunc->allifunc==G__MAXIFUNC) {
      G__p_ifunc->next=(struct G__ifunc_table *)malloc(sizeof(struct G__ifunc_table));
      G__p_ifunc->next->allifunc=0;
      G__p_ifunc->next->next=(struct G__ifunc_table *)NULL;
      G__p_ifunc->next->page = G__p_ifunc->page+1;
#ifdef G__NEWINHERIT
      G__p_ifunc->next->tagnum = G__p_ifunc->tagnum;
#endif
    }
  }
  /* else: default parameter does not exists in K&R style 
   * no need to free default parameter buffer */

  } /* of G__ifile.filenum<G__nfile */
  else {
    G__genericerror("Limitation: Can not define body of function in tempfile");
  }
  
  if(dobody) {
    store_def_struct_member = G__def_struct_member;
    G__def_struct_member = 0;
    G__exec_statement();
    G__def_struct_member = store_def_struct_member;
#ifdef G__ASM_FUNC
    if(ifunc) {
      ifunc->pentry[iexist]->size = 
	G__ifile.line_number-ifunc->pentry[iexist]->line_number+1;
    }
    else {
      G__p_ifunc->pentry[func_now]->size = 
	G__ifile.line_number-G__p_ifunc->pentry[func_now]->line_number+1;
    }
#endif
#ifdef G__ASM_WHOLEFUNC
    /***************************************************************
    * compile as bytecode at load time if -O10 or #pragma bytecode
    ***************************************************************/
    if(G__asm_loopcompile>=10 
#ifdef G__OLDIMPLEMENTATION507
       && 0==G__def_struct_member
#endif
       ) {
      if(ifunc) {
	G__compile_bytecode(ifunc,iexist);
      }
      else {
	G__compile_bytecode(G__p_ifunc,func_now);
      }
    }
#endif
  }

#ifdef G__FONS_COMMENT
  if(G__fons_comment && G__def_struct_member) {
    if(ifunc) G__fsetcomment(&ifunc->comment[iexist]);
    else      G__fsetcomment(&G__p_ifunc->comment[func_now]);
  }
#endif

#ifdef G__NEWINHERIT
  /***********************************************************************
  * If this is a non-pure virtual member function declaration, decrement
  * isabstract flag in G__struct.
  ***********************************************************************/
#ifndef G__OLDIMPLEMENTATION820
  if(-1!=G__tagdefining && !ifunc) {
#else
  if(-1!=G__tagdefining && !ifunc && G__struct.isabstract[G__tagdefining]) {
#endif
    baseclass = G__struct.baseclass[G__tagdefining];
    for(basen=0;basen<baseclass->basen;basen++) {
      G__incsetup_memfunc(baseclass->basetagnum[basen]);
      ifunc=G__struct.memfunc[baseclass->basetagnum[basen]];
      ifunc=G__ifunc_exist(G__p_ifunc,func_now ,ifunc,&iexist);
      if(ifunc) {
#ifndef G__OLDIMPLEMENTATION820
	if(ifunc->ispurevirtual[iexist] &&
	   G__struct.isabstract[G__tagdefining]) {
#else
	if(ifunc->ispurevirtual[iexist]) {
#endif
	  --G__struct.isabstract[G__tagdefining];
	}
	G__p_ifunc->isvirtual[func_now] |= ifunc->isvirtual[iexist]; 
#ifndef G__OLDIMPLEMENTATION943
	break; /* revived by Scott Snyder */
#endif
#ifdef G__OLDIMPLEMENTATION897
	break;
#endif
      }
    }
  }
#endif

  /* finishing up */
  G__no_exec = 0;
  G__func_now = -1;
  G__p_ifunc = store_ifunc ;

  return; /* end of G__make_ifunctable */
}

/***********************************************************************
* G__readansiproto()
*
*  func(type , type* , ...)
*       ^
*
***********************************************************************/
int G__readansiproto(ifunc,func_now)
struct G__ifunc_table *ifunc;
int func_now;
{
  char paraname[G__ONELINE];
  char name[G__ONELINE];
  int c=0,iin=0;
  int tagnum,typenum,type,pointlevel,reftype;
  int isunsigned,isdefault;
  int ip,itemp;
  int store_var_type;
#ifndef G__OLDIMPLEMENTATION713
  int store_tagnum_default=0,store_def_struct_member_default=0;
  int store_exec_memberfunc=0;
#else
  int store_tagnum_default,store_def_struct_member_default;
  int store_exec_memberfunc;
#endif
#ifndef G__OLDIMPLEMENTATION573
  int arydim;
#endif

  ifunc->ansi[func_now] = 1;
  while(')'!=c) {
#ifndef G__OLDIMPLEMENTATION824
    if(G__MAXFUNCPARA==iin) {
      fprintf(G__serr
	     ,"Limitation: cint can not accept more than %d function arguments"
	      ,G__MAXFUNCPARA);
      G__printlinenum();
      G__fignorestream(")");
      return(1);
    }
#endif
    arydim=0;
    pointlevel=0;
    reftype=0; /* this reftype has different meaning from G__reftype */
    typenum = -1;
    tagnum = -1;
    isunsigned=0;
    isdefault=0;
    name[0]='\0';
    ifunc->para_isconst[func_now][iin]=G__VARIABLE;

    /* read typename */
    c=G__fgetname_template(paraname,",)&*[(=");
#ifndef G__PHILIPPE8
  if (strlen(paraname) && isspace(c)) {
    /* There was an argument and the parsing was stopped by a white
     * space rather than on of ",)*&<=", it is possible that 
     * we have a namespace followed by '::' in which case we have
     * to grab more before stopping!
    */
    int namespace_tagnum;
    char more[G__LONGLINE];
    
    namespace_tagnum = G__defined_tagname(paraname,2);
    while ( ( ( (namespace_tagnum!=-1)
                && (G__struct.type[namespace_tagnum]=='n') )
              || (strcmp("std",paraname)==0)
              || (paraname[strlen(paraname)-1]==':') )
            && isspace(c) ) {
      c = G__fgetname(more,",)&*[(=");
      strcat(paraname,more);
      namespace_tagnum = G__defined_tagname(paraname,2);
    }
  }
#endif           

    /* check const and unsigned keyword */
    if(strcmp(paraname,"...")==0) {
      strcpy(paraname,"int");
#ifndef G__OLDIMPLEMENTATION832
      ifunc->ansi[func_now] = 2;
#endif
    }
    while(strcmp(paraname,"const")==0 || strcmp(paraname,"register")==0 ||
       strcmp(paraname,"auto")==0 || strcmp(paraname,"volatile")==0) {
      if(strcmp(paraname,"const")==0) 
	ifunc->para_isconst[func_now][iin]|=G__CONSTVAR;
      c=G__fgetname_template(paraname,",)&*[(=");
    }
    if(strcmp(paraname,"unsigned")==0||strcmp(paraname,"signed")==0) {
      switch(c) {
      case ',':
      case ')':
      case '&':
      case '[':
      case '(':
      case '=':
#ifndef G__OLDIMPLEMENTATION840
      case '*':
#endif
	strcpy(paraname,"int");
	break;
      default:
#ifndef G__OLDIMPLEMENTATION840
      if(isspace(c)) {
	c=G__fgetname(paraname,",)&*[(=");
      }
      else {
	fpos_t pos;
	int store_line = G__ifile.line_number;
	fgetpos(G__ifile.fp,&pos);
	c=G__fgetname(paraname,",)&*[(=");
	if(strcmp(paraname,"int")!=0 && strcmp(paraname,"long")!=0 &&
	   strcmp(paraname,"short")!=0) {
	  G__ifile.line_number = store_line;
	  fsetpos(G__ifile.fp,&pos);
	  strcpy(paraname,"int");
	  c=' ';
	}
      }
#else
	c=G__fgetname(paraname,",)&*[(=");
#endif
	break;
      }
      isunsigned=-1;
    }

    /* determine type */
    if(strcmp(paraname,"struct")==0 || strcmp(paraname,"class")==0 ||
       strcmp(paraname,"union")==0) {
      c=G__fgetname_template(paraname,",)&*[(=");
      tagnum = G__defined_tagname(paraname,0);
      type = 'u';
    }
    else if(strcmp(paraname,"enum")==0) {
      c=G__fgetname_template(paraname,",)&*[(=");
      tagnum = G__defined_tagname(paraname,0);
      type = 'i';
    }
    else if(strcmp(paraname,"int")==0) type='i'+isunsigned;
    else if(strcmp(paraname,"char")==0) type='c'+isunsigned;
    else if(strcmp(paraname,"short")==0) type='s'+isunsigned ;
    else if(strcmp(paraname,"long")==0) type='l'+isunsigned;
    else if(strcmp(paraname,"float")==0) type='f'+isunsigned;
    else if(strcmp(paraname,"double")==0) type='d'+isunsigned;
    else if(strcmp(paraname,"void")==0) type='y';
    else if(strcmp(paraname,"FILE")==0) type='e';
    else {
      typenum=G__defined_typename(paraname);
      if(-1==typenum) {
	tagnum = G__defined_tagname(paraname,1);
	if(-1==tagnum) {
	  if(G__fpundeftype) {
	    tagnum=G__search_tagname(paraname,'c');
	    fprintf(G__fpundeftype,"class %s; /* %s %d */\n",paraname
		    ,G__ifile.name,G__ifile.line_number);
#ifndef G__OLDIMPLEMENTATION1133
	    fprintf(G__fpundeftype,"#pragma link off class %s;\n\n",paraname);
	    G__struct.globalcomp[tagnum] = G__NOLINK;
#endif
	    type='u';
	  }
	  else {
            /* In case of f(unsigned x,signed y) */
	    type='i'+isunsigned;
#ifndef G__OLDIMPLEMENTATION1126
	    if(!isdigit(paraname[0]) && 0==isunsigned) {
	      fprintf(G__serr
            ,"Warning: Unknown type '%s' in function argument handled as int"
		      ,paraname);
	      G__printlinenum();
	    }
#endif
	  }
	}
	else if('e'==G__struct.type[tagnum]) {
	  type='i';
	}
	else {
	  type = 'u';
	}
      }
      else {
	tagnum=G__newtype.tagnum[typenum];
	type=G__newtype.type[typenum];
      }
    }

    /* determine pointer level */
    while(','!=c&&')'!=c) {
      switch(c) {
      case '&': 
	++reftype; 
	c=G__fgetspace();
	break;
      case '[': 
#ifndef G__OLDIMPLEMENTATION573
	++arydim;
#endif
#ifndef G__OLDIMPLEMENTATION846
	c=G__fignorestream("],)");
#endif
      case '*':  
	++pointlevel; 
	c=G__fgetspace();
	break;
      case '(': /* func(type (*)(type,...)) */
	ip=strlen(paraname);
	if(reftype) paraname[ip++]='&';
	reftype=0;
	paraname[ip++]=' ';
	for(itemp=0;itemp<pointlevel;itemp++) paraname[ip++]='*';
	pointlevel=0;
	paraname[ip++]='(';
	c = G__fgetstream(paraname+ip,"*)");
	if('*'==c) {
#ifndef G__OLDIMPLEMENTATION1310
	  int ixx;
#endif
	  paraname[ip++]=c;
	  c = G__fgetstream(name,")");
#ifndef G__OLDIMPLEMENTATION1310
	  ixx=0;
	  while('*'==name[ixx]) {
	    paraname[ip++]='*';
	    ++ixx;
	  }
	  if(ixx) {
	    int ixxx=0;
	    while(name[ixx]) name[ixxx++] = name[ixx++];
	    name[ixxx] = 0;
	  }
#endif
	}
	if(')'==c) paraname[ip++]=')';
#ifndef G__OLDIMPLEMENTATION1249
	c = G__fdumpstream(paraname+ip,",)=");
#else
	c = G__fdumpstream(paraname+ip,",)");
#endif
	ip=strlen(paraname);
	paraname[ip++]='\0';
	typenum = G__search_typename(paraname,'Q',-1,0);
	type='Q';
	tagnum = -1;
	break;
      case '=':
	isdefault=1;
#ifndef G__OLDIMPLEMENTATION791
	c=G__fgetstream_template(paraname,",)");
#else
	c=G__fgetstream(paraname,",)");
#endif
	break;
#ifndef G__OLDIMPLEMENTATION832
      case '.':
	ifunc->ansi[func_now] = 2;
	c=G__fignorestream(",)");
	break;
#endif
#ifndef G__OLDIMPLEMENTATION909
      case EOF:
	return(1);
#endif
      default:
	if(isspace(c)) c=G__fgetspace();
	else {
#ifndef G__OLDIMPLEMENTATION832
	  if(strcmp(name,"long")==0 && strcmp(paraname,"long")==0) {
	    tagnum=G__defined_tagname("G__longlong",2);
	    if(-1==tagnum) {
	      G__genericerror("Error: 'long long' not ready. Go to $CINTSYSDIR/lib/longlong and run setup");
	    }
	    typenum=G__search_typename("long long",'u',G__tagnum,G__PARANORMAL);
	    type='u';
	  }
#endif
	  name[0] = c;
	  c=G__fgetstream(name+1,"[=,)& \t");
	  if(strcmp(name,"const")==0) {
	    ifunc->para_isconst[func_now][iin]|=G__PCONSTVAR;
#ifndef G__OLDIMPLEMENTATION1009
	    name[0]=0;
#endif
	  }
#ifndef G__OLDIMPLEMENTATION1009
	  if(strcmp(name,"const*")==0) {
	    ifunc->para_isconst[func_now][iin]|=G__PCONSTVAR;
	    ++pointlevel; 
	    name[0]=0;
	  }
#endif
	  else {
#ifndef G__OLDIMPLEMENTATION573
	    while('['==c || ']'==c) {
	      if('['==c) {
		++pointlevel;
		++arydim;
		if(G__NOLINK>G__globalcomp && 2==arydim) {
		  int len=strlen(name);
		  if(']'==name[0]) len=0;
		  strcpy(name+len,"[]");
		  pointlevel-=2;
		  len=strlen(name);
		  fseek(G__ifile.fp,-1,SEEK_CUR);
		  if(G__dispsource) G__disp_mask=1;
		  c=G__fgetstream(name+len,"=,)");
		  break;
		}
	      }
	      c=G__fignorestream("[=,)");
	    }
#else
	    while('['==c || ']'==c) {
	      if('['==c) ++pointlevel;
	      c=G__fignorestream("[=,)");
	    }
#endif
	    if('='==c) {
	      c=G__fgetstream(paraname,",)");
	      isdefault=1;
	    }
	  }
	}
	break;
      }
    }
    ifunc->para_p_tagtable[func_now][iin] = tagnum;
    ifunc->para_p_typetable[func_now][iin] = typenum;
    if(isdefault) {
#ifndef G__OLDIMPLEMENTATION1110
      int store_prerun=G__prerun;
      int store_decl=G__decl;
#endif
      ifunc->para_def[func_now][iin] = 
	(char*)malloc(strlen(paraname)+1);
      strcpy(ifunc->para_def[func_now][iin],paraname);
      ifunc->para_default[func_now][iin] = 
	(G__value*)malloc(sizeof(G__value));
      store_var_type=G__var_type;
      G__var_type='p';
      if(-1!=G__def_tagnum) {
	store_tagnum_default = G__tagnum;
	store_def_struct_member_default=G__def_struct_member;
	store_exec_memberfunc=G__exec_memberfunc;
	G__tagnum = G__def_tagnum;
	G__exec_memberfunc=1;
	G__def_struct_member=0;
      }
#ifndef G__OLDIMPLEMENTATION1233
      if('('==paraname[0]) {
	int paranamelen = strlen(paraname);
	if(paranamelen>5 && strcmp(")()",paraname+paranamelen-3)==0 &&
	   strchr(paraname,'<')) {
	  int ix;
	  for(ix=1;ix<paranamelen-4;ix++) paraname[ix-1] = paraname[ix];
	  strcpy(paraname+ix,"()");
	}
      }
#endif
#ifndef G__OLDIMPLEMENTATION1110
      if(G__NOLINK==G__globalcomp) {
	G__prerun=0;
	G__decl=1;
      }
      *ifunc->para_default[func_now][iin] = G__getexpr(paraname);
      G__prerun=store_prerun;
      G__decl=store_decl;
#else
      *ifunc->para_default[func_now][iin] = G__getexpr(paraname);
#endif
      if(-1!=G__def_tagnum) {
	G__tagnum = store_tagnum_default;
	G__exec_memberfunc=store_exec_memberfunc;
	G__def_struct_member=store_def_struct_member_default;
      }
      G__var_type=store_var_type;
    }
    else {
      ifunc->para_default[func_now][iin] = (G__value*)NULL;
      ifunc->para_def[func_now][iin] = (char*)NULL;
    }
    if(reftype) {
      if(pointlevel) type = toupper(type);
      ifunc->para_type[func_now][iin] = type ;
      ifunc->para_reftype[func_now][iin] = G__PARAREFERENCE ;
    }
    else {
      if(isupper(type)&&pointlevel) pointlevel++;
#ifndef G__OLDIMPLEMENTATION929
      if(-1!=typenum && G__newtype.reftype[typenum]>=G__PARAP2P) {
        pointlevel+=G__newtype.reftype[typenum]-G__PARAP2P+2;
        type=tolower(type);
      }
#endif
      switch(pointlevel) {
      case 0:
	ifunc->para_type[func_now][iin] = type ;
	ifunc->para_reftype[func_now][iin] = G__PARANORMAL ;
	break;
      case 1:
	ifunc->para_type[func_now][iin] = toupper(type) ;
	ifunc->para_reftype[func_now][iin] = G__PARANORMAL ;
	break;
#ifndef G__OLDIMPLEMENTATION763
      default:
	ifunc->para_type[func_now][iin] = toupper(type) ;
	ifunc->para_reftype[func_now][iin] = pointlevel-2 + G__PARAP2P ;
#else
      case 2:
	ifunc->para_type[func_now][iin] = toupper(type) ;
	ifunc->para_reftype[func_now][iin] = G__PARAP2P ;
	break;
      default:
	ifunc->para_type[func_now][iin] = toupper(type) ;
	ifunc->para_reftype[func_now][iin] = G__PARAP2P2P ;
	break;
#endif
      }
    }

    /* paranemter name omitted */
    if(name[0]) {
      ifunc->para_name[func_now][iin] = (char*)malloc(strlen(name)+1);
      strcpy(ifunc->para_name[func_now][iin],name);
    }
    else {
      ifunc->para_name[func_now][iin] = (char*)NULL;
    }
    ++iin;
  } /* while(')'!=c) */
  ifunc->para_nu[func_now]=iin;
  return(0);
}

#ifndef G__OLDIMPLEMENTATION1120
/***********************************************************************
* int G__matchpointlevel
***********************************************************************/
int G__matchpointlevel(param_reftype,formal_reftype)
int param_reftype;
int formal_reftype;
{
  switch(param_reftype) {
  case G__PARANORMAL:
  case G__PARAREFERENCE:
    if(G__PARANORMAL==formal_reftype||G__PARAREFERENCE==formal_reftype) 
      return(1);
    else
      return(0);
  default:
    return(formal_reftype==param_reftype);
  }
}
#endif

/***********************************************************************
* int G__param_match()
***********************************************************************/
int G__param_match(formal_type,formal_tagnum
		   ,default_parameter
		   ,param_type,param_tagnum
		   ,param
		   ,parameter
		   ,funcmatch
		   ,rewind_arg
#ifndef G__OLDIMPLEMENTATION1120
		   ,formal_reftype
#endif
#ifndef G__OLDIMPLEMENTATION1208
		   ,formal_isconst
#endif
		   )
char formal_type;
int formal_tagnum;
G__value *default_parameter;
char param_type;
int param_tagnum;
G__value *param;
char *parameter;
int funcmatch;
int rewind_arg;
#ifndef G__OLDIMPLEMENTATION1120
int formal_reftype;
#endif
#ifndef G__OLDIMPLEMENTATION1208
int formal_isconst;
#endif
{
  int match;
  static int recursive=0;
  long store_struct_offset; /* used to be int */
  int store_tagnum;
  char conv[G__ONELINE],arg1[G__ONELINE];
  int baseoffset;
  G__value reg;
  int store_oprovld;
  int rewindflag=0;

  if(default_parameter && param_type == '\0') {
    return(2);
  }

  if(funcmatch>=G__EXACT) {
    if(param_type==formal_type && 0==recursive){

      if(tolower(param_type)=='u') {
	/* If struct,class,union, check tagnum */
	if(formal_tagnum != param_tagnum) { /* unmatch */
	  match=0; 
	}
	else {  /* match */
	  match=1;
	}
      }
      else { /* match */
	match=1;
      }

    }
#ifndef G__OLDIMPLEMENTATION978
    else if('I'==param_type&&'U'==formal_type&&param_tagnum==formal_tagnum&&
	    -1!=formal_tagnum&&'e'==G__struct.type[formal_tagnum]) {
      match=1;
    }
#endif
    else {  /* unmatch */
      match=0;
    }
  }

  if(match==0&&funcmatch>=G__PROMOTION) {
    switch(formal_type) {
    case 'd':
    case 'f':
      switch(param_type) {
      case 'b':
      case 'c':
      case 'r':
      case 's':
      case 'h':
      case 'i':
      case 'k':
      case 'l':
#define G__OLDIMPLEMENTATION1165
#ifndef G__OLDIMPLEMENTATION1165
	if(G__PARAREFERENCE==formal_reftype) {
          param->obj.d = param->obj.i;
          param->type = formal_type;
          param->ref = 0;
        }
#endif
      case 'd':
      case 'f':
	match=1;
	break;
      default:
	match=0;
	break;
      }
      break;
    case 'l':
      switch(param_type) {
      case 'b':
      case 'c':
      case 'r':
      case 's':
      /* case 'h': */
      case 'i':
      /* case 'k': */
      case 'l':
	match=1;
	break;
      default:
	match=0;
	break;
      }
      break;
    case 'i':
      switch(param_type) {
      case 'b':
      case 'c':
      case 'r':
      case 's':
      /* case 'h': */
      case 'i':
      /* case 'k': */
      /* case 'l': */
	match=1;
	break;
#ifndef G__OLDIMPLEMENTATION914
      case 'u':
        if('e'==G__struct.type[param_tagnum]) {
          if(param->ref) param->obj.i = *(long*)(param->ref);
          match=1;
          break;
        }
#endif
      default:
	match=0;
	break;
      }
      break;
    case 's':
      switch(param_type) {
      case 'b':
      case 'c':
      /* case 'r': */
      case 's':
      /* case 'h': */
      /* case 'i': */
      /* case 'k': */
      /* case 'l': */
	match=1;
	break;
      default:
	match=0;
	break;
      }
      break;
    case 'k':
      switch(param_type) {
      case 'b':
      /* case 'c': */
      case 'r':
      /* case 's': */
      case 'h':
      /* case 'i': */
      case 'k':
      /* case 'l': */
	match=1;
	break;
      default:
	match=0;
	break;
      }
      break;
    case 'h':
      switch(param_type) {
      case 'b':
      /* case 'c': */
      case 'r':
      /* case 's': */
      case 'h':
      /* case 'i': */
      /* case 'k': */
      /* case 'l': */
	match=1;
	break;
      default:
	match=0;
	break;
      }
      break;
    case 'r':
      switch(param_type) {
      case 'b':
      /* case 'c': */
      case 'r':
      /* case 's': */
      /* case 'h': */
      /* case 'i': */
      /* case 'k': */
      /* case 'l': */
	match=1;
	break;
      default:
	match=0;
	break;
      }
      break;
    case 'u':
      if(0<=formal_tagnum && 'e'==G__struct.type[formal_tagnum]) {
	switch(param_type) {
	case 'i':
	case 's':
	case 'l':
	case 'c':
	case 'h':
	case 'r':
	case 'k':
	case 'b':
	  match=1;
	  break;
	default:
	  match=0;
	  break;
	}
      }
      else {
	match=0;
      }
      break;
    default:
      match=0;
      break;
    }
  }

  if(match==0&&funcmatch>=G__STDCONV) {
    switch(formal_type) {
#ifndef G__OLDIMPLEMENTATION1165
    case 'b':
    case 'c':
    case 'r':
    case 's':
    case 'h':
    case 'i':
    case 'k':
    case 'l':
      switch(param_type) {
      case 'd':
      case 'f':
	if(G__PARAREFERENCE==formal_reftype) {
          param->obj.i = (long)param->obj.d;
          param->type = formal_type;
          param->ref = 0;
        }
      case 'b':
      case 'c':
      case 'r':
      case 's':
      case 'h':
      case 'i':
      case 'k':
      case 'l':
	match=1;
	break;
      default:
	match=0;
	break;
      }
      break;
    case 'd':
    case 'f':
      switch(param_type) {
      case 'b':
      case 'c':
      case 'r':
      case 's':
      case 'h':
      case 'i':
      case 'k':
      case 'l':
	if(G__PARAREFERENCE==formal_reftype) {
          param->obj.d = param->obj.i;
          param->type = formal_type;
          param->ref = 0;
        }
      case 'd':
      case 'f':
	match=1;
	break;
      default:
	match=0;
	break;
      }
      break;
#else
    case 'b':
    case 'c':
    case 'r':
    case 's':
    case 'h':
    case 'i':
    case 'k':
    case 'l':
    case 'd':
    case 'f':
      switch(param_type) {
      case 'b':
      case 'c':
      case 'r':
      case 's':
      case 'h':
      case 'i':
      case 'k':
      case 'l':
      case 'd':
      case 'f':
	match=1;
	break;
      default:
	match=0;
	break;
      }
      break;
#endif
    case 'C':
      switch(param_type) {
      case 'i':
      case 'l':
#ifndef G__OLDIMPLEMENTATION839
      if(0==param->obj.i) match=1;
      else match=0;
      break;
#endif
      case 'Y':
#ifdef G__OLDIMPLEMENTATION1219
      case 'Q': /* questionable */
#endif
	match=1;
	break;
      default:
	match=0;
	break;
      }
      break;
    case 'Y':
#ifdef G__OLDIMPLEMENTATION1219
    case 'Q': /* questionable */
#endif
#ifndef G__OLDIMPLEMENTATION839
      if(isupper(param_type)||0==param->obj.i) {
#else
      if(isupper(param_type)) {
#endif
	match=1;
      }
      else {
	match=0;
      }
      break;
#ifndef G__OLDIMPLEMENTATION1225
    case 'Q': /* questionable */
      if('Q'==param_type||'C'==param_type
#ifndef G__OLDIMPLEMENTATION1248
	 ||'Y'==param_type
#endif
	 ) match=1;
      else  match=0;
      break;
#endif
#ifdef G__WORKAROUND000209_1
      /* reference type conversin should not be handled in this way. 
       * difference was found from g++ when activating this part. */
      /* Added condition for formal_reftype and recursive, then things
       * are working 1999/12/5 */
    case 'u':
      if(G__PARAREFERENCE==formal_reftype && recursive) {
	switch(param_type) {
	case 'u':
	  /* reference to derived class can be converted to reference to base 
	   * class. add offset, modify char *parameter and G__value *param */
	  if(-1 != (baseoffset=G__ispublicbase(formal_tagnum,param_tagnum
					       ,param->obj.i))) {
	    param->tagnum = formal_tagnum;
	    param->obj.i += baseoffset;
	    param->ref += baseoffset;
	    match=1;
	  }
	  else {
	    match=0;
	  }
	  break;
	}
      }
      break;
#endif
    case 'U':
      switch(param_type) {
      case 'U':
	/* Pointer to derived class can be converted to
	 * pointer to base class.
	 * add offset, modify char *parameter and 
	 * G__value *param
	 */
#ifdef G__VIRTUALBASE
	if(-1 != (baseoffset=G__ispublicbase(formal_tagnum,param_tagnum
					     ,param->obj.i))) {
#else
	if(-1 != (baseoffset=G__ispublicbase(formal_tagnum,param_tagnum))) {
#endif
	  param->tagnum = formal_tagnum;
	  param->obj.i += baseoffset;
	  param->ref = 0;
	  match=1;
	}
	else {
	  match=0;
	}
	break;
      case 'Y':
      case 'Q': /* questionable */
	match=1;
	break;
#ifndef G__OLDIMPLEMENTATION764
      case 'i':
      case 0:
	if(0==param->obj.i) match=1;
	else                match=0;
	break;
#endif
      default:
	match=0;
	break;
      }
      break;
    default:
      /* questionable */
#ifndef G__OLDIMPLEMENTATION764
      if((param_type=='Y'||param_type=='Q'||0==param->obj.i)&&
	 (isupper(formal_type)
#ifndef G__OLDIMPLEMENTATION1289
	 || 'a'==formal_type
#endif
	  )) {
#else
      if((param_type=='Y'||param_type=='Q')&&isupper(formal_type)) {
#endif
	match=1;
      }
      else {
	match=0;
      }
      break;
    }
  }

  if(match==0&&funcmatch>=G__USERCONV) {
    if(formal_type=='u' && 0==recursive) {
      /* create temp object buffer */
      if(G__CPPLINK!=G__struct.iscpplink[formal_tagnum]) {
	G__alloc_tempobject(formal_tagnum,-1);
#ifdef G__ASM
	if(G__asm_noverflow) {
#ifdef G__ASM_DBG
	  if(G__asm_dbg) {
	    fprintf(G__serr,"%3x: ALLOCTEMP %s\n"
		    ,G__asm_cp,G__struct.name[formal_tagnum]);
	    fprintf(G__serr,"%3x: SETTEMP\n",G__asm_cp+2);
	  }
#endif
	  G__asm_inst[G__asm_cp] = G__ALLOCTEMP;
	  G__asm_inst[G__asm_cp+1] = formal_tagnum;
	  G__inc_cp_asm(2,0);
	  G__asm_inst[G__asm_cp] = G__SETTEMP;
	  G__inc_cp_asm(1,0);
	}
#endif
      }

      /* try finding constructor */
      if('u'==param_type) {
#ifndef G__OLDIMPLEMENTATION749
	if(param->obj.i<0) 
	  sprintf(arg1,"(%s)(%ld)"
		  ,G__fulltagname(param_tagnum,1),param->obj.i);
	else
	  sprintf(arg1,"(%s)%ld",G__fulltagname(param_tagnum,1),param->obj.i);
#else
	if(param->obj.i<0) 
	  sprintf(arg1,"(%s)(%ld)",G__struct.name[param_tagnum],param->obj.i);
	else
	  sprintf(arg1,"(%s)%ld",G__struct.name[param_tagnum],param->obj.i);
#endif
      }
      else {
	G__valuemonitor(*param,arg1);
      }
      sprintf(conv,"%s(%s)",G__struct.name[formal_tagnum],arg1);

      if(G__dispsource) {
	fprintf(G__serr, "!!!Trying implicit conversion %s,%d\n"
		,conv,G__templevel);
      }

      store_struct_offset = G__store_struct_offset;
      G__store_struct_offset = G__p_tempbuf->obj.obj.i;

      store_tagnum = G__tagnum;
      G__tagnum = formal_tagnum;

      /* avoid duplicated argument evaluation in p-code stack */
      store_oprovld = G__oprovld;
      G__oprovld=1;

#ifdef G__ASM
      if(G__asm_noverflow && rewind_arg) {
	rewindflag=1;
#ifdef G__ASM_DBG
	if(G__asm_dbg) fprintf(G__serr,"%3x: REWINDSTACK %d\n"
			       ,G__asm_cp,rewind_arg);
#endif
	G__asm_inst[G__asm_cp] = G__REWINDSTACK;
	G__asm_inst[G__asm_cp+1] = rewind_arg;
	G__inc_cp_asm(2,0);
      }
#endif

      ++recursive;
      if(G__CPPLINK==G__struct.iscpplink[formal_tagnum]) {
	/* in case of pre-compiled class */
#ifndef G__OLDIMPLEMENTATINO1250
	reg=G__getfunction(conv,&match,G__TRYIMPLICITCONSTRUCTOR);
#else
	reg=G__getfunction(conv,&match,G__TRYCONSTRUCTOR);
#endif
	if(match) {
	  G__store_tempobject(reg);
#ifdef G__ASM
	  if(G__asm_noverflow) {
#ifdef G__ASM_DBG
	    if(G__asm_dbg) fprintf(G__serr,"%3x: STORETEMP\n",G__asm_cp);
#endif
	    G__asm_inst[G__asm_cp]=G__STORETEMP;
	    G__inc_cp_asm(1,0);
	  }
#endif
	}
	else {
#ifndef G__OLDIMPLEMENTATION1018
	  sprintf(conv,"operator %s()",G__fulltagname(formal_tagnum,1));
	  G__store_struct_offset = param->obj.i;
	  G__tagnum = param->tagnum;
	  if(-1!=G__tagnum) reg=G__getfunction(conv,&match,G__TRYMEMFUNC);
	  if(!match) G__store_tempobject(G__null);
#else
	  G__store_tempobject(G__null);
#endif
	}
      }
      else {
	/* in case of interpreted class */
#ifndef G__OLDIMPLEMENTATINO1250
	G__getfunction(conv,&match,G__TRYIMPLICITCONSTRUCTOR);
#else
	G__getfunction(conv,&match,G__TRYCONSTRUCTOR);
#endif
#ifndef G__OLDIMPLEMENTATION1018
	if(match) {
	  if(G__asm_noverflow) {
#ifdef G__ASM_DBG
	    if(G__asm_dbg) fprintf(G__serr,"%3x: POPTEMP\n",G__asm_cp);
#endif
	    G__asm_inst[G__asm_cp] = G__POPTEMP;
	    G__asm_inst[G__asm_cp+1] = formal_tagnum;
	    G__inc_cp_asm(2,0);
	  }
	}
	else {
	  if(G__asm_noverflow) G__inc_cp_asm(-3,0);
	  sprintf(conv,"operator %s()",G__fulltagname(formal_tagnum,1));
	  G__store_struct_offset = param->obj.i;
	  G__tagnum = param->tagnum;
	  reg=G__getfunction(conv,&match,G__TRYMEMFUNC);
	  if(!match) {
	    if(G__asm_noverflow) {
	      if(rewindflag) {
		G__asm_inst[G__asm_cp-2]=G__REWINDSTACK; 
		G__asm_inst[G__asm_cp-1] = rewind_arg;
	      }
#ifdef G__ASM_DBG
	      if(G__asm_dbg) 
		fprintf(G__serr,"ALLOCTEMP,SETTEMP Cancelled %x\n",G__asm_cp);
#endif
	    }
	  }
	}
#else /* ON1018 */
#ifdef G__ASM
	if(G__asm_noverflow) {
	  if(match) {
#ifdef G__ASM_DBG
	    if(G__asm_dbg) fprintf(G__serr,"%3x: POPTEMP\n",G__asm_cp);
#endif
	    G__asm_inst[G__asm_cp] = G__POPTEMP;
	    G__asm_inst[G__asm_cp+1] = formal_tagnum;
	    G__inc_cp_asm(2,0);
	  }
	  else {
	    G__inc_cp_asm(-3,0);
	    if(rewindflag) {
	      G__asm_inst[G__asm_cp-2]=G__REWINDSTACK; 
	      G__asm_inst[G__asm_cp-1] = rewind_arg;
	    }
#ifdef G__ASM_DBG
	    if(G__asm_dbg) 
	      fprintf(G__serr,"ALLOCTEMP,SETTEMP Cancelled %x\n",G__asm_cp);
#endif
	  }
	}
#endif
#endif /* ON1018 */
      }
      --recursive;

      G__oprovld=store_oprovld;

      G__tagnum = store_tagnum;
      G__store_struct_offset = store_struct_offset;

      /* if no constructor, try converting to base class */


      if(match==0) {
	if('u'==param_type &&
#ifdef G__VIRTUALBASE
	   -1 != (baseoffset=G__ispublicbase(formal_tagnum,param_tagnum
					     ,param->obj.i))) {
#else
	   -1 != (baseoffset=G__ispublicbase(formal_tagnum,param_tagnum))) {
#endif
	  if(G__dispsource) {
	    fprintf(G__serr, "!!!Implicit conversion from %s to base %s\n"
		    ,G__struct.name[param_tagnum]
		    ,G__struct.name[formal_tagnum]);
	  }
	  param->typenum = -1;
	  param->tagnum = formal_tagnum;
	  param->obj.i += baseoffset;
	  param->ref += baseoffset;
#ifdef G__ASM
	  if(G__asm_noverflow) {
#ifdef G__ASM_DBG
	    if(G__asm_dbg) fprintf(G__serr,"%3x: BASECONV %d %d\n"
				   ,G__asm_cp,formal_tagnum,baseoffset);
#endif
	    G__asm_inst[G__asm_cp] = G__BASECONV;
	    G__asm_inst[G__asm_cp+1] = formal_tagnum;
	    G__asm_inst[G__asm_cp+2] = baseoffset;
	    G__inc_cp_asm(3,0);
	    if(rewind_arg) {
	      rewindflag=1;
#ifdef G__ASM_DBG
	      if(G__asm_dbg) fprintf(G__serr,"%3x: REWINDSTACK %d\n"
				     ,G__asm_cp,-rewind_arg);
#endif
	      G__asm_inst[G__asm_cp] = G__REWINDSTACK;
	      G__asm_inst[G__asm_cp+1] = -rewind_arg;
	      G__inc_cp_asm(2,0);
	    }
#endif
	    if(param->obj.i<0) 
	      sprintf(parameter,"(%s)(%ld)",G__struct.name[formal_tagnum]
		      ,param->obj.i);
	    else
	      sprintf(parameter,"(%s)%ld",G__struct.name[formal_tagnum]
		      ,param->obj.i);
	  }
	  match=1;
	  G__pop_tempobject();
	}
	else { /* all conversion failed */
	  if(G__dispsource) {
	    fprintf(G__serr,
		    "!!!Implicit conversion %s,%d tried, but failed\n"
		    ,conv,G__templevel);
	  }
	  G__pop_tempobject();
#ifdef G__ASM
	  if(rewindflag) {
#ifdef G__ASM_DBG
	    if(G__asm_dbg) fprintf(G__serr,"REWINDSTACK cancelled\n");
#endif
	    G__inc_cp_asm(-2,0);
	  }
	}

#else /* ON181 */

	/* all conversion failed */
	if(G__dispsource) {
	  fprintf(G__serr,
		  "!!!Implicit conversion %s,%d tried, but failed\n"
		  ,conv,G__templevel);
	}
	G__pop_tempobject();
#ifdef G__ASM
	if(rewindflag) {
#ifdef G__ASM_DBG
	  if(G__asm_dbg) fprintf(G__serr,"REWINDSTACK cancelled\n");
#endif
	  G__inc_cp_asm(-2,0);
	}
#endif

#endif /* ON181 */
      }
      else { /* match==1, conversion successful */
	if(G__dispsource) {
	  if(G__p_tempbuf->obj.obj.i<0) 
	    fprintf(G__serr,
	      "!!!Create temp object (%s)(%ld),%d for implicit conversion\n"
		    ,conv ,G__p_tempbuf->obj.obj.i ,G__templevel);
	  else
	    fprintf(G__serr,
		  "!!!Create temp object (%s)%ld,%d for implicit conversion\n"
		    ,conv ,G__p_tempbuf->obj.obj.i ,G__templevel);
	}
#ifdef G__ASM
	if(G__asm_noverflow && rewind_arg) {
	  rewindflag=1;
#ifdef G__ASM_DBG
	  if(G__asm_dbg) fprintf(G__serr,"%3x: REWINDSTACK %d\n"
				 ,G__asm_cp,-rewind_arg);
#endif
	  G__asm_inst[G__asm_cp] = G__REWINDSTACK;
	  G__asm_inst[G__asm_cp+1] = -rewind_arg;
	  G__inc_cp_asm(2,0);
	}
#endif
	*param = G__p_tempbuf->obj;
        sprintf(parameter,"(%s)%ld" ,G__struct.name[formal_tagnum]
		,G__p_tempbuf->obj.obj.i);
      } /* end of if(match==0) */

    }
#ifndef G__OLDIMPLEMENTATION1077
    else if(-1!=param->tagnum) {
      long store_struct_offset=G__store_struct_offset;
      int store_tagnum=G__tagnum;
      sprintf(conv,"operator %s()"
	      ,G__type2string(formal_type,formal_tagnum,-1,0,0));
      G__store_struct_offset = param->obj.i;
      G__tagnum=param->tagnum;
#ifndef G__OLDIMPLEMENTATION1130
#ifdef G__ASM
      if(G__asm_noverflow) {
	if(rewind_arg) {
	  rewindflag=1;
#ifdef G__ASM_DBG
	  if(G__asm_dbg) fprintf(G__serr,"%3x: REWINDSTACK %d\n"
			         ,G__asm_cp,rewind_arg);
#endif
	  G__asm_inst[G__asm_cp] = G__REWINDSTACK;
	  G__asm_inst[G__asm_cp+1] = rewind_arg;
	  G__inc_cp_asm(2,0);
        }
        G__asm_inst[G__asm_cp] = G__PUSHSTROS;
        G__asm_inst[G__asm_cp+1] = G__SETSTROS;
        G__inc_cp_asm(2,0);
#ifdef G__ASM_DBG
        if(G__asm_dbg) {
          fprintf(G__serr,"%3x: PUSHSTROS\n",G__asm_cp-2);
          fprintf(G__serr,"%3x: SETSTROS\n",G__asm_cp-1);
        }
#endif
      }
#endif
#endif
      reg=G__getfunction(conv,&match,G__TRYMEMFUNC);
#ifndef G__OLDIMPLEMENTATION1124
      if(!match
#ifndef G__OLDIMPLEMENTATION1208
	 && 0!=formal_isconst
#endif
	 ) {
	sprintf(conv,"operator const %s()"
		,G__type2string(formal_type,formal_tagnum,-1,0,0));
	G__store_struct_offset = param->obj.i;
	G__tagnum=param->tagnum;
	reg=G__getfunction(conv,&match,G__TRYMEMFUNC);
      }
#endif
      G__tagnum=store_tagnum;
      G__store_struct_offset=store_struct_offset;
#ifndef G__OLDIMPLEMENTATION1130
#ifdef G__ASM
      if(G__asm_noverflow) {
        if(rewind_arg) {
	  rewindflag=1;
#ifdef G__ASM_DBG
	  if(G__asm_dbg) fprintf(G__serr,"%3x: REWINDSTACK %d\n"
	 		         ,G__asm_cp,-rewind_arg);
#endif
	  G__asm_inst[G__asm_cp] = G__REWINDSTACK;
	  G__asm_inst[G__asm_cp+1] = -rewind_arg;
	  G__inc_cp_asm(2,0);
        }
        G__asm_inst[G__asm_cp] = G__POPSTROS;
        G__inc_cp_asm(1,0);
#ifdef G__ASM_DBG
        if(G__asm_dbg) fprintf(G__serr,"%3x: POPSTROS\n",G__asm_cp-1);
#endif
      }
#endif
#endif
#ifndef G__OLDIMPLEMENTATION1129
      /* fixing 'cout<<x' fundamental conversion opr with opr overloading 
      * Not 100% sure if this is OK. */
      if(match) *param = reg;
      else if(rewindflag) {
#ifdef G__ASM_DBG
        if(G__asm_dbg) fprintf(G__serr,"REWINDSTACK~ cancelled\n");
#endif
        G__inc_cp_asm(-7,0);
      }
      else {
#ifdef G__ASM_DBG
        if(G__asm_dbg) fprintf(G__serr,"PUSHSTROS~ cancelled\n");
#endif
        G__inc_cp_asm(-3,0);
      }
#endif
    }
#endif
    else {
      match=0;
/* #ifdef G__DEBUG */
      if(recursive&&G__dispsource) {
	G__valuemonitor(*param,arg1);
	fprintf(G__serr ,"!!!Recursive implicit conversion %s(%s) rejected\n"
		,G__struct.name[formal_tagnum],arg1);
      }
/* #endif */
    }
  }

#ifndef G__OLDIMPLEMENTATION1120
  if(match && isupper(param_type) && isupper(formal_type) && 
     'Y'!=param_type && 'Y'!=formal_type
#ifndef G__OLDIMPLEMENTATION1266
     && 'Q'!=param_type 
#endif
     ) {
    match=G__matchpointlevel(param->obj.reftype.reftype,formal_reftype);
  }
#endif

  return(match);
}

#ifndef G__OLDIMPLEMENTATION1290
/***********************************************************************
* macro 
**********************************************************************/
#define G__NOMATCH        0xffffffff
#define G__EXACTMATCH     0x00000000
#define G__PROMOTIONMATCH 0x00000100
#define G__STDCONVMATCH   0x00010000
#define G__USRCONVMATCH   0x01000000
#define G__CVCONVMATCH    0x00000001
#define G__BASECONVMATCH  0x00000001
#define G__C2P2FCONVMATCH 0x00000001
#define G__V2P2FCONVMATCH 0x00000002

/***********************************************************************
* struct G__overload_func
**********************************************************************/
struct G__funclist {
  struct G__ifunc_table *ifunc;
  int ifn;
  unsigned int rate;
  unsigned int p_rate[G__MAXFUNCPARA];
  struct G__funclist *prev;
};

struct G__funclist* G__funclist_add(last,ifunc,ifn,rate)
struct G__funclist *last;
struct G__ifunc_table *ifunc;
int ifn;
int rate;
{
  struct G__funclist *latest = 
    (struct G__funclist*)malloc(sizeof(struct G__funclist));
  latest->prev = last;
  latest->ifunc = ifunc;
  latest->ifn = ifn;
  latest->rate = rate;
  return(latest);
}

void G__funclist_delete(body)
struct G__funclist *body;
{
  if(body) {
    if(body->prev) G__funclist_delete(body->prev);
    free((void*)body);
  }
}

/***********************************************************************
* G__rate_inheritance()
***********************************************************************/
unsigned int G__rate_inheritance(basetagnum,derivedtagnum)
{
  struct G__inheritance *derived;
  int i,n;

  if(0>derivedtagnum||0>basetagnum) return(G__NOMATCH);
  if(basetagnum==derivedtagnum) return(G__EXACTMATCH);
  derived = G__struct.baseclass[derivedtagnum];
  n = derived->basen;

  for(i=0;i<n;i++) {
    if(basetagnum == derived->basetagnum[i]) {
      if(derived->baseaccess[i]==G__PUBLIC ||
	 (G__exec_memberfunc && G__tagnum==derivedtagnum &&
	  G__GRANDPRIVATE!=derived->baseaccess[i])) {
	if(G__ISDIRECTINHERIT&derived->property[i]) {
	  return(G__BASECONVMATCH);
	}
	else {
#ifndef G__OLDIMPLEMENTATION1302
	  int distance = 1;
	  int ii=i; /* i is not 0, because !G__ISDIRECTINHERIT */
	  struct G__inheritance *derived2 = derived;
	  int derivedtagnum2 = derivedtagnum;
	  while(0==(derived2->property[ii]&G__ISDIRECTINHERIT)) {
	    ++distance;
	    while(ii && 0==(derived2->property[--ii]&G__ISDIRECTINHERIT));
	    derivedtagnum2 = derived2->basetagnum[ii];
	    derived2 = G__struct.baseclass[derivedtagnum2];
	    for(ii=0;ii<derived2->basen;ii++) {
	      if(derived2->basetagnum[ii]==basetagnum) break;
	    }
	    if(ii==derived2->basen) return(G__NOMATCH);
	  }
#else
	  int distance = 1;
	  int ii=i; /* i is not 0, because !G__ISDIRECTINHERIT */
	  struct G__inheritance *derived2 = derived;
	  int basetagnum2 = basetagnum;
	  while(0==(derived2->property[ii]&G__ISDIRECTINHERIT)) {
	    ++distance;
	    while(ii && 0==(derived->property[--ii]&G__ISDIRECTINHERIT));
	    basetagnum2 = derived->basetagnum[ii];
	    derived2 = G__struct.baseclass[basetagnum2];
	    for(ii=0;ii<derived2->basen;ii++) {
	      if(derived2->basetagnum[ii]==derivedtagnum) break;
	    }
	  }
#endif
	  return(distance*G__BASECONVMATCH);
	}
      }
    }
  }
  return(G__NOMATCH);
}

#ifndef __CINT__
struct G__ifunc_table* G__overload_match G__P((char* funcname,struct G__param *libp,int hash,struct G__ifunc_table *p_ifunc,int memfunc_flag,int access,int *pifn,int recursive)) ;
#endif

/***********************************************************************
* int G__rate_parameter_match(libp,ifunc,ifn,i)
**********************************************************************/
void G__rate_parameter_match(libp,p_ifunc,ifn,funclist,recursive)
struct G__param *libp;
struct G__ifunc_table *p_ifunc; 
int ifn;
struct G__funclist *funclist;
int recursive;
{
  int i;
  char param_type,formal_type;
  int param_tagnum,formal_tagnum;
  int param_reftype,formal_reftype;
  funclist->rate = 0;
  for(i=0;i<libp->paran;i++) {
    param_type = libp->para[i].type;
    formal_type = p_ifunc->para_type[ifn][i];
    param_tagnum = libp->para[i].tagnum;
    formal_tagnum = p_ifunc->para_p_tagtable[ifn][i];
    param_reftype = libp->para[i].obj.reftype.reftype;
    formal_reftype = p_ifunc->para_reftype[ifn][i];
    funclist->p_rate[i] = G__NOMATCH;

    /* exact match */
    if(param_type==formal_type){
      if(tolower(param_type)=='u') {
	/* If struct,class,union, check tagnum */
	if(formal_tagnum == param_tagnum) { /* match */
	  funclist->p_rate[i] = G__EXACTMATCH;
	}
      }
      else if(isupper(param_type)) {
	if(param_reftype==formal_reftype ||
	   (param_reftype<=G__PARAREFERENCE &&
	    formal_reftype<=G__PARAREFERENCE)) {
	  funclist->p_rate[i] = G__EXACTMATCH;
	}
      }
      else { /* match */
	funclist->p_rate[i] = G__EXACTMATCH;
      }
    }
    else if('I'==param_type&&'U'==formal_type&&param_tagnum==formal_tagnum&&
	    -1!=formal_tagnum&&'e'==G__struct.type[formal_tagnum]) {
      funclist->p_rate[i] = G__EXACTMATCH;
    }
#ifndef G__OLDIMPLEMENTATION1319
    else if(isupper(formal_type)&&'i'==param_type&&0==libp->para[i].obj.i) {
      funclist->p_rate[i] = G__STDCONVMATCH;
    }
#endif
    
    /* promotion */
    if(G__NOMATCH==funclist->p_rate[i]) {
      switch(formal_type) {
      case 'd': 
      case 'f':
	switch(param_type) {
	case 'b':
	case 'c':
	case 'r':
	case 's':
	case 'h':
	case 'i':
	case 'k':
	case 'l':
	case 'd':
	case 'f':
	  funclist->p_rate[i] = G__PROMOTIONMATCH;
	  break;
	default:
	  break;
	}
	break;
      case 'l':
	switch(param_type) {
	case 'b':
	case 'c':
	case 'r':
	case 's':
	  /* case 'h': */
	case 'i':
	  /* case 'k': */
	case 'l':
	  funclist->p_rate[i] = G__PROMOTIONMATCH;
	  break;
	default:
	break;
	}
	break;
      case 'i':
	switch(param_type) {
	case 'b':
	case 'c':
	case 'r':
	case 's':
	  /* case 'h': */
	case 'i':
	  /* case 'k': */
	  /* case 'l': */
	  funclist->p_rate[i] = G__PROMOTIONMATCH;
	  break;
	case 'u':
	  if('e'==G__struct.type[param_tagnum]) {
	    funclist->p_rate[i] = G__PROMOTIONMATCH;
	    break;
	  }
	default:
	  break;
	}
	break;
      case 's':
	switch(param_type) {
	case 'b':
	case 'c':
	  /* case 'r': */
	case 's':
	  /* case 'h': */
	  /* case 'i': */
	  /* case 'k': */
	  /* case 'l': */
	  funclist->p_rate[i] = G__PROMOTIONMATCH;
	  break;
	default:
	  break;
	}
	break;
      case 'k':
	switch(param_type) {
	case 'b':
	  /* case 'c': */
	case 'r':
	  /* case 's': */
	case 'h':
	  /* case 'i': */
	case 'k':
	  /* case 'l': */
	  funclist->p_rate[i] = G__PROMOTIONMATCH;
	  break;
	default:
	  break;
	}
	break;
      case 'h':
	switch(param_type) {
	case 'b':
	  /* case 'c': */
	case 'r':
	  /* case 's': */
	case 'h':
	  /* case 'i': */
	  /* case 'k': */
	  /* case 'l': */
	  funclist->p_rate[i] = G__PROMOTIONMATCH;
	  break;
	default:
	  break;
	}
	break;
      case 'r':
	switch(param_type) {
	case 'b':
	  /* case 'c': */
	case 'r':
	  /* case 's': */
	  /* case 'h': */
	  /* case 'i': */
	  /* case 'k': */
	  /* case 'l': */
	  funclist->p_rate[i] = G__PROMOTIONMATCH;
	  break;
	default:
	  break;
	}
	break;
      case 'u':
	if(0<=formal_tagnum && 'e'==G__struct.type[formal_tagnum]) {
	  switch(param_type) {
	  case 'i':
	  case 's':
	  case 'l':
	  case 'c':
	  case 'h':
	  case 'r':
	  case 'k':
	  case 'b':
	    funclist->p_rate[i] = G__PROMOTIONMATCH;
	    break;
	  default:
	    break;
	  }
	}
	else {
	}
	break;
      default:
	break;
      }
    }

    /* standard conversion */
    if(G__NOMATCH==funclist->p_rate[i]) {
      switch(formal_type) {
      case 'b':
      case 'c':
      case 'r':
      case 's':
      case 'h':
      case 'i':
      case 'k':
      case 'l':
	switch(param_type) {
	case 'd':
	case 'f':
	case 'b':
	case 'c':
	case 'r':
	case 's':
	case 'h':
	case 'i':
	case 'k':
	case 'l':
	  funclist->p_rate[i] = G__STDCONVMATCH;
	  break;
	default:
	  break;
	}
	break;
      case 'd':
      case 'f':
	switch(param_type) {
	case 'b':
	case 'c':
	case 'r':
	case 's':
	case 'h':
	case 'i':
	case 'k':
	case 'l':
	case 'd':
	case 'f':
	  funclist->p_rate[i] = G__STDCONVMATCH;
	  break;
	default:
	  break;
	}
	break;
      case 'C':
	switch(param_type) {
	case 'i':
	case 'l':
#ifndef G__OLDIMPLEMENTATION839
	  if(0==libp->para[i].obj.i) funclist->p_rate[i] = G__STDCONVMATCH;
	  break;
#endif
	case 'Y':
#ifdef G__OLDIMPLEMENTATION1219
	case 'Q': /* questionable */
#endif
	  funclist->p_rate[i] = G__STDCONVMATCH;
	  break;
	default:
	  break;
	}
	break;
      case 'Y':
#ifdef G__OLDIMPLEMENTATION1219
      case 'Q': /* questionable */
#endif
	if(isupper(param_type)||0==libp->para[i].obj.i) {
	  funclist->p_rate[i] = G__STDCONVMATCH;
	}
	break;
#ifndef G__OLDIMPLEMENTATION1225
      case 'Q': /* questionable */
#ifndef G__OLDIMPLEMENTATION1299
	if('Q'==param_type) 
	  funclist->p_rate[i] = G__STDCONVMATCH;
	else if('Y'==param_type) 
	  funclist->p_rate[i] = G__STDCONVMATCH+G__V2P2FCONVMATCH;
	else if('C'==param_type) {
	  if(p_ifunc->pentry[ifn]->filenum>=0) 
	    funclist->p_rate[i] = G__STDCONVMATCH-G__C2P2FCONVMATCH;
	  else
	    funclist->p_rate[i] = G__STDCONVMATCH+G__C2P2FCONVMATCH;
	}
	
#else
	if('Q'==param_type||'C'==param_type
#ifndef G__OLDIMPLEMENTATION1248
	   ||'Y'==param_type
#endif
	   ) funclist->p_rate[i] = G__STDCONVMATCH;
#endif
	break;
#endif
      case 'u':
	switch(param_type) {
	case 'u':
	  /* reference to derived class can be converted to reference to base 
	   * class. add offset, modify char *parameter and G__value *param */
	  {
	    int rate_inheritance = 
	      G__rate_inheritance(formal_tagnum,param_tagnum);
	    if(G__NOMATCH!=rate_inheritance) {
	      funclist->p_rate[i] = G__STDCONVMATCH+rate_inheritance;
	    }
	  }
	  break;
	}
	break;
      case 'U':
	switch(param_type) {
	case 'U':
	  /* Pointer to derived class can be converted to
	   * pointer to base class.
	   * add offset, modify char *parameter and 
	   * G__value *param
	   */
	  {
	    int rate_inheritance = 
	      G__rate_inheritance(formal_tagnum,param_tagnum);
	    if(G__NOMATCH!=rate_inheritance) {
	      funclist->p_rate[i] = G__STDCONVMATCH+rate_inheritance;
	    }
	  }
	  break;
	case 'Y':
	case 'Q': /* questionable */
	  funclist->p_rate[i] = G__STDCONVMATCH;
	  break;
#ifndef G__OLDIMPLEMENTATION764
	case 'i':
	case 0:
	  if(0==libp->para[0].obj.i) funclist->p_rate[i] = G__STDCONVMATCH;
	  break;
#endif
	default:
	  break;
	}
	break;
      default:
	/* questionable */
	if((param_type=='Y'||param_type=='Q'||0==libp->para[0].obj.i)&&
	   (isupper(formal_type)
	    || 'a'==formal_type
	    )) {
	  funclist->p_rate[i] = G__STDCONVMATCH;
	}
	break;
      }
    }

    /* user defined conversion */
    if(0==recursive && G__NOMATCH==funclist->p_rate[i]) {
      if(formal_type=='u') {
	struct G__ifunc_table *ifunc2;
	int ifn2;
	int hash2;
	char funcname2[G__ONELINE];
	struct G__param para;
	G__incsetup_memfunc(formal_tagnum);
	ifunc2 = G__struct.memfunc[formal_tagnum];
	para.paran = 1;
	para.para[0] = libp->para[i];
	strcpy(funcname2,G__struct.name[formal_tagnum]);
	G__hash(funcname2,hash2,ifn2);
	ifunc2 = G__overload_match(funcname2,&para,hash2,ifunc2
				    ,G__TRYCONSTRUCTOR,G__PUBLIC,&ifn2,1);
	if(ifunc2 && -1!=ifn2) 
	  funclist->p_rate[i] = G__USRCONVMATCH;
      }
    }

    if(0==recursive && G__NOMATCH==funclist->p_rate[i]) {
      if(param_type=='u') {
	struct G__ifunc_table *ifunc2;
	int ifn2;
	int hash2;
	char funcname2[G__ONELINE];
	struct G__param para;
	G__incsetup_memfunc(param_tagnum);
	ifunc2 = G__struct.memfunc[param_tagnum];
	para.paran = 0;
#ifndef G__OLDIMPLEMENTATION1316
	sprintf(funcname2,"operator %s"
		,G__type2string(formal_type,formal_tagnum,-1,0,0));
#else
	sprintf(funcname2,"operator %s"
		,G__type2string(param_type,param_tagnum,-1,0,0));
#endif
	G__hash(funcname2,hash2,ifn2);
	ifunc2 = G__overload_match(funcname2,&para,hash2,ifunc2
				   ,G__TRYMEMFUNC,G__PUBLIC,&ifn2,1);
#ifndef G__OLDIMPLEMENTATION1316
	if(!ifunc2) {
	  sprintf(funcname2,"operator %s"
		  ,G__type2string(formal_type,formal_tagnum,-1,0,1));
	  G__hash(funcname2,hash2,ifn2);
	  ifunc2 = G__struct.memfunc[param_tagnum];
	  ifunc2 = G__overload_match(funcname2,&para,hash2,ifunc2
				     ,G__TRYMEMFUNC,G__PUBLIC,&ifn2,1);
	}
#endif
	if(ifunc2 && -1!=ifn2) 
	  funclist->p_rate[i] = G__USRCONVMATCH;
      }
    }

    /* add up matching rate */
    if(G__NOMATCH==funclist->p_rate[i]) {
      funclist->rate = G__NOMATCH;
      break;
    }
    else
      funclist->rate += funclist->p_rate[i];
  }
#ifndef G__OLDIMPLEMENTATION1260
  if(G__NOMATCH!=funclist->rate && ((0==G__isconst && p_ifunc->isconst[ifn])
     || (G__isconst && 0==p_ifunc->isconst[ifn]))
     )
    funclist->rate += G__CVCONVMATCH;
#endif
}

/***********************************************************************
* int G__convert_param(libp,p_ifunc,ifn,i)
**********************************************************************/
void G__convert_param(libp,p_ifunc,ifn,pmatch)
struct G__param *libp;
struct G__ifunc_table *p_ifunc; 
int ifn;
struct G__funclist *pmatch;
{
  int i;
  unsigned int rate;
  char param_type,formal_type;
  int param_tagnum,formal_tagnum;
  int formal_reftype;
  int formal_isconst;
  G__value *param;

  char conv[G__ONELINE],arg1[G__ONELINE],parameter[G__ONELINE];
  long store_struct_offset; /* used to be int */
  int store_tagnum;
  int baseoffset;
  G__value reg;
  int store_oprovld;
  int rewindflag=0;
  int recursive =0 ;
  int rewind_arg;
  int match = 0;

  for(i=0;i<libp->paran;i++) {
    rate = pmatch->p_rate[i];
    param_type = libp->para[i].type;
    formal_type = p_ifunc->para_type[ifn][i];
    param_tagnum = libp->para[i].tagnum;
    formal_tagnum = p_ifunc->para_p_tagtable[ifn][i];
    param = &libp->para[i];
    formal_reftype = p_ifunc->para_reftype[ifn][i];
    rewind_arg = p_ifunc->para_nu[ifn]-i-1;
    formal_isconst = p_ifunc->para_isconst[ifn][i];

    if(rate&G__USRCONVMATCH) {
      if(formal_type=='u') {
	/* create temp object buffer */
	if(G__CPPLINK!=G__struct.iscpplink[formal_tagnum]) {
	  G__alloc_tempobject(formal_tagnum,-1);
#ifdef G__ASM
	  if(G__asm_noverflow) {
#ifdef G__ASM_DBG
	    if(G__asm_dbg) {
	      fprintf(G__serr,"%3x: ALLOCTEMP %s\n"
		      ,G__asm_cp,G__struct.name[formal_tagnum]);
	      fprintf(G__serr,"%3x: SETTEMP\n",G__asm_cp+2);
	    }
#endif
	    G__asm_inst[G__asm_cp] = G__ALLOCTEMP;
	    G__asm_inst[G__asm_cp+1] = formal_tagnum;
	    G__inc_cp_asm(2,0);
	    G__asm_inst[G__asm_cp] = G__SETTEMP;
	    G__inc_cp_asm(1,0);
	  }
#endif
	}
	
	/* try finding constructor */
	if('u'==param_type) {
#ifndef G__OLDIMPLEMENTATION749
	  if(param->obj.i<0) 
	    sprintf(arg1,"(%s)(%ld)"
		    ,G__fulltagname(param_tagnum,1),param->obj.i);
	  else
	    sprintf(arg1,"(%s)%ld",G__fulltagname(param_tagnum,1),param->obj.i);
#else
	  if(param->obj.i<0) 
	    sprintf(arg1,"(%s)(%ld)",G__struct.name[param_tagnum],param->obj.i);
	  else
	    sprintf(arg1,"(%s)%ld",G__struct.name[param_tagnum],param->obj.i);
#endif
	}
	else {
	  G__valuemonitor(*param,arg1);
	}
	sprintf(conv,"%s(%s)",G__struct.name[formal_tagnum],arg1);
	
	if(G__dispsource) {
	  fprintf(G__serr, "!!!Trying implicit conversion %s,%d\n"
		  ,conv,G__templevel);
	}
	
	store_struct_offset = G__store_struct_offset;
	G__store_struct_offset = G__p_tempbuf->obj.obj.i;
	
	store_tagnum = G__tagnum;
	G__tagnum = formal_tagnum;
	
	/* avoid duplicated argument evaluation in p-code stack */
	store_oprovld = G__oprovld;
	G__oprovld=1;
	
#ifdef G__ASM
	if(G__asm_noverflow && rewind_arg) {
	  rewindflag=1;
#ifdef G__ASM_DBG
	  if(G__asm_dbg) fprintf(G__serr,"%3x: REWINDSTACK %d\n"
				 ,G__asm_cp,rewind_arg);
#endif
	  G__asm_inst[G__asm_cp] = G__REWINDSTACK;
	  G__asm_inst[G__asm_cp+1] = rewind_arg;
	  G__inc_cp_asm(2,0);
	}
#endif
	
	++recursive;
	if(G__CPPLINK==G__struct.iscpplink[formal_tagnum]) {
	  /* in case of pre-compiled class */
#ifndef G__OLDIMPLEMENTATINO1250
	  reg=G__getfunction(conv,&match,G__TRYIMPLICITCONSTRUCTOR);
#else
	  reg=G__getfunction(conv,&match,G__TRYCONSTRUCTOR);
#endif
	  if(match) {
	    G__store_tempobject(reg);
#ifdef G__ASM
	    if(G__asm_noverflow) {
#ifdef G__ASM_DBG
	      if(G__asm_dbg) fprintf(G__serr,"%3x: STORETEMP\n",G__asm_cp);
#endif
	      G__asm_inst[G__asm_cp]=G__STORETEMP;
	      G__inc_cp_asm(1,0);
	    }
#endif
	  }
	  else {
#ifndef G__OLDIMPLEMENTATION1018
	    sprintf(conv,"operator %s()",G__fulltagname(formal_tagnum,1));
	    G__store_struct_offset = param->obj.i;
	    G__tagnum = param->tagnum;
	    if(-1!=G__tagnum) reg=G__getfunction(conv,&match,G__TRYMEMFUNC);
	    if(!match) G__store_tempobject(G__null);
#else
	    G__store_tempobject(G__null);
#endif
	  }
	}
	else {
	  /* in case of interpreted class */
#ifndef G__OLDIMPLEMENTATINO1250
	  G__getfunction(conv,&match,G__TRYIMPLICITCONSTRUCTOR);
#else
	  G__getfunction(conv,&match,G__TRYCONSTRUCTOR);
#endif
#ifndef G__OLDIMPLEMENTATION1018
	  if(match) {
	    if(G__asm_noverflow) {
#ifdef G__ASM_DBG
	      if(G__asm_dbg) fprintf(G__serr,"%3x: POPTEMP\n",G__asm_cp);
#endif
	      G__asm_inst[G__asm_cp] = G__POPTEMP;
	      G__asm_inst[G__asm_cp+1] = formal_tagnum;
	      G__inc_cp_asm(2,0);
	    }
	  }
	  else {
	    if(G__asm_noverflow) G__inc_cp_asm(-3,0);
	    sprintf(conv,"operator %s()",G__fulltagname(formal_tagnum,1));
	    G__store_struct_offset = param->obj.i;
	    G__tagnum = param->tagnum;
	    reg=G__getfunction(conv,&match,G__TRYMEMFUNC);
	    if(!match) {
	      if(G__asm_noverflow) {
		if(rewindflag) {
		  G__asm_inst[G__asm_cp-2]=G__REWINDSTACK; 
		  G__asm_inst[G__asm_cp-1] = rewind_arg;
		}
#ifdef G__ASM_DBG
		if(G__asm_dbg) 
		  fprintf(G__serr,"ALLOCTEMP,SETTEMP Cancelled %x\n",G__asm_cp);
#endif
	      }
	    }
	  }
#else /* ON1018 */
#ifdef G__ASM
	  if(G__asm_noverflow) {
	    if(match) {
#ifdef G__ASM_DBG
	      if(G__asm_dbg) fprintf(G__serr,"%3x: POPTEMP\n",G__asm_cp);
#endif
	      G__asm_inst[G__asm_cp] = G__POPTEMP;
	      G__asm_inst[G__asm_cp+1] = formal_tagnum;
	      G__inc_cp_asm(2,0);
	    }
	    else {
	      G__inc_cp_asm(-3,0);
	      if(rewindflag) {
		G__asm_inst[G__asm_cp-2]=G__REWINDSTACK; 
		G__asm_inst[G__asm_cp-1] = rewind_arg;
	      }
#ifdef G__ASM_DBG
	    if(G__asm_dbg) 
	      fprintf(G__serr,"ALLOCTEMP,SETTEMP Cancelled %x\n",G__asm_cp);
#endif
	    }
	  }
#endif
#endif /* ON1018 */
	}
	--recursive;
	
	G__oprovld=store_oprovld;
	
	G__tagnum = store_tagnum;
	G__store_struct_offset = store_struct_offset;
	
	/* if no constructor, try converting to base class */
	
	
	if(match==0) {
	  if('u'==param_type &&
#ifdef G__VIRTUALBASE
	     -1 != (baseoffset=G__ispublicbase(formal_tagnum,param_tagnum
					       ,param->obj.i))) {
#else
	     -1 != (baseoffset=G__ispublicbase(formal_tagnum,param_tagnum))) {
#endif
	    if(G__dispsource) {
	      fprintf(G__serr, "!!!Implicit conversion from %s to base %s\n"
		      ,G__struct.name[param_tagnum]
		      ,G__struct.name[formal_tagnum]);
	    }
	    param->typenum = -1;
	    param->tagnum = formal_tagnum;
	    param->obj.i += baseoffset;
	    param->ref += baseoffset;
#ifdef G__ASM
	    if(G__asm_noverflow) {
#ifdef G__ASM_DBG
	      if(G__asm_dbg) fprintf(G__serr,"%3x: BASECONV %d %d\n"
				   ,G__asm_cp,formal_tagnum,baseoffset);
#endif
	      G__asm_inst[G__asm_cp] = G__BASECONV;
	      G__asm_inst[G__asm_cp+1] = formal_tagnum;
	      G__asm_inst[G__asm_cp+2] = baseoffset;
	      G__inc_cp_asm(3,0);
	      if(rewind_arg) {
		rewindflag=1;
#ifdef G__ASM_DBG
		if(G__asm_dbg) fprintf(G__serr,"%3x: REWINDSTACK %d\n"
				       ,G__asm_cp,-rewind_arg);
#endif
		G__asm_inst[G__asm_cp] = G__REWINDSTACK;
		G__asm_inst[G__asm_cp+1] = -rewind_arg;
		G__inc_cp_asm(2,0);
	      }
#endif
	      if(param->obj.i<0) 
		sprintf(parameter,"(%s)(%ld)",G__struct.name[formal_tagnum]
			,param->obj.i);
	      else
		sprintf(parameter,"(%s)%ld",G__struct.name[formal_tagnum]
			,param->obj.i);
	    }
	    match=1;
	    G__pop_tempobject();
					       }
	  else { /* all conversion failed */
	    if(G__dispsource) {
	      fprintf(G__serr,
		      "!!!Implicit conversion %s,%d tried, but failed\n"
		      ,conv,G__templevel);
	    }
	    G__pop_tempobject();
#ifdef G__ASM
	    if(rewindflag) {
#ifdef G__ASM_DBG
	      if(G__asm_dbg) fprintf(G__serr,"REWINDSTACK cancelled\n");
#endif
	      G__inc_cp_asm(-2,0);
	    }
	  }
	  
#else /* ON181 */
	  
	  /* all conversion failed */
	  if(G__dispsource) {
	    fprintf(G__serr,
		    "!!!Implicit conversion %s,%d tried, but failed\n"
		    ,conv,G__templevel);
	  }
	  G__pop_tempobject();
#ifdef G__ASM
	  if(rewindflag) {
#ifdef G__ASM_DBG
	    if(G__asm_dbg) fprintf(G__serr,"REWINDSTACK cancelled\n");
#endif
	    G__inc_cp_asm(-2,0);
	  }
#endif
	  
#endif /* ON181 */
	}
	else { /* match==1, conversion successful */
	  if(G__dispsource) {
	    if(G__p_tempbuf->obj.obj.i<0) 
	      fprintf(G__serr,
		      "!!!Create temp object (%s)(%ld),%d for implicit conversion\n"
		      ,conv ,G__p_tempbuf->obj.obj.i ,G__templevel);
	    else
	      fprintf(G__serr,
		      "!!!Create temp object (%s)%ld,%d for implicit conversion\n"
		      ,conv ,G__p_tempbuf->obj.obj.i ,G__templevel);
	  }
#ifdef G__ASM
	  if(G__asm_noverflow && rewind_arg) {
	    rewindflag=1;
#ifdef G__ASM_DBG
	    if(G__asm_dbg) fprintf(G__serr,"%3x: REWINDSTACK %d\n"
				   ,G__asm_cp,-rewind_arg);
#endif
	    G__asm_inst[G__asm_cp] = G__REWINDSTACK;
	    G__asm_inst[G__asm_cp+1] = -rewind_arg;
	    G__inc_cp_asm(2,0);
	  }
#endif
	  *param = G__p_tempbuf->obj;
	  sprintf(parameter,"(%s)%ld" ,G__struct.name[formal_tagnum]
		  ,G__p_tempbuf->obj.obj.i);
	} /* end of if(match==0) */
	
      }
#ifndef G__OLDIMPLEMENTATION1077
      else if(-1!=param->tagnum) {
	long store_struct_offset=G__store_struct_offset;
	int store_tagnum=G__tagnum;
	sprintf(conv,"operator %s()"
		,G__type2string(formal_type,formal_tagnum,-1,0,0));
	G__store_struct_offset = param->obj.i;
	G__tagnum=param->tagnum;
#ifndef G__OLDIMPLEMENTATION1130
#ifdef G__ASM
	if(G__asm_noverflow) {
	  if(rewind_arg) {
	    rewindflag=1;
#ifdef G__ASM_DBG
	    if(G__asm_dbg) fprintf(G__serr,"%3x: REWINDSTACK %d\n"
				   ,G__asm_cp,rewind_arg);
#endif
	    G__asm_inst[G__asm_cp] = G__REWINDSTACK;
	    G__asm_inst[G__asm_cp+1] = rewind_arg;
	    G__inc_cp_asm(2,0);
	  }
	  G__asm_inst[G__asm_cp] = G__PUSHSTROS;
	  G__asm_inst[G__asm_cp+1] = G__SETSTROS;
	  G__inc_cp_asm(2,0);
#ifdef G__ASM_DBG
	  if(G__asm_dbg) {
	    fprintf(G__serr,"%3x: PUSHSTROS\n",G__asm_cp-2);
	    fprintf(G__serr,"%3x: SETSTROS\n",G__asm_cp-1);
	  }
#endif
	}
#endif
#endif
	reg=G__getfunction(conv,&match,G__TRYMEMFUNC);
#ifndef G__OLDIMPLEMENTATION1124
	if(!match
#ifndef G__OLDIMPLEMENTATION1208
	   && 0!=formal_isconst
#endif
	   ) {
	  sprintf(conv,"operator const %s()"
		  ,G__type2string(formal_type,formal_tagnum,-1,0,0));
	  G__store_struct_offset = param->obj.i;
	  G__tagnum=param->tagnum;
	  reg=G__getfunction(conv,&match,G__TRYMEMFUNC);
	}
#endif
	G__tagnum=store_tagnum;
	G__store_struct_offset=store_struct_offset;
#ifndef G__OLDIMPLEMENTATION1130
#ifdef G__ASM
	if(G__asm_noverflow) {
	  if(rewind_arg) {
	    rewindflag=1;
#ifdef G__ASM_DBG
	    if(G__asm_dbg) fprintf(G__serr,"%3x: REWINDSTACK %d\n"
				   ,G__asm_cp,-rewind_arg);
#endif
	    G__asm_inst[G__asm_cp] = G__REWINDSTACK;
	    G__asm_inst[G__asm_cp+1] = -rewind_arg;
	    G__inc_cp_asm(2,0);
	  }
	  G__asm_inst[G__asm_cp] = G__POPSTROS;
	  G__inc_cp_asm(1,0);
#ifdef G__ASM_DBG
	  if(G__asm_dbg) fprintf(G__serr,"%3x: POPSTROS\n",G__asm_cp-1);
#endif
	}
#endif
#endif
#ifndef G__OLDIMPLEMENTATION1129
	/* fixing 'cout<<x' fundamental conversion opr with opr overloading 
	 * Not 100% sure if this is OK. */
	if(match) *param = reg;
	else if(rewindflag) {
#ifdef G__ASM_DBG
	  if(G__asm_dbg) fprintf(G__serr,"REWINDSTACK~ cancelled\n");
#endif
	  G__inc_cp_asm(-7,0);
	}
	else {
#ifdef G__ASM_DBG
	  if(G__asm_dbg) fprintf(G__serr,"PUSHSTROS~ cancelled\n");
#endif
	  G__inc_cp_asm(-3,0);
      }
#endif
      }
#endif
      else {
	match=0;
	/* #ifdef G__DEBUG */
	if(recursive&&G__dispsource) {
	  G__valuemonitor(*param,arg1);
	  fprintf(G__serr ,"!!!Recursive implicit conversion %s(%s) rejected\n"
		  ,G__struct.name[formal_tagnum],arg1);
	}
	/* #endif */
      }
      continue;
    }

    switch(formal_type) {
    case 'b':
    case 'c':
    case 'r':
    case 's':
    case 'h':
    case 'i':
    case 'k':
    case 'l':
      switch(param_type) {
      case 'd':
      case 'f':
	/* std conv */
	if(G__PARAREFERENCE==formal_reftype) {
          param->obj.i = (long)param->obj.d;
          param->type = formal_type;
          param->ref = 0;
        }
	break;
      }
      break;
    case 'd':
    case 'f':
      switch(param_type) {
      case 'b':
      case 'c':
      case 'r':
      case 's':
      case 'h':
      case 'i':
      case 'k':
      case 'l':
	/* std conv */
	if(G__PARAREFERENCE==formal_reftype) {
          param->obj.d = param->obj.i;
          param->type = formal_type;
          param->ref = 0;
        }
	break;
      }
      break;
    case 'u':
      switch(param_type) {
      case 'u':
	if(0==(rate&0xffffff00)) {
	  /* exact */
	  if('e'==G__struct.type[param_tagnum]) {
	    if(param->ref) param->obj.i = *(long*)(param->ref);
	  }
	}
	else /* if(G__PARAREFERENCE==formal_reftype) */ {
	  if(-1 != (baseoffset=G__ispublicbase(formal_tagnum,param_tagnum
					       ,param->obj.i))) {
	    param->tagnum = formal_tagnum;
	    param->obj.i += baseoffset;
	    param->ref = param->obj.i;
#ifdef G__ASM
	    if(G__asm_noverflow) {
	      if(rewind_arg && baseoffset) {
#ifdef G__ASM_DBG
		if(G__asm_dbg) fprintf(G__serr,"%3x: REWINDSTACK %d\n"
				       ,G__asm_cp,rewind_arg);
#endif
		G__asm_inst[G__asm_cp]=G__REWINDSTACK; 
		G__asm_inst[G__asm_cp+1] = rewind_arg;
		G__inc_cp_asm(2,0);
	      }
#ifdef G__ASM_DBG
	      if(G__asm_dbg) fprintf(G__serr,"%3x: BASECONV %d %d\n"
				     ,G__asm_cp,formal_tagnum,baseoffset);
#endif
	      G__asm_inst[G__asm_cp] = G__BASECONV;
	      G__asm_inst[G__asm_cp+1] = formal_tagnum;
	      G__asm_inst[G__asm_cp+2] = baseoffset;
	      G__inc_cp_asm(3,0);
	      if(rewind_arg && baseoffset) {
#ifdef G__ASM_DBG
		if(G__asm_dbg) fprintf(G__serr,"%3x: REWINDSTACK %d\n"
				       ,G__asm_cp,-rewind_arg);
#endif
		G__asm_inst[G__asm_cp] = G__REWINDSTACK;
		G__asm_inst[G__asm_cp+1] = -rewind_arg;
		G__inc_cp_asm(2,0);
	      }
	    }
#endif
	  }
	}
	break;
      }
      break;
    case 'U':
      switch(param_type) {
      case 'U':
	/* Pointer to derived class can be converted to
	 * pointer to base class.
	 * add offset, modify char *parameter and 
	 * G__value *param
	 */
	if(-1 != (baseoffset=G__ispublicbase(formal_tagnum,param_tagnum
					     ,param->obj.i))) {
	  param->tagnum = formal_tagnum;
	  param->obj.i += baseoffset;
#ifndef G__OLDIMPLEMENTATION1308
	  param->ref += baseoffset;
#else
	  param->ref = 0;
#endif
#ifdef G__ASM
	  if(G__asm_noverflow) {
	    if(rewind_arg && baseoffset) {
#ifdef G__ASM_DBG
	      if(G__asm_dbg) fprintf(G__serr,"%3x: REWINDSTACK %d\n"
				     ,G__asm_cp,rewind_arg);
#endif
	      G__asm_inst[G__asm_cp]=G__REWINDSTACK; 
	      G__asm_inst[G__asm_cp+1] = rewind_arg;
	      G__inc_cp_asm(2,0);
	    }
#ifdef G__ASM_DBG
	    if(G__asm_dbg) fprintf(G__serr,"%3x: BASECONV %d %d\n"
				   ,G__asm_cp,formal_tagnum,baseoffset);
#endif
	    G__asm_inst[G__asm_cp] = G__BASECONV;
	    G__asm_inst[G__asm_cp+1] = formal_tagnum;
	    G__asm_inst[G__asm_cp+2] = baseoffset;
	    G__inc_cp_asm(3,0);
	    if(rewind_arg && baseoffset) {
#ifdef G__ASM_DBG
	      if(G__asm_dbg) fprintf(G__serr,"%3x: REWINDSTACK %d\n"
				     ,G__asm_cp,-rewind_arg);
#endif
	      G__asm_inst[G__asm_cp] = G__REWINDSTACK;
	      G__asm_inst[G__asm_cp+1] = -rewind_arg;
	      G__inc_cp_asm(2,0);
	    }
	  }
#endif
	}
	break;
      }
    }
    
  }
}

/***********************************************************************
* G__display_param(scopetagnum,funcname,libp);
**********************************************************************/
void G__display_param(fp,scopetagnum,funcname,libp)
FILE* fp;
int scopetagnum;
char *funcname;
struct G__param *libp;
{
  int i;
  if(-1!=scopetagnum) fprintf(fp,"%s::",G__fulltagname(scopetagnum,1));
  fprintf(fp,"%s(",funcname);
  for(i=0;i<libp->paran;i++) {
    switch(libp->para[i].type) {
    case 'd':
    case 'f':
      fprintf(fp,"%s",G__type2string(libp->para[i].type
				     ,libp->para[i].tagnum
				     ,libp->para[i].typenum
				     ,0
				     ,0));
      break;
    default:
      fprintf(fp,"%s",G__type2string(libp->para[i].type
				     ,libp->para[i].tagnum
				     ,libp->para[i].typenum
				     ,libp->para[i].obj.reftype.reftype
				     ,0));
      break;
    }
    if(i!=libp->paran-1) fprintf(fp,",");
  }
  fprintf(fp,");\n");
}

/***********************************************************************
* G__display_func(G__serr,ifunc,ifn);
**********************************************************************/
void G__display_func(fp,ifunc,ifn)
FILE *fp;
struct G__ifunc_table *ifunc;
int ifn;
{
  int i;
  int store_iscpp = G__iscpp;
  G__iscpp = 1;
  
  if(ifunc->pentry[ifn]->filenum>=0) {
    fprintf(fp,"%-10s%4d "
		  ,G__stripfilename(G__srcfile[ifunc->pentry[ifn]->filenum].filename)
		  ,ifunc->pentry[ifn]->line_number);
  }
  else {
    fprintf(fp,"%-10s%4d ","(compiled)",0);
  }
  fprintf(fp,"%s ",G__type2string(ifunc->type[ifn]
				  ,ifunc->p_tagtable[ifn]
				  ,ifunc->p_typetable[ifn]
				  ,ifunc->reftype[ifn]
				  ,ifunc->isconst[ifn]));
  if(-1!=ifunc->tagnum) fprintf(fp,"%s::",G__fulltagname(ifunc->tagnum,1));
  fprintf(fp,"%s(",ifunc->funcname[ifn]);
  for(i=0;i<ifunc->para_nu[ifn];i++) {
    fprintf(fp,"%s",G__type2string(ifunc->para_type[ifn][i]
				   ,ifunc->para_p_tagtable[ifn][i]
				   ,ifunc->para_p_typetable[ifn][i]
				   ,ifunc->para_reftype[ifn][i]
				   ,ifunc->para_isconst[ifn][i]));
    if(i!=ifunc->para_nu[ifn]-1) fprintf(fp,",");
  }
  fprintf(fp,");\n");

  G__iscpp = store_iscpp;
}

/***********************************************************************
* G__display_ambiguous(funclist,bestmatch);
**********************************************************************/
void G__display_ambiguous(scopetagnum,funcname,libp,funclist,bestmatch)
int scopetagnum;
char *funcname;
struct G__param *libp;
struct G__funclist *funclist;
int bestmatch;
{
  fprintf(G__serr,"Calling : ");
  G__display_param(G__serr,scopetagnum,funcname,libp);
  fprintf(G__serr,"Match rank: file     line  signature\n");
  while(funclist) {
    struct G__ifunc_table *ifunc = funclist->ifunc; 
    int ifn = funclist->ifn;
    if(bestmatch==funclist->rate) fprintf(G__serr,"* %8x ",funclist->rate);
    else                          fprintf(G__serr,"  %8x ",funclist->rate);
    G__display_func(G__serr,ifunc,ifn);
    funclist = funclist->prev;
  }
}

/***********************************************************************
* G__add_templatefunc()
*
* Search matching template function, search by name then parameter.
* If match found, expand template, parse as pre-run 
***********************************************************************/
struct G__funclist* G__add_templatefunc(funcname,libp,hash,funclist
					,p_ifunc,recursive)
char *funcname;
struct G__param *libp;
int hash;
struct G__funclist *funclist;
struct G__ifunc_table *p_ifunc; 
int recursive;
{
  struct G__Definetemplatefunc *deftmpfunc;
  struct G__Charlist call_para;
  /* int env_tagnum=G__get_envtagnum(); */
  int env_tagnum = p_ifunc->tagnum;
  struct G__inheritance *baseclass;
  int store_friendtagnum = G__friendtagnum;
  struct G__ifunc_table *ifunc; 
  int ifn;

  if(-1!=env_tagnum) baseclass = G__struct.baseclass[env_tagnum];
  else               baseclass = &G__globalusingnamespace;
  if(0==baseclass->basen) baseclass = (struct G__inheritance*)NULL;

  call_para.string = (char*)NULL;
  call_para.next = (struct G__Charlist*)NULL;
  deftmpfunc = &G__definedtemplatefunc;
  
  /* Search matching template function name */
  while(deftmpfunc->next) {
    G__freecharlist(&call_para);
    if(deftmpfunc->hash==hash && strcmp(deftmpfunc->name,funcname)==0 &&
       G__matchtemplatefunc(deftmpfunc,libp,&call_para,G__PROMOTION)) {

      if(-1!=deftmpfunc->parent_tagnum && 
	 env_tagnum!=deftmpfunc->parent_tagnum) {
	if(baseclass) {
	  int temp;
	  for(temp=0;temp<baseclass->basen;temp++) {
	    if(baseclass->basetagnum[temp]==deftmpfunc->parent_tagnum) {
	      goto match_found;
	    }
	  }
	}
	deftmpfunc = deftmpfunc->next;
	continue;
      }
    match_found:

      G__friendtagnum = deftmpfunc->friendtagnum;
      
      /* matches funcname and parameter,
       * then expand the template and parse as prerun */
      G__replacetemplate("",funcname
			 ,&call_para /* needs to make this up */
			 ,deftmpfunc->def_fp
			 ,deftmpfunc->line
			 ,deftmpfunc->filenum
			 ,&(deftmpfunc->def_pos)
			 ,deftmpfunc->def_para
			 ,0
			 ,SHRT_MAX /* large enough number */
			 ,deftmpfunc->parent_tagnum
			 );

      G__friendtagnum = store_friendtagnum;

      /* search for instantiated template function */
      ifunc = p_ifunc;
      while(ifunc && ifunc->next && ifunc->next->allifunc) ifunc=ifunc->next;
      if(ifunc) {
	ifn = ifunc->allifunc-1;
	if(strcmp(funcname,ifunc->funcname[ifn])==0) {
	  funclist = G__funclist_add(funclist,ifunc,ifn);
	  if(ifunc->para_nu[ifn]<libp->paran ||
	     (ifunc->para_nu[ifn]>libp->paran&&
	      !ifunc->para_default[ifn][libp->paran])) {
	    funclist->rate = G__NOMATCH;
	  }
	  else {
	    G__rate_parameter_match(libp,ifunc,ifn,funclist,recursive);
	  }
	}
      }
      G__freecharlist(&call_para);
    }
    deftmpfunc = deftmpfunc->next;
  }
  G__freecharlist(&call_para);

  return(funclist);
}

/***********************************************************************
* G__overload_match(funcname,libp,hash,p_ifunc,memfunc_flag,access,pifn)
**********************************************************************/
struct G__ifunc_table* G__overload_match(funcname
				       ,libp
				       ,hash
				       ,p_ifunc
				       ,memfunc_flag
				       ,access
				       ,pifn
				       ,recursive)
char* funcname;
struct G__param *libp;
int hash;
struct G__ifunc_table *p_ifunc; 
int memfunc_flag;
int access;
int *pifn;
int recursive;
{
  struct G__funclist *funclist = (struct G__funclist*)NULL;
  struct G__funclist *match = (struct G__funclist*)NULL;
  unsigned int bestmatch = G__NOMATCH;
  struct G__funclist *func;
  int ambiguous = 0;
  int scopetagnum = p_ifunc->tagnum;
  struct G__ifunc_table *store_ifunc = p_ifunc; 


  /* Search for name match
   *  if reserved func or K&R, match immediately
   *  check number of arguments and default parameters
   *  rate parameter match */
  while(p_ifunc) {
    int ifn;
    for(ifn=0;ifn<p_ifunc->allifunc;++ifn) {
      if(hash==p_ifunc->hash[ifn]&&strcmp(funcname,p_ifunc->funcname[ifn])==0){
	if(p_ifunc->ansi[ifn]==0 || /* K&R C style header */
	   p_ifunc->ansi[ifn]==2 || /* variable number of args */
	   (G__HASH_MAIN==hash && strcmp(funcname,"main")==0)) {
	  /* special match */
	  *pifn = ifn;
	  G__funclist_delete(funclist);
	  return(p_ifunc);
	}
	funclist = G__funclist_add(funclist,p_ifunc,ifn);
	if(p_ifunc->para_nu[ifn]<libp->paran ||
	   (p_ifunc->para_nu[ifn]>libp->paran&&
	    !p_ifunc->para_default[ifn][libp->paran])
#ifdef G__OLDIMPLEMENTATION1260_YET
	   || (G__isconst && 0==p_ifunc->isconst[ifn])
#endif
#ifndef G__OLDIMPLEMENTATION1315
	   || (recursive && p_ifunc->isexplicit[ifn])
#endif
	   ) {
	  funclist->rate = G__NOMATCH;
	}
	else {
	  G__rate_parameter_match(libp,p_ifunc,ifn,funclist,recursive);
	}
	if(G__EXACTMATCH==(funclist->rate&0xffffff00)) match = funclist;
      }
    }
    p_ifunc = p_ifunc->next;
  }

  /* If exact match does not exist 
   *    search for template func
   *    rate parameter match */
  if(!match) {
    funclist =  G__add_templatefunc(funcname,libp,hash,funclist
				    ,store_ifunc,recursive);
  }

  /* if there is no name match, return null */
  if((struct G__funclist*)NULL==funclist) return((struct G__ifunc_table*)NULL);
  /* else  there is function name match */


  /*  choose the best match
   *    display error if the call is ambiguous
   *    display error if there is no parameter match */
  func = funclist;
  ambiguous = 0;
  while(func) {
    if(func->rate<bestmatch) {
      bestmatch = func->rate;
      match = func;
      ambiguous = 0;
    }
    else if(func->rate==bestmatch && bestmatch!=G__NOMATCH) {
      match = func;
      ++ambiguous;
    }
    func = func->prev;
  }

#ifdef G__ASM_DBG2
  G__display_ambiguous(scopetagnum,funcname,libp,funclist,bestmatch);
#endif

  if(!match) {
#if 0
    G__genericerror("Error: No appropriate match in the scope");
    *pifn = -1;
#endif
    G__funclist_delete(funclist);
    return((struct G__ifunc_table*)NULL);
  }

  if(ambiguous && G__EXACTMATCH!=bestmatch) {
    /* error, ambiguous overloading resolution */
    fprintf(G__serr,"Error: Ambiguous overload resolution (%x,%d)"
	    ,bestmatch,ambiguous+1);
    G__genericerror((char*)NULL);
    G__display_ambiguous(scopetagnum,funcname,libp,funclist,bestmatch);
    *pifn = -1;
    G__funclist_delete(funclist);
    return((struct G__ifunc_table*)NULL);
  }

  /* best match function found */
  p_ifunc = match->ifunc;
  *pifn = match->ifn;

  /*  check private, protected access rights 
   *    display error if no access right
   *    do parameter conversion if needed */
  if(0==(p_ifunc->access[*pifn]&access)&&(!G__isfriend(p_ifunc->tagnum))) {
    /* no access right */
    fprintf(G__serr,"Error: can not call private or protected function");
    G__genericerror((char*)NULL);
    fprintf(G__serr,"  ");
    G__display_func(G__serr,p_ifunc,*pifn);
    G__display_ambiguous(scopetagnum,funcname,libp,funclist,bestmatch);
    *pifn = -1;
    G__funclist_delete(funclist);
    return((struct G__ifunc_table*)NULL);
  }

  /* convert parameter */
  G__convert_param(libp,p_ifunc,*pifn,match);

  G__funclist_delete(funclist);
  return(p_ifunc);
}
#endif


/***********************************************************************
* int G__interpret_func(result7,funcname,libp,hash,ifunc,funcmatch)
*
*
*  This function has to be changed to support ANSI style function 
* definition.
*
*
***********************************************************************/
int G__interpret_func(result7,funcname,libp,hash,p_ifunc,funcmatch
		      ,memfunc_flag)
/*  return 1 if function is executed */
/*  return 0 if function isn't executed */
G__value *result7;
char *funcname;
struct G__param *libp;
int hash;
struct G__ifunc_table *p_ifunc; /*local variable overrides global variable*/
int funcmatch;
int memfunc_flag;
{
  int ifn=0;
  struct G__var_array G_local;
  FILE *prev_fp;
  fpos_t prev_pos /*,temppos */;
  /* paraname[][] is used only for K&R func param. length should be OK */
  char paraname[G__MAXFUNCPARA][G__MAXNAME];
  char temp[G__ONELINE];
  int ipara=0;
  int cin='\0';
  int /* ichar=0,*/ itemp=0;
  /* int store_linenumber; */
  int break_exit_func;
  int store_decl;
  G__value buf;
  int store_var_type;
  int store_tagnum;
  long store_struct_offset; /* used to be int */
  int store_inherit_tagnum;
  long store_inherit_offset;
  struct G__ifunc_table *ifunc;
  int iexist,virtualtag;
  int store_var_typeB;
  int store_doingconstruction;
  int store_func_now;
#ifndef G__OLDIMPLEMENTATION927
  int store_func_page;
#endif
  int store_iscpp;
  int store_exec_memberfunc;
  G__UINT32 store_security;

#ifdef G__ASM_IFUNC
  long asm_inst_g[G__MAXINST]; /* p-code instruction buffer */
  G__value asm_stack_g[G__MAXSTACK]; /* data stack */
  char asm_name[G__ASM_FUNCNAMEBUF];

  long *store_asm_inst;
  G__value *store_asm_stack;
  char *store_asm_name;
  int store_asm_name_p;
  struct G__param *store_asm_param;
  int store_asm_exec;
  int store_asm_noverflow;
  int store_asm_cp;
  int store_asm_dt;
  int store_asm_index; /* maybe unneccessary */
#endif
#ifdef G__ASM_WHOLEFUNC
  int store_no_exec_compile=0;
  struct G__var_array *localvar=NULL;
#endif
#ifdef G__NEWINHERIT
  int basen=0;
  int isbase;
  int access;
  int memfunc_or_friend=0;
  struct G__inheritance *baseclass=NULL;
#endif
/* #define G__OLDIMPLEMENTATION590 */
#ifndef G__OLDIMPLEMENTATION590
  int local_tagnum=0;
#endif
#ifndef G__OLDIMPLEMENTATION1076
#ifdef G__OLDIMPLEMENTATION1290
  int store_funcmatch=funcmatch;
#endif
  struct G__ifunc_table *store_p_ifunc=p_ifunc;
#ifdef G__OLDIMPLEMENTATION1290
  int badflag=0;
#endif
#endif
#ifndef G__OLDIMPLEMENTATION1312
  int specialflag=0;
#endif

#ifdef G__NEWINHERIT
  store_inherit_offset = G__store_struct_offset;
  store_inherit_tagnum = G__tagnum;
#endif
#ifndef G__OLDIMPLEMENTATION755
  store_asm_noverflow = G__asm_noverflow;
#endif
  
#ifdef G__ASM_IFUNC
  if(G__asm_exec) {
    ifn = G__asm_index;
#ifndef G__OLDIMPLEMENTATION864
    /* delete 0 ~destructor ignored */
    if(0==G__store_struct_offset && -1!=p_ifunc->tagnum && 
       0==p_ifunc->staticalloc[ifn] && '~'==p_ifunc->funcname[ifn][0]) {
      return(1);
    }
#endif
    goto asm_ifunc_start;
  }
#endif
  
  
  /*******************************************************
   * searching function 
   *******************************************************/
#ifdef G__NEWINHERIT
  if((G__exec_memberfunc&&(-1!=G__tagnum||-1!=G__memberfunc_tagnum)) ||
      G__TRYNORMAL!=memfunc_flag) {
    isbase=1;
    basen=0;
    if(G__exec_memberfunc&&-1==G__tagnum) local_tagnum=G__memberfunc_tagnum;
    else                                  local_tagnum=G__tagnum;
    baseclass = G__struct.baseclass[local_tagnum];
    if(G__exec_memberfunc || G__isfriend(G__tagnum)) {
      access = G__PUBLIC_PROTECTED_PRIVATE ;
      memfunc_or_friend = 1;
    }
    else {
      access = G__PUBLIC;
      memfunc_or_friend = 0;
    }
  }
  else {
    access = G__PUBLIC;
    isbase=0;
#ifndef G__OLDIMPLEMENTATION1307
    if (p_ifunc && p_ifunc == G__p_ifunc) {
      basen=0;
      isbase = 1;
      baseclass = &G__globalusingnamespace;
    }
#endif
  }
 next_base:
#endif

  /* FROM HERE */
#ifndef G__OLDIMPLEMENTATION1290

  p_ifunc = G__overload_match(funcname,libp,hash,p_ifunc,memfunc_flag
			      ,access,&ifn,0);
  /* error */
  if(-1==ifn) {
    *result7 = G__null;
    return(1);
  }

#else /* 1290 */

  /*******************************************************
   * while interpreted function list exists
   *******************************************************/
  while(p_ifunc) {
    while((ipara==0)&&(ifn<p_ifunc->allifunc)) {
      /* if hash (sum of funcname char) matchs */
      if(hash==p_ifunc->hash[ifn] &&
	 strcmp(funcname,p_ifunc->funcname[ifn])==0 
#ifdef G__OLDIMPLEMENTATION1076
	 && (p_ifunc->access[ifn]&access)
#endif
#ifndef G__OLDIMPLEMENTATION1250
	 && (G__TRYIMPLICITCONSTRUCTOR!=memfunc_flag ||
	     0==p_ifunc->isexplicit[ifn])
#endif
	 ) {
#ifdef G__OVERLOADFUNC
	/**************************************************
	 * for overloading of function and operator
	 **************************************************/
	/**************************************************
	 * check if parameter type matchs
	 **************************************************/
	/* set(reset) match flag ipara temporarily */
	itemp=0;
	ipara=1;
	
	if(p_ifunc->ansi[ifn]==0) break; /* K&R C style header */
#ifndef G__OLDIMPLEMENTATION832
	if(p_ifunc->ansi[ifn]==2) break; /* variable number of args */
#endif
	/* main() no overloading */
	if(G__HASH_MAIN==hash && strcmp(funcname,"main")==0) break; 
	
	/* if more actual parameter than formal parameter, unmatch */
#ifndef G__OLDIMPLEMENTATION1137
	if(p_ifunc->para_nu[ifn]<libp->paran ||
	   (p_ifunc->para_nu[ifn]>libp->paran&&
	    !p_ifunc->para_default[ifn][libp->paran])
#ifndef G__OLDIMPLEMENTATION1260
	   || (G__isconst && 0==p_ifunc->isconst[ifn])
	   || (0==G__isconst && p_ifunc->isconst[ifn] && G__EXACT==funcmatch)
#endif
	   ) {
#else
	if(p_ifunc->para_nu[ifn]<libp->paran) {
#endif
	  ipara=0;
	  itemp=p_ifunc->para_nu[ifn]; /* end of this parameter */
	  ++ifn; /* next function */
	}
	else {
	  /* scan each parameter */
	  while(itemp<p_ifunc->para_nu[ifn]) {
	    if((G__value*)NULL==p_ifunc->para_default[ifn][itemp] && 
	       itemp>libp->paran) {
	      ipara = 0;
	    }
#ifndef G__FONS41
	    else if (p_ifunc->para_default[ifn][itemp] && itemp>=libp->paran) {
	      ipara = 2; /* I'm not sure what this is, Fons. */
	    }
#endif
	    else {   
	      ipara=G__param_match(p_ifunc->para_type[ifn][itemp]
				   ,p_ifunc->para_p_tagtable[ifn][itemp]
				   ,p_ifunc->para_default[ifn][itemp]
				   ,libp->para[itemp].type
				   ,libp->para[itemp].tagnum
				   ,&(libp->para[itemp])
				   ,libp->parameter[itemp]
				   ,funcmatch
				   ,p_ifunc->para_nu[ifn]-itemp-1
#ifndef G__OLDIMPLEMENTATION1120
				   ,p_ifunc->para_reftype[ifn][itemp]
#endif
#ifndef G__OLDIMPLEMENTATION1208
				   ,p_ifunc->para_isconst[ifn][itemp]
#endif
				   );
	    }
	    switch(ipara) {
	    case 2: /* default parameter */
#ifdef G__ASM_DBG
	      if(G__asm_dbg) {
		fprintf(G__serr," default%d %c tagnum%d %d : %c tagnum%d %d\n"
			,itemp
			,p_ifunc->para_type[ifn][itemp]
			,p_ifunc->para_p_tagtable[ifn][itemp]
			,p_ifunc->para_default[ifn][itemp]
			,libp->para[itemp].type
			,libp->para[itemp].tagnum
			,funcmatch);
	      }
#endif
	      itemp=p_ifunc->para_nu[ifn]; /* end of this parameter */
	      break;
	    case 1: /* match this one, next parameter */
#ifdef G__ASM_DBG
	      if(G__asm_dbg) {
		fprintf(G__serr," match%d %c tagnum%d %d : %c tagnum%d %d\n"
			,itemp
			,p_ifunc->para_type[ifn][itemp]
			,p_ifunc->para_p_tagtable[ifn][itemp]
			,p_ifunc->para_default[ifn][itemp]
			,libp->para[itemp].type
			,libp->para[itemp].tagnum
			,funcmatch);
	      }
#endif
#ifndef G__OLDIMPLEMENTATION1003
	      if(G__EXACT!=funcmatch)
		G__warn_refpromotion(p_ifunc,ifn,itemp,libp);
#endif
	      ++itemp; /* next function parameter */
	      break;
	    case 0: /* unmatch, next function */
#ifdef G__ASM_DBG
	      if(G__asm_dbg) {
		fprintf(G__serr," unmatch%d %c tagnum%d %d : %c tagnum%d %d\n"
			,itemp
			,p_ifunc->para_type[ifn][itemp]
			,p_ifunc->para_p_tagtable[ifn][itemp]
			,p_ifunc->para_default[ifn][itemp]
			,libp->para[itemp].type
			,libp->para[itemp].tagnum
			,funcmatch);
	      }
#endif
	      itemp=p_ifunc->para_nu[ifn]; 
	      /* exit from while loop */
	      break;
	    }
	    
	  } /* end of while(itemp<p_ifunc->para_nu[ifn]) */
	  if(ipara==0) { /* parameter doesn't match */
	    ++ifn; /* next function */
	  }
	}
#else
	/**************************************************
	 * doesn't check parameter type 
	 **************************************************/
	ipara=1;
#endif
      }
      else {  /* funcname doesn't match */
	++ifn;
      }
    }  /* end of while((ipara==0))&&(ifn<p_ifunc->allifunc)) */
    /******************************************************************
     * next page of interpreted function list
     *******************************************************************/
    if(ifn>=p_ifunc->allifunc) {
      p_ifunc=p_ifunc->next;
      ifn=0;
    }
    else {
      break; /* get out from while(p_ifunc) loop */
    }
  } /* end of while(p_ifunc) */


#ifndef G__OLDIMPLEMENTATION1076
  /**********************************************************************
   * another iteration for overloading match
   **********************************************************************/
  if(NULL==p_ifunc && store_funcmatch==G__EXACT &&-1!=store_p_ifunc->tagnum) {
#ifndef G__OLDIMPLEMENTATION1160
    if(G__EXACT==funcmatch) {
      int storeX_exec_memberfunc=G__exec_memberfunc;
      int storeX_memberfunc_tagnum=G__memberfunc_tagnum;
      G__exec_memberfunc=1;
      G__memberfunc_tagnum=store_p_ifunc->tagnum;
      if(G__templatefunc(result7,funcname,libp,hash,funcmatch)==1){
        G__exec_memberfunc=storeX_exec_memberfunc;
        G__memberfunc_tagnum=storeX_memberfunc_tagnum;
        return(1);
      }
      G__exec_memberfunc=storeX_exec_memberfunc;
      G__memberfunc_tagnum=storeX_memberfunc_tagnum;
    }
#endif
    if(funcmatch<G__USERCONV) {
      ipara=0;
      ifn = 0;
      p_ifunc=store_p_ifunc;
      ++funcmatch;
      goto next_base; /* I know this is a bad manner */
    }
    funcmatch=store_funcmatch;
  }

  /**********************************************************************
   * error detection for private or protected member func access
   **********************************************************************/
  if(p_ifunc&&0==(p_ifunc->access[ifn]&access)&&!G__isfriend(G__tagnum)) {
    ++badflag;
    if(funcmatch<G__USERCONV) {
      ipara=0;
      ifn = 0;
      p_ifunc=store_p_ifunc;
      ++funcmatch;
      goto next_base; /* I know this is a bad manner */
    }
    p_ifunc=(struct G__ifunc_table*)NULL;
  }
  if(NULL==p_ifunc && badflag
#ifndef G__OLDIMPLEMENTATION1143
     && G__NOLINK==G__globalcomp
#endif
     ) {
    if(-1==store_p_ifunc->tagnum) 
      fprintf(G__serr,"Error: %s is private or protected",funcname);
    else
      fprintf(G__serr,"Error: %s::%s is private or protected"
	      ,G__fulltagname(store_p_ifunc->tagnum,1),funcname);
    G__genericerror((char*)NULL);
    return(1);
  }
#endif
  
#endif /* 1290 */

  /* TO HERE */

#ifdef G__NEWINHERIT
  /**********************************************************************
   * iteration for base class member function search
   **********************************************************************/
  if(p_ifunc==NULL || 
     (G__PUBLIC!=p_ifunc->access[ifn] && !G__isfriend(G__tagnum)
      && (0==G__exec_memberfunc || 
	  (local_tagnum!=G__memberfunc_tagnum 
#ifndef G__OLDIMPLEMENTATION760
	   && (G__PROTECTED!=p_ifunc->access[ifn]
	       || -1==G__ispublicbase(local_tagnum,G__memberfunc_tagnum
				      ,store_inherit_offset))
#endif
	  )))) {
    if(isbase) {
      while(baseclass && basen<baseclass->basen) {
	if(memfunc_or_friend) {
	  if((baseclass->baseaccess[basen]&G__PUBLIC_PROTECTED) ||
	     baseclass->property[basen]&G__ISDIRECTINHERIT) {
	    access = G__PUBLIC_PROTECTED;
	    G__incsetup_memfunc(baseclass->basetagnum[basen]);
	    p_ifunc = G__struct.memfunc[baseclass->basetagnum[basen]];
#ifdef G__VIRTUALBASE
	    if(baseclass->property[basen]&G__ISVIRTUALBASE) {
	      G__store_struct_offset = store_inherit_offset + 
		G__getvirtualbaseoffset(store_inherit_offset,G__tagnum
					,baseclass,basen);
	    }
	    else {
	      G__store_struct_offset
		= store_inherit_offset + baseclass->baseoffset[basen];
	    }
#else
	    G__store_struct_offset
	      = store_inherit_offset + baseclass->baseoffset[basen];
#endif
	    G__tagnum = baseclass->basetagnum[basen];
	    ++basen;
#ifndef G__OLDIMPLEMENTATION1076
	    store_p_ifunc=p_ifunc;
#endif
	    goto next_base; /* I know this is a bad manner */
	  }
	}
	else {
	  if(baseclass->baseaccess[basen]&G__PUBLIC) {
	    access = G__PUBLIC;
	    G__incsetup_memfunc(baseclass->basetagnum[basen]);
	    p_ifunc = G__struct.memfunc[baseclass->basetagnum[basen]];
#ifdef G__VIRTUALBASE
	    if(baseclass->property[basen]&G__ISVIRTUALBASE) {
	      G__store_struct_offset = store_inherit_offset + 
		G__getvirtualbaseoffset(store_inherit_offset,G__tagnum
					,baseclass,basen);
	    }
	    else {
	      G__store_struct_offset
		= store_inherit_offset + baseclass->baseoffset[basen];
	    }
#else
	    G__store_struct_offset
	      = store_inherit_offset + baseclass->baseoffset[basen];
#endif
	    G__tagnum = baseclass->basetagnum[basen];
	    ++basen;
#ifndef G__OLDIMPLEMENTATION1076
	    store_p_ifunc=p_ifunc;
#endif
	    goto next_base; /* I know this is a bad manner */
	  }
	}
	++basen;
      }
      isbase=0;
    }

#ifndef G__OLDIMPLEMENTATION1312
    if(0==specialflag && 1==libp->paran && -1!=libp->para[0].tagnum &&
       -1!=G__struct.parent_tagnum[libp->para[0].tagnum]) {
      p_ifunc =
	G__struct.memfunc[G__struct.parent_tagnum[libp->para[0].tagnum]];
      store_p_ifunc=p_ifunc;
      specialflag = 1;
      goto next_base;
    }
#endif

    /* not found */
    G__store_struct_offset = store_inherit_offset ;
    G__tagnum = store_inherit_tagnum ;
#ifndef G__OLDIMPLEMENTATION755
    G__asm_noverflow = store_asm_noverflow;
#endif
    return(0);
  }
#else
  /******************************************************************
   * if no such func, return 0
   *******************************************************************/
  if(p_ifunc==NULL) {
    return(0);
  }

  /******************************************************************
   * member access control
   *******************************************************************/
  if(G__PUBLIC!=p_ifunc->access[ifn] && !G__isfriend(G__tagnum)) {
    return(0);
  }
#endif


asm_ifunc_start:   /* loop compilation execution label */


  /******************************************************************
   * Constructor or destructor call in G__make_ifunctable() parameter
   * type allocation. Return without call.
   * Also, when parameter analysis with -c optoin, return without call.
   *******************************************************************/
  if(G__globalcomp) { /* with -c-1 or -c-2 option */
    result7->obj.d = 0.0;
    result7->ref = 0;
    result7->type=p_ifunc->type[ifn];
    result7->tagnum=p_ifunc->p_tagtable[ifn];
    result7->typenum=p_ifunc->p_typetable[ifn];
#ifndef G__OLDIMPLEMENTATION1259
    result7->isconst = p_ifunc->isconst[ifn];
#endif
    if(isupper(result7->type)) {
      result7->obj.reftype.reftype = p_ifunc->reftype[ifn];
    }
    return(1);
  }
  else if(G__prerun) { /* in G__make_ifunctable parameter allocation */
    result7->obj.i = p_ifunc->type[ifn];
    result7->ref = 0;
    result7->type= G__DEFAULT_FUNCCALL;
    result7->tagnum=p_ifunc->p_tagtable[ifn];
    result7->typenum=p_ifunc->p_typetable[ifn];
#ifndef G__OLDIMPLEMENTATION1259
    result7->isconst = p_ifunc->isconst[ifn];
#endif
    if(isupper(result7->type)) {
      result7->obj.reftype.reftype = p_ifunc->reftype[ifn];
    }
    return(1);
  }

  /******************************************************************
   * error if body not defined
   *******************************************************************/
#ifdef G__ASM_WHOLEFUNC
  if((FILE*)NULL==(FILE*)p_ifunc->pentry[ifn]->p 
     && 0==p_ifunc->ispurevirtual[ifn] 
     && G__ASM_FUNC_NOP==G__asm_wholefunction) {
#else
  if((FILE*)NULL==(FILE*)p_ifunc->pentry[ifn]->p 
     && 0==p_ifunc->ispurevirtual[ifn]) {
#endif
#ifdef G__OLDIMPLEMENTATION1293
    if(G__EXACT==funcmatch) {
      return(0);
    }
    else 
#endif
    {
#ifndef G__OLDIMPLEMENTATION851
      if(0==G__templatefunc(result7,funcname,libp,hash,funcmatch)) {
	if(G__USERCONV==funcmatch) {
	  fprintf(G__serr,"Error: %s() header declared but not defined"
		  ,funcname);
	  G__genericerror((char*)NULL);
	  return(1);
	}
	else return(0);
      }
      return(1);
#else
      fprintf(G__serr,"Error: %s() header declared but not defined",funcname);
      G__genericerror((char*)NULL);
      return(1);
#endif
    }
  }
  
  /******************************************************************
   * function was found in interpreted function list
   *******************************************************************/
  /* p_ifunc has found */
  
  /******************************************************************
   * Add baseoffset if calling base class member function.
   * Resolution of virtual function is not done here. There is a
   * separate section down below. Search string 'virtual function'
   * to get there.
   *******************************************************************/
  G__tagnum = p_ifunc->tagnum;
  store_exec_memberfunc=G__exec_memberfunc;
#ifndef G__OLDIMPLEMENTATION589
  if(-1==G__tagnum&&-1==G__memberfunc_tagnum) G__exec_memberfunc=0;
#else
  if(-1==G__tagnum) G__exec_memberfunc=0;
#endif

#define G__OLDIMPLEMENTATION1101
#ifndef G__OLDIMPLEMENTATION1101
  if(memfunc_flag==G__CALLSTATICMEMFUNC && 0==G__store_struct_offset &&
     -1!=G__tagnum && 0==p_ifunc->staticalloc[ifn] && 0==G__no_exec) {
    fprintf(G__serr,"Error: %s() Illegal non-static member function call"
	    ,funcname);
    G__genericerror((char*)NULL);
    *result7 = G__null;
    return(1);
  }
#endif
  
  store_var_typeB = G__var_typeB;
  G__var_typeB='p';

#ifdef G__NEWINHERIT
#ifdef G__ASM
  if(G__asm_noverflow && G__store_struct_offset && 
     G__store_struct_offset!=store_inherit_offset) {
#ifdef G__ASM_DBG
    if(G__asm_dbg) {
      fprintf(G__serr,"%3x: ADDSTROS %d\n"
	      ,G__asm_cp,G__store_struct_offset-store_inherit_offset);
    }
#endif
    G__asm_inst[G__asm_cp]=G__ADDSTROS;
    G__asm_inst[G__asm_cp+1]=G__store_struct_offset-store_inherit_offset;
    G__inc_cp_asm(2,0);
  }
#endif
#endif

  
  /******************************************************************
   * C++ compiled function
   *******************************************************************/
  if(-1 == p_ifunc->pentry[ifn]->filenum) {
    G__call_cppfunc(result7,libp,p_ifunc,ifn);
    /* recover tag environment */
    G__store_struct_offset = store_inherit_offset ;
    G__tagnum = store_inherit_tagnum ;
    if('P'==store_var_typeB) G__val2pointer(result7);
#ifdef G__NEWINHERIT
#ifdef G__ASM
    if(G__asm_noverflow && G__store_struct_offset && 
       G__store_struct_offset!=store_inherit_offset) {
#ifdef G__ASM_DBG
      if(G__asm_dbg) 
	fprintf(G__serr,"%3x: ADDSTROS %d\n"
		,G__asm_cp, -G__store_struct_offset+store_inherit_offset);
#endif
      G__asm_inst[G__asm_cp]=G__ADDSTROS;
      G__asm_inst[G__asm_cp+1]= -G__store_struct_offset+store_inherit_offset;
      G__inc_cp_asm(2,0);
    }
#endif /* ASM */
#endif /* NEWINHERIT */
    G__exec_memberfunc=store_exec_memberfunc;
    return(1);
  }
  
#ifdef G__ASM
  /******************************************************************
   * create bytecode instruction for calling interpreted function
   *******************************************************************/
  if(G__asm_noverflow) {
#ifdef G__ASM_DBG
    if(G__asm_dbg) fprintf(G__serr ,"%3x: LD_IFUNC %s paran=%d\n"
			   ,G__asm_cp,funcname,libp->paran);
#endif
    G__asm_inst[G__asm_cp]=G__LD_IFUNC;
    G__asm_inst[G__asm_cp+1]=(long)p_ifunc->funcname[ifn];
    G__asm_inst[G__asm_cp+2]=hash;
    G__asm_inst[G__asm_cp+3]=libp->paran;
    G__asm_inst[G__asm_cp+4]=(long)p_ifunc;
    G__asm_inst[G__asm_cp+5]=(long)funcmatch;
    G__asm_inst[G__asm_cp+6]=(long)memfunc_flag;
    G__asm_inst[G__asm_cp+7]=(long)ifn;
    G__inc_cp_asm(8,0);
    if(G__store_struct_offset && G__store_struct_offset!=store_inherit_offset){
#ifdef G__ASM_DBG
      if(G__asm_dbg) 
	fprintf(G__serr,"%3x: ADDSTROS %d\n"
		,G__asm_cp, -G__store_struct_offset+store_inherit_offset);
#endif
      G__asm_inst[G__asm_cp]=G__ADDSTROS;
      G__asm_inst[G__asm_cp+1]= -G__store_struct_offset+store_inherit_offset;
      G__inc_cp_asm(2,0);
    }
  }
#endif /* G__ASM */

  /* G__oprovld is set when calling operator overload function after 
   * evaluating its' argument to avoid duplication in p-code stack data.
   * This must be reset when calling lower level interpreted function */
  G__oprovld=0;

#ifdef G__ASM
  if(G__no_exec_compile) {
    G__store_struct_offset = store_inherit_offset ;
    G__tagnum = store_inherit_tagnum ;
    result7->tagnum = p_ifunc->p_tagtable[ifn];
    if(-1!=result7->tagnum && 'e'!=G__struct.type[result7->tagnum]) {
      if(isupper(p_ifunc->type[ifn])) result7->type='U';
      else                            result7->type = 'u';
    }
    else {
      result7->type = p_ifunc->type[ifn];
    }
    result7->typenum = p_ifunc->p_typetable[ifn];
    result7->ref = p_ifunc->reftype[ifn];
#ifndef G__OLDIMPLEMENTATION1259
    result7->isconst = p_ifunc->isconst[ifn];
#endif
    result7->obj.d = 0.0;
    result7->obj.i = 1;
    if(isupper(result7->type)) {
      result7->obj.reftype.reftype = p_ifunc->reftype[ifn];
    }
    /* To be implemented */
    G__exec_memberfunc=store_exec_memberfunc;
    return(1);
  }
#endif /* G__ASM */

#ifndef G__OLDIMPLEMENTATION891
  /******************************************************************
   * virtual function
   *  If virtual function flag is set, get actual tag identity by
   * taginfo member at offset of G__struct.virtual_offset[].
   * Then search for virtual function in actual tag. If found, 
   * change p_ifunc,ifn,G__store_struct_offset and G__tagnum.
   * G__store_struct_offset and G__tagnum are already stored above,
   * so no need to store it to temporary here.
   *******************************************************************/
  if(p_ifunc->isvirtual[ifn] && !G__fixedscope) {
#ifndef G__OLDIMPLEMENTATION1279
    if(-1!=G__struct.virtual_offset[G__tagnum])
      virtualtag= *(long*)(G__store_struct_offset /* NEED TO CHECK THIS PART */
			   +G__struct.virtual_offset[G__tagnum]);
    else {
      virtualtag = G__tagnum;
    }
#else
    virtualtag= *(long*)(G__store_struct_offset /* NEED TO CHECK THIS PART */
			 +G__struct.virtual_offset[G__tagnum]);
#endif
    if(virtualtag!=G__tagnum) {
      struct G__inheritance *baseclass = G__struct.baseclass[virtualtag];
#ifndef G__OLDIMPLEMENTATION916
      int xbase[G__MAXBASE],ybase[G__MAXBASE];
      int nxbase=0,nybase;
#endif
      int basen;
      G__incsetup_memfunc(virtualtag);
      ifunc=G__ifunc_exist(p_ifunc,ifn,G__struct.memfunc[virtualtag],&iexist);
#ifndef G__OLDIMPLEMENTATION916
      for(basen=0;!ifunc&&basen<baseclass->basen;basen++) {
	virtualtag = baseclass->basetagnum[basen];
	if(0==(baseclass->property[basen]&G__ISDIRECTINHERIT)) continue;
	xbase[nxbase++] = virtualtag;
	G__incsetup_memfunc(virtualtag);
	ifunc
	  =G__ifunc_exist(p_ifunc,ifn,G__struct.memfunc[virtualtag],&iexist);
      } 
      while(!ifunc && nxbase) {
	int xxx;
	nybase=0;
	for(xxx=0;!ifunc&&xxx<nxbase;xxx++) {
	  baseclass = G__struct.baseclass[xbase[xxx]];
	  for(basen=0;!ifunc&&basen<baseclass->basen;basen++) {
	    virtualtag = baseclass->basetagnum[basen];
	    if(0==(baseclass->property[basen]&G__ISDIRECTINHERIT)) continue;
	    ybase[nybase++] = virtualtag;
	    G__incsetup_memfunc(virtualtag);
	    ifunc
	      =G__ifunc_exist(p_ifunc,ifn,G__struct.memfunc[virtualtag]
			      ,&iexist);
	  } 
	} 
	nxbase=nybase;
	memcpy((void*)xbase,(void*)ybase,sizeof(int)*nybase);
      }
#else
      for(basen=0;!ifunc&&basen<baseclass->basen;basen++) {
	virtualtag = baseclass->basetagnum[basen];
	G__incsetup_memfunc(virtualtag);
	ifunc
	  =G__ifunc_exist(p_ifunc,ifn,G__struct.memfunc[virtualtag],&iexist);
      } 
#endif
      if(ifunc) {
	if((FILE*)NULL==(FILE*)ifunc->pentry[iexist]->p) {
	  fprintf(G__serr,"Error: virtual %s() header found but not defined",funcname);
	  G__genericerror((char*)NULL);
	  G__exec_memberfunc=store_exec_memberfunc;
	  return(1);
	}
	p_ifunc=ifunc;
	ifn=iexist;
	G__store_struct_offset -= G__find_virtualoffset(virtualtag);
	G__tagnum=virtualtag;
	if('~'==funcname[0]) {
	  strcpy(funcname+1,G__struct.name[G__tagnum]);
	  G__hash(funcname,hash,itemp);
	}
      }
      else if(p_ifunc->ispurevirtual[ifn]) {
	fprintf(G__serr,"Error: pure virtual %s() not defined",funcname);
	G__genericerror((char*)NULL);
	G__exec_memberfunc=store_exec_memberfunc;
	return(1);
      }
    }
  }
#endif /* ON891 */


#ifdef G__ASM
#ifdef G__ASM_WHOLEFUNC
#ifndef G__OLDIMPLEMENTATION507
  /******************************************************************
   * try bytecode compilation
   *******************************************************************/
  if(G__BYTECODE_NOTYET==p_ifunc->pentry[ifn]->bytecodestatus &&
     G__asm_loopcompile>3 && G__ASM_FUNC_NOP==G__asm_wholefunction && 
#ifndef G__TO_BE_DELETED
     G__CALLCONSTRUCTOR!=memfunc_flag && G__TRYCONSTRUCTOR!=memfunc_flag &&
#ifndef G__OLDIMPLEMENTATINO1250
     G__TRYIMPLICITCONSTRUCTOR!=memfunc_flag && 
#endif
     G__TRYDESTRUCTOR!=memfunc_flag && 
#ifdef G__OLDIMPLEMENTATION891
     !p_ifunc->isvirtual[ifn] &&
#endif
#endif
     0==G__step && (G__asm_noverflow||G__asm_exec
#ifndef G__OLDIMPLEMENTATION857
		    ||G__asm_loopcompile>4
#endif
		    )) {
    G__compile_bytecode(p_ifunc,ifn);
  }
#endif /* ON507 */
  /******************************************************************
   * if already compiled as bytecode run bytecode
   *******************************************************************/
  if(p_ifunc->pentry[ifn]->bytecode
#ifndef G__OLDIMPLEMENTATION1164
     && G__BYTECODE_ANALYSIS!=p_ifunc->pentry[ifn]->bytecodestatus
#endif
     ) {
    struct G__input_file store_ifile;
    store_ifile=G__ifile;
    G__ifile.filenum=p_ifunc->pentry[ifn]->filenum;
    G__ifile.line_number=p_ifunc->pentry[ifn]->line_number;
    G__exec_bytecode(result7,(char*)p_ifunc->pentry[ifn]->bytecode,libp,hash);
    G__ifile=store_ifile;
    G__tagnum = store_inherit_tagnum;
    G__store_struct_offset = store_inherit_offset;
    return(1);
  }
#endif /* G__ASM_WHOLEFUNC */
#endif /* G__ASM */

#ifndef G__OLDIMPLEMENTATION1167
  G__reftypeparam(p_ifunc,ifn,libp);
#endif

#ifdef G__ASM
#ifdef G__ASM_IFUNC
  /******************************************************************
   * push bytecode environment stack
   *******************************************************************/
  /*
  if(G__asm_noverflow) G__asm_inst[G__asm_cp+1]=(long)p_ifunc->funcname[ifn];
  */
  /* Push loop compilation environment */
  store_asm_inst = G__asm_inst;
  store_asm_stack = G__asm_stack;
  store_asm_name = G__asm_name;
  store_asm_name_p = G__asm_name_p;
  store_asm_param  = G__asm_param ;
  store_asm_exec  = G__asm_exec ;
  store_asm_noverflow  = G__asm_noverflow ;
  store_asm_cp  = G__asm_cp ;
  store_asm_dt  = G__asm_dt ;
  store_asm_index  = G__asm_index ;

  G__asm_inst = asm_inst_g;
  G__asm_stack = asm_stack_g;
  G__asm_name = asm_name;
  G__asm_name_p = 0;
  /* G__asm_param ; */
  G__asm_exec = 0 ;
#endif /* G__ASM_IFUNC */
#endif /* G__ASM */

#ifdef G__ASM
#ifdef G__ASM_IFUNC
#ifdef G__ASM_WHOLEFUNC
  /******************************************************************
   * bytecode function compilation start
   *******************************************************************/
  if(G__ASM_FUNC_COMPILE&G__asm_wholefunction) {
    if(G__asm_dbg) {
      fprintf(G__serr,"!!!bytecode compilation %s start"
	      ,p_ifunc->funcname[ifn]);
      G__printlinenum();
    }
#ifndef G__OLDIMPLEMENTATION513
    G__asm_name = (char*)malloc(G__ASM_FUNCNAMEBUF);
#endif
    G__asm_noverflow = 1;
    store_no_exec_compile = G__no_exec_compile;
    G__no_exec_compile = 1;
    localvar = (struct G__var_array*)malloc(sizeof(struct G__var_array));

    localvar->prev_local = G__p_local;
    localvar->ifunc = p_ifunc;
    localvar->ifn = ifn;
#ifdef G__VAARG
    localvar->libp = libp;
#endif
    localvar->tagnum=G__tagnum;
    localvar->struct_offset=G__store_struct_offset;
    localvar->exec_memberfunc=G__exec_memberfunc;
    localvar->allvar=0;
    localvar->varlabel[0][0]=0;
    localvar->next=NULL;
    localvar->prev_filenum = G__ifile.filenum;
    localvar->prev_line_number = G__ifile.line_number;
  }
  else {
    G__asm_noverflow = 0;
  }
#else /* G__ASM_WHOLEFUNC */
  G__asm_noverflow = 0;
#endif /* G__ASM_WHOLEFUNC */
  G__asm_cp = 0;
  G__asm_dt = G__MAXSTACK-1;
  /* G__asm_index  ; */
#endif /* G__ASM_IFUNC */
#endif /* G__ASM */

  
#ifdef G__OLDIMPLEMENTATION891
  /******************************************************************
   * virtual function
   *  If virtual function flag is set, get actual tag identity by
   * taginfo member at offset of G__struct.virtual_offset[].
   * Then search for virtual function in actual tag. If found, 
   * change p_ifunc,ifn,G__store_struct_offset and G__tagnum.
   * G__store_struct_offset and G__tagnum are already stored above,
   * so no need to store it to temporary here.
   *******************************************************************/
  if(p_ifunc->isvirtual[ifn] && !G__fixedscope) {
    virtualtag= *(long*)(G__store_struct_offset /* NEED TO CHECK THIS PART */
			 +G__struct.virtual_offset[G__tagnum]);
    if(virtualtag!=G__tagnum) {
      struct G__inheritance *baseclass = G__struct.baseclass[virtualtag];
      int basen;
      G__incsetup_memfunc(virtualtag);
      ifunc=G__ifunc_exist(p_ifunc,ifn,G__struct.memfunc[virtualtag],&iexist);
      for(basen=0;!ifunc&&basen<baseclass->basen;basen++) {
	virtualtag = baseclass->basetagnum[basen];
	G__incsetup_memfunc(virtualtag);
	ifunc
	  =G__ifunc_exist(p_ifunc,ifn,G__struct.memfunc[virtualtag],&iexist);
      } 
      if(ifunc) {
	if((FILE*)NULL==(FILE*)ifunc->pentry[iexist]->p) {
	  fprintf(G__serr,"Error: virtual %s() header found but not defined",funcname);
	  G__genericerror((char*)NULL);
	  G__exec_memberfunc=store_exec_memberfunc;
	  return(1);
	}
	p_ifunc=ifunc;
	ifn=iexist;
	G__store_struct_offset -= G__find_virtualoffset(virtualtag);
	G__tagnum=virtualtag;
	if('~'==funcname[0]) {
	  strcpy(funcname+1,G__struct.name[G__tagnum]);
	  G__hash(funcname,hash,itemp);
	}
      }
      else if(p_ifunc->ispurevirtual[ifn]) {
	fprintf(G__serr,"Error: pure virtual %s() not defined",funcname);
	G__genericerror((char*)NULL);
	G__exec_memberfunc=store_exec_memberfunc;
	return(1);
      }
    }
  }
#endif /* ON891 */

  /******************************************************************
   * G__exec_memberfunc and G__memberfunc_tagnum are stored in one 
   * upper level G__getfunction() and G__parenthesisovld() and restored 
   * when exit from these functions.
   *******************************************************************/
#ifndef G__OLDIMPLEMENTATION725
  if(-1==p_ifunc->tagnum) G__exec_memberfunc=0;
  else                    G__exec_memberfunc=1;
#else
  if(G__TRYNORMAL!=memfunc_flag) G__exec_memberfunc=1;
#endif
  G__setclassdebugcond(G__tagnum,0);
  G__memberfunc_tagnum = G__tagnum;
  G__memberfunc_struct_offset=G__store_struct_offset;
  
  /**********************************************
   * If return value is struct,class,union, 
   * create temp object buffer
   **********************************************/
  if(p_ifunc->type[ifn]=='u' && p_ifunc->reftype[ifn]==G__PARANORMAL
     && G__CPPLINK!=G__struct.iscpplink[p_ifunc->p_tagtable[ifn]]) {
    /* create temp object buffer */
    
    G__alloc_tempobject(p_ifunc->p_tagtable[ifn] ,p_ifunc->p_typetable[ifn]);
    
    if(G__dispsource) {
#ifndef G__FONS31
      fprintf(G__serr,"!!!Create temp object (%s)0x%lx,%d for %s() return\n"
	      ,G__struct.name[p_ifunc->p_tagtable[ifn]]
	      ,G__p_tempbuf->obj.obj.i ,G__templevel ,p_ifunc->funcname[ifn]);
#else
      fprintf(G__serr,"!!!Create temp object (%s)0x%x,%d for %s() return\n"
	      ,G__struct.name[p_ifunc->p_tagtable[ifn]]
	      ,G__p_tempbuf->obj.obj.i ,G__templevel ,p_ifunc->funcname[ifn]);
#endif
    }
  }
  
  /**********************************************
   * increment busy flag
   **********************************************/
  p_ifunc->busy[ifn]++;
  
  /**********************************************
   * set global variable G__func_now. This is
   * used in G__malloc() to allocate static
   * variable.
   **********************************************/
  store_func_now=G__func_now;
#ifndef G__OLDIMPLEMENTATION927
  store_func_page = G__func_page;
#endif
  G__func_now=ifn;
  G__func_page=p_ifunc->page;
  
  /* store old local to prev buffer and allocate new local variable */
  G_local.prev_local = G__p_local;
  G_local.ifunc = p_ifunc;
  G_local.ifn = ifn;
#ifdef G__VAARG
  G_local.libp = libp;
#endif
  G_local.tagnum=G__tagnum;
  G_local.struct_offset=G__store_struct_offset;
  G_local.exec_memberfunc=G__exec_memberfunc;
#ifdef G__ASM_WHOLEFUNC
  if(G__ASM_FUNC_COMPILE&G__asm_wholefunction) G__p_local = localvar;
  else                     G__p_local = &G_local;
#else
  G__p_local = &G_local;
#endif
  
  break_exit_func=G__break_exit_func;
  
  G__break_exit_func=0;
  G__p_local->allvar=0;
  G__p_local->varlabel[0][0]=0;
  G__p_local->next=NULL;
  
  /* store line number and filename*/
  G_local.prev_filenum = G__ifile.filenum;
  G_local.prev_line_number = G__ifile.line_number;
  G__ifile.line_number = p_ifunc->pentry[ifn]->line_number;
  strcpy(G__ifile.name,G__srcfile[p_ifunc->pentry[ifn]->filenum].filename);
  G__ifile.filenum=p_ifunc->pentry[ifn]->filenum;

  /* check breakpoint */
  if(0==G__nobreak && 0==G__no_exec_compile &&
     G__srcfile[G__ifile.filenum].breakpoint &&
     G__srcfile[G__ifile.filenum].maxline>G__ifile.line_number &&
     G__TESTBREAK&(G__srcfile[G__ifile.filenum].breakpoint[G__ifile.line_number]|=G__TRACED)) {
    G__BREAKfgetc();
  }

  store_security = G__security;
  G__security = G__srcfile[G__ifile.filenum].security;

  /* store file pointer and fpos*/
  /* store_linenumber=G__ifile.line_number; */
  if(G__ifile.fp) fgetpos(G__ifile.fp,&prev_pos);
  prev_fp = G__ifile.fp ;
  
#ifndef G__PHILIPPE0
  /* Find the right file pointer */
  if( G__mfp && (FILE*)p_ifunc->pentry[ifn]->p == G__mfp ) {
    /* In case of macro expanded by cint, we use the tmpfile */
    G__ifile.fp = (FILE*)p_ifunc->pentry[ifn]->p ;
  } 
  else if(G__srcfile[G__ifile.filenum].fp) {
    /* The file is already open use that */
    G__ifile.fp = G__srcfile[G__ifile.filenum].fp;
  } 
  else {
    /* The file had been closed, let's reopen the proper file
     * resp from the preprocessor and raw */
    if ( G__srcfile[G__ifile.filenum].prepname ) {
      G__ifile.fp = fopen(G__srcfile[G__ifile.filenum].prepname,"r");
    } else {
      G__ifile.fp = fopen(G__srcfile[G__ifile.filenum].filename,"r");
    }
    G__srcfile[G__ifile.filenum].fp =  G__ifile.fp;
#ifndef G__OLDIMPLEMENTATION1301
    if(!G__ifile.fp) G__ifile.fp = (FILE*)p_ifunc->pentry[ifn]->p ;
#endif
  }
#else
  G__ifile.fp = (FILE*)p_ifunc->pentry[ifn]->p ;
#endif
  fsetpos(G__ifile.fp,&p_ifunc->pentry[ifn]->pos);
  
  /* print function header if debug mode */
  
  if(G__dispsource) {
#ifndef G__OLDIMPLEMENTATION854
    G__disp_mask=0;
    if((G__debug||G__break||G__step
	||(strcmp(G__breakfile,G__ifile.name)==0)||(strcmp(G__breakfile,"")==0)
	)&&((G__prerun!=0)||(G__no_exec==0))) {
      if(G__ifile.name&&G__ifile.name[0]) 
	fprintf(G__serr,"\n# %s",G__ifile.name);
#else
    if((G__debug||G__break||G__step
	||(strcmp(G__breakfile,G__ifile.name)==0)||(strcmp(G__breakfile,"")==0)
	)&&((G__prerun!=0)||(G__no_exec==0))&&
       (G__disp_mask==0)){
#endif
      if(-1!=p_ifunc->tagnum) {
	fprintf(G__serr ,"\n%-5d%s::%s(" ,G__ifile.line_number
		,G__struct.name[p_ifunc->tagnum] ,funcname);
      }
      else {
	fprintf(G__serr ,"\n%-5d%s(" ,G__ifile.line_number,funcname);
      }
    }
  }

  
  /* now came to         func( para1,para2,,,)
   *                          ^
   */
  
  store_doingconstruction=G__doingconstruction;
  /**************************************************************
   * standard C
   *
   *   func( para1 ,para2 ,,, )
   *        ^
   **************************************************************/
  if(p_ifunc->ansi[ifn]==0) {
    
    /* read pass parameters , standard C */
    ipara=0;
    while(cin!=')') {
      cin=G__fgetstream(temp,",)");
      if(temp[0]!='\0') {
	strcpy(paraname[ipara],temp);
	ipara++;
      }
    }
    
    /* read and exec pass parameter declaration , standard C 
     * G__exec_statement() returns at '{' if G__funcheader==1
     */
    G__funcheader=1;
    G__mparen=0;
    do {
      buf=G__exec_statement();
    } while(buf.type==G__null.type && G__return<G__RETURN_EXIT1);
    
    /* set pass parameters , standard C 
     * Parameters can be constant. When G__funcheader==1,
     * error message of changing const doesn't appear.
     */
    
    for(itemp=0;itemp<ipara;itemp++) {
      G__letvariable(paraname[itemp],libp->para[itemp]
		     ,&G__global,G__p_local);
    }
    
    G__funcheader=0;
    /* fsetpos(G__ifile.fp,&temppos) ; */
    /* G__ifile.line_number=store_linenumber; */
    fseek(G__ifile.fp,-1,SEEK_CUR);
    if(G__dispsource) G__disp_mask=1;
  }
  
  /**************************************************************
   * ANSI C
   *
   *   type func( type para1, type para2 ,,, )
   *             ^
   **************************************************************/
  else {
#ifndef G__OLDIMPLEMENTATION522
    G__value store_ansipara;
    store_ansipara=G__ansipara;
#endif
    G__ansiheader=1;
    G__funcheader=1;
    
    ipara=0;
    while(G__ansiheader!=0 && G__return<G__RETURN_EXIT1) {
      /****************************************
       * for default parameter
       ****************************************/
      /****************************************
       * if parameter exists, set G__ansipara
       ****************************************/
      if(ipara<libp->paran) {
	G__ansipara=libp->para[ipara];
	/* assigning reference for fundamental type reference argument */
#ifndef G__OLDIMPLEMENTATION1016
	if(0==G__ansipara.ref) {
	  switch(p_ifunc->para_type[ifn][ipara]) {
	  case 'f':
	    G__Mfloat(libp->para[ipara]);
	    G__ansipara.ref = (long)(&libp->para[ipara].obj.fl);
	    break;
	  case 'd':
	    G__ansipara.ref = (long)(&libp->para[ipara].obj.d);
	    break;
	  case 'c':
	    G__Mchar(libp->para[ipara]);
	    G__ansipara.ref = (long)(&libp->para[ipara].obj.ch);
	    break;
	  case 's':
	    G__Mshort(libp->para[ipara]);
	    G__ansipara.ref = (long)(&libp->para[ipara].obj.sh);
	    break;
	  case 'i':
	    G__Mint(libp->para[ipara]);
	    G__ansipara.ref = (long)(&libp->para[ipara].obj.in);
	    break;
	  case 'l':
	    G__ansipara.ref = (long)(&libp->para[ipara].obj.i);
	    break;
	  case 'b':
	    G__Muchar(libp->para[ipara]);
	    G__ansipara.ref = (long)(&libp->para[ipara].obj.uch);
	    break;
	  case 'r':
	    G__Mushort(libp->para[ipara]);
	    G__ansipara.ref = (long)(&libp->para[ipara].obj.ush);
	    break;
	  case 'h':
	    /* G__Muint(libp->para[ipara]); */
	    G__ansipara.ref = (long)(&libp->para[ipara].obj.i);
	    break;
	  case 'k':
	    G__ansipara.ref = (long)(&libp->para[ipara].obj.i);
	    break;
	  case 'u':
	    G__ansipara.ref = G__ansipara.obj.i;
	    break;
          default:
	    G__ansipara.ref = (long)(&libp->para[ipara].obj.i);
	    break;
	  }
	}
#else
	if(0==G__ansipara.ref)
	  G__ansipara.ref = (long)(&libp->para[ipara].obj);
#endif
      }
      /****************************************
       * if not, set null.
       * Default value will be used
       *  type func(type paraname=default,...)
       ****************************************/
      else {
	if(
#ifndef G__OLDIMPLEMENTATION1324
	   p_ifunc->para_nu[ifn]>ipara && 
#endif
	   p_ifunc->para_default[ifn][ipara]) {
	  if(p_ifunc->para_default[ifn][ipara]->type==G__DEFAULT_FUNCCALL) {
	    G__ASSERT(p_ifunc->para_default[ifn][ipara]->ref);
	    *p_ifunc->para_default[ifn][ipara] =
	      G__getexpr((char*)p_ifunc->para_default[ifn][ipara]->ref);
	    G__ansiheader=1;
	    G__funcheader=1;
	  }
	  G__ansipara = *p_ifunc->para_default[ifn][ipara];
	}
	else
	  G__ansipara = G__null;
      }
      G__refansipara = libp->parameter[ipara];
      
#ifndef G__OLDIMPLEMENTATION517
      if(G__ASM_FUNC_COMPILE==G__asm_wholefunction &&
	 p_ifunc->para_default[ifn][ipara]) {
#ifdef G__ASM_DBG
	if(G__asm_dbg) {
	  fprintf(G__serr,"%3x: ISDEFAULTPARA %x\n",G__asm_cp,G__asm_cp+4);
	  fprintf(G__serr,"%3x: LD %d %g\n",G__asm_cp+2
		  ,p_ifunc->para_default[ifn][ipara]->obj.i
		  ,p_ifunc->para_default[ifn][ipara]->obj.d
		  );
	}
#endif
	G__asm_inst[G__asm_cp] = G__ISDEFAULTPARA;
	G__asm_wholefunc_default_cp=G__asm_cp+1;
	G__inc_cp_asm(2,0);

	/* set default param in stack */
	G__asm_inst[G__asm_cp]=G__LD;
	G__asm_inst[G__asm_cp+1]=G__asm_dt;
	G__asm_stack[G__asm_dt] = *p_ifunc->para_default[ifn][ipara];
	G__inc_cp_asm(2,1);

	G__asm_inst[G__asm_wholefunc_default_cp]=G__asm_cp;
#ifndef G__OLDIMPLEMENTATION1164
	G__suspendbytecode(); /* mask default param evaluation */
#else
	G__abortbytecode(); /* mask default param evaluation */
#endif
	G__exec_statement(); /* Create var entry and ST_LVAR inst */
	G__asm_wholefunc_default_cp=0;
	G__asm_noverflow=1;
      }
      else {
	G__exec_statement();
      }
#else
      G__exec_statement();
#endif
      ipara++;
    }
    
    G__funcheader=0;
    
    switch(memfunc_flag) {
    case G__CALLCONSTRUCTOR:
    case G__TRYCONSTRUCTOR:
#ifndef G__OLDIMPLEMENTATINO1250
    case G__TRYIMPLICITCONSTRUCTOR:
#endif
      /* read parameters for base constructors and 
       * constructor for base calss and class members 
       * maybe with some parameters
       */
      G__baseconstructorwp();
      G__doingconstruction=1;
    }

#ifndef G__OLDIMPLEMENTATION522
    G__ansipara=store_ansipara;
#endif
  }

#ifdef G__SECURITY
  if((G__security&G__SECURE_STACK_DEPTH)&&
     G__max_stack_depth && G__templevel>G__max_stack_depth) {
    fprintf(G__serr,"Error: Stack depth exceed %d",G__max_stack_depth);
    G__genericerror((char*)NULL);
    G__return=G__RETURN_EXIT1;
  }

  if(G__return>G__RETURN_EXIT1) {
    G__exec_memberfunc=store_exec_memberfunc;
    G__security = store_security;
    return(1);
  }
#endif

  G__setclassdebugcond(G__memberfunc_tagnum,1);
  
  /**************************************************************
   * execute ifunction body 
   *
   * common to standard and ANSI
   **************************************************************/

  store_iscpp=G__iscpp;
  G__iscpp=p_ifunc->iscpp[ifn];

  G__templevel++;
#ifdef G__ASM_DBG
  if(G__istrace>1) {
    if(G__istrace>G__templevel) {
      G__debug = 1;
      G__asm_dbg = 1;
    }
    else {
      G__debug = 0;
      G__asm_dbg = 0;
    }
  }
#endif
  
  G__ASSERT(0==G__decl || 1==G__decl);
  store_decl=G__decl;
  G__decl=0;
  G__no_exec=0;	
  G__mparen=0;
  *result7=G__exec_statement();
  G__decl=store_decl;
  G__ASSERT(0==G__decl || 1==G__decl);

  if(G__RETURN_IMMEDIATE==G__return &&
     G__interactivereturnvalue.type && '\0'==result7->type) {
    *result7 = G__interactivereturnvalue;
    G__interactivereturnvalue = G__null;
  }
  
  G__templevel--;
#ifdef G__ASM_DBG
  if(G__istrace>1) {
    if(G__istrace>G__templevel) {
      G__debug = 1;
      G__asm_dbg = 1;
    }
    else {
      G__debug = 0;
      G__asm_dbg = 0;
    }
  }
#endif

  G__iscpp=(short)store_iscpp;
  
  G__doingconstruction=store_doingconstruction;

  /**************************************************************
   * Error if goto label not found
   **************************************************************/
  if(G__gotolabel[0]) {
    fprintf(G__serr,"Error: Goto label '%s' not found in %s()"
	    ,G__gotolabel,funcname);
    G__genericerror((char*)NULL);
    G__gotolabel[0]='\0';
  }
  
  /**************************************************************
   * return value type conversion
   *
   **************************************************************/
  if(
#ifndef G__OLDIMPLEMENTATION1164
     0==G__xrefflag &&
#endif
     result7->type!='\0' && G__RETURN_EXIT1!=G__return
#ifndef G__OLDIMPLEMENTATION507
     && (G__ASM_FUNC_NOP==G__asm_wholefunction||G__asm_noverflow)
#endif     
     ) {
    switch(p_ifunc->type[ifn]) {
      
      /***************************************************
       * in case of double and float.
       ***************************************************/
    case 'd': /* double */
    case 'f': /* float */
    case 'w': /* logic (original type) */
      G__letdouble(result7,p_ifunc->type[ifn]
		   ,G__double(*result7));
#define G__OLDIMPLEMENTATION753
#ifdef G__OLDIMPLEMENTATION753
      if(p_ifunc->reftype[ifn]==G__PARANORMAL) result7->ref=0;
#endif
#ifndef G__OLDIMPLEMENTATION1259
      result7->isconst = p_ifunc->isconst[ifn];
#endif
      break;
      
      /***************************************************
       * in case of void, if return(); statement exists
       * it is illegal.
       * Maybe bug if return; statement exists without
       * return value.
       ***************************************************/
    case 'y': /* void */
      if(G__RETURN_NORMAL==G__return) {
	fprintf(G__serr,"Warning: Return value of void %s() ignored"
		,p_ifunc->funcname[ifn]);
	G__genericerror((char*)NULL);
      }
      *result7 = G__null ;
      break;
      
      /***************************************************
       * result7 contains pointer to the local variable
       * which will be destroyed right after this.
       ***************************************************/
    case 'u': /* struct, union, class */
      if(
#ifndef G__OLDIMPLEMENTATION1274
	 1
#else
#ifndef G__OLDIMPLEMENTATION1011
	 (result7->type=='u' || (result7->type=='i'&& -1!=result7->tagnum))
#else
	 result7->type=='u' 
#endif
	 && result7->tagnum == p_ifunc->p_tagtable[ifn]
#endif
	 ) {
	
	/* don't call copy constructor if returning reference type */
	if(G__PARANORMAL!=p_ifunc->reftype[ifn]) break;
	
#ifndef G__OLDIMPLEMENTATION1274
	if(result7->type=='u' || (result7->type=='i'&& -1!=result7->tagnum)){
	  if(result7->obj.i<0)
	    sprintf(temp,"%s((%s)(%ld))",G__struct.name[p_ifunc->p_tagtable[ifn]]
		    ,G__fulltagname(result7->tagnum,1) ,result7->obj.i);
	  else
	    sprintf(temp,"%s((%s)%ld)",G__struct.name[p_ifunc->p_tagtable[ifn]]
		    ,G__fulltagname(result7->tagnum,1) ,result7->obj.i);
	}
	else {
	  char buf2[G__ONELINE];
	  G__valuemonitor(*result7,buf2);
	    sprintf(temp,"%s(%s)",G__struct.name[p_ifunc->p_tagtable[ifn]]
		    ,buf2);
	}
#else
	if(result7->obj.i<0)
	  sprintf(temp,"%s((%s)(%ld))" ,G__struct.name[result7->tagnum]
		  ,G__fulltagname(result7->tagnum,1) ,result7->obj.i);
	else
	  sprintf(temp,"%s((%s)%ld)" ,G__struct.name[result7->tagnum]
		  ,G__fulltagname(result7->tagnum,1) ,result7->obj.i);
#endif
	
	store_tagnum = G__tagnum;
#ifndef G__OLDIMPLEMENTATION1274
	G__tagnum = p_ifunc->p_tagtable[ifn];
#else
	G__tagnum = result7->tagnum;
#endif
	store_var_type = G__var_type; /* bug fix */

	  
#ifdef G__SECURITY
	G__castcheckoff = 1;
#endif
	if(G__RETURN_IMMEDIATE>=G__return) G__return=G__RETURN_NON;
	itemp=0;
	store_struct_offset = G__store_struct_offset;

	if(G__CPPLINK!=G__struct.iscpplink[G__tagnum]) {
	  /* interpreted class */
	  G__store_struct_offset=G__p_tempbuf->obj.obj.i;
	  if(G__dispsource) {
	    fprintf(G__serr
	    ,"\n!!!Calling copy/conversion constructor for return temp object 0x%lx.%s"
		    ,G__store_struct_offset
		    ,temp);
	  }
	  G__getfunction(temp,&itemp,G__TRYCONSTRUCTOR);
	}
	else {
	  /* precompiled class */
	  G__store_struct_offset=0xffff;
	  if(G__dispsource) {
	    fprintf(G__serr
	    ,"\n!!!Calling copy/conversion constructor for return temp object 0x%lx.%s"
		    ,G__store_struct_offset
		    ,temp);
	  }
	  buf=G__getfunction(temp,&itemp,G__TRYCONSTRUCTOR);
#ifndef G__OLDIMPLEMENtATION1274
	  if(itemp) G__store_tempobject(buf);
#else
	  G__store_tempobject(buf);
#endif
#ifdef G__ASM
	  /* It is not needed to explicitly create STORETEMP instruction
	   * because it is preincluded in the compiled funciton call 
	   * interface */
#endif
	  if(G__dispsource) {
	    fprintf(G__serr
		    ,"!!!Create temp object (%s)0x%lx,%d for %s() return\n"
		    ,G__struct.name[G__tagnum] ,G__p_tempbuf->obj.obj.i
		    ,G__templevel ,p_ifunc->funcname[ifn]);
	  }
	}
	
	G__store_struct_offset = store_struct_offset;
	G__tagnum = store_tagnum;
	G__var_type = store_var_type; /* bug fix */
	
	/**************************************
	 * if no copy constructor, memberwise
	 * copy
	 **************************************/
	if(itemp==0
#ifndef G__OLDIMPLEMENTATION1164
	   && 0==G__xrefflag
#endif
	   ) {
	  
#ifndef G__OLDIMPLEMENTATION1274
	  long offset=0;
	  if(result7->tagnum == p_ifunc->p_tagtable[ifn]) {
	    memcpy((void*)G__p_tempbuf->obj.obj.i
		   ,(void*)(result7->obj.i)
		   ,(size_t)G__struct.size[result7->tagnum]);
	  }
	  else if(-1!=(offset=G__ispublicbase(p_ifunc->p_tagtable[ifn]
					      ,result7->tagnum
					      ,result7->obj.i))){
	    sprintf(temp,"%s((%s)(%ld))"
		    ,G__struct.name[p_ifunc->p_tagtable[ifn]]
		    ,G__fulltagname(p_ifunc->p_tagtable[ifn],1)
		    ,result7->obj.i+offset);
	    if(G__CPPLINK!=G__struct.iscpplink[G__tagnum]) {
	      /* interpreted class */
	      G__store_struct_offset=G__p_tempbuf->obj.obj.i;
	      G__getfunction(temp,&itemp,G__TRYCONSTRUCTOR);
	    }
	    else {
	      /* precompiled class */
	      G__store_struct_offset=0xffff;
	      buf=G__getfunction(temp,&itemp,G__TRYCONSTRUCTOR);
	      if(itemp) G__store_tempobject(buf);
	    }
	  }
#else
	  memcpy((void*)G__p_tempbuf->obj.obj.i
		 ,(void*)(result7->obj.i)
		 ,(size_t)G__struct.size[result7->tagnum]);
#endif
	}
	
	*result7 = G__p_tempbuf->obj;
	
      }
      else {
	fprintf(G__serr,"Error: Return type mismatch %s()"
		,p_ifunc->funcname[ifn]);
	G__genericerror((char*)NULL);
      }
#ifndef G__OLDIMPLEMENTATION1259
      result7->isconst = p_ifunc->isconst[ifn];
#endif
      break;

#ifndef G__OLDIMPLEMENTATION522
    case 'i':
      /* return value of constructor */
      if(-1!=p_ifunc->p_tagtable[ifn]) {
#ifndef G__OLDIMPLEMENTATION843
	if(G__CPPLINK!=G__struct.iscpplink[p_ifunc->p_tagtable[ifn]] &&
           'e'!=G__struct.type[p_ifunc->p_tagtable[ifn]] &&
	   0!=G__store_struct_offset && 1!=G__store_struct_offset)
	  result7->obj.i=G__store_struct_offset;
#endif
	result7->ref = result7->obj.i;
#ifndef G__OLDIMPLEMENTATION1259
	result7->isconst = 0;
#endif
	break;
      }
#endif

      /***************************************************
       * Everything else is returned as integer. This
       * includes char,short,int,long,unsigned version
       * of them, pointer and struct/union.
       * If return value is struct/union, malloced memory
       * area will be freed about 20 lines below by
       * G__destroy(). To prevent any data loss, memory
       * area has to be copied to left hand side memory
       * area of assignment (or temp buffer of expression
       * parser which doesn't exist in this version).
       ***************************************************/
    default:
#ifdef G__SECURITY
      if(isupper(p_ifunc->type[ifn])&&islower(result7->type)&&result7->obj.i
#ifndef G__OLDIMPLEMENTATION531
         && 0==G__asm_wholefunction
#endif
	 ) {
	fprintf(G__serr,"Error: Return type mismatch %s()"
		,p_ifunc->funcname[ifn]);
	G__genericerror((char*)NULL);
	break;
      }
#endif
      G__letint(result7,p_ifunc->type[ifn],G__int(*result7));
#ifdef G__OLDIMPLEMENTATION753
      if(p_ifunc->reftype[ifn]==G__PARANORMAL) result7->ref=0;
#endif
      if(isupper(result7->type)) {
	result7->obj.reftype.reftype = p_ifunc->reftype[ifn];
      }
#ifdef G__SECURITY
      if(isupper(result7->type)&&G__security&G__SECURE_GARBAGECOLLECTION
#ifndef G__OLDIMPLEMENTATION545
	 && (!G__no_exec_compile)
#endif
	) {
	/* Add reference count to avoid garbage collection when pointer is
	 * returned */
	G__add_refcount((void*)result7->obj.i,(void**)NULL);
      }
#endif
#ifndef G__OLDIMPLEMENTATION1259
      result7->isconst = p_ifunc->isconst[ifn];
#endif
      break;
    }
  }
  
  if(G__RETURN_EXIT1!=G__return) { /* if not exit */
    /* return struct and typedef identity */
    result7->tagnum  = p_ifunc->p_tagtable[ifn];
    result7->typenum = p_ifunc->p_typetable[ifn];
  }
  
  
  /**************************************************************
   * reset no exec flag
   **************************************************************/
  G__no_exec=0;
  
  /**************************************************************
   * reset return flag
   * G__return is set to 1 if interpreted function returns by
   * return(); statement.  Until G__return is reset to 0, 
   * execution flow exits from G__exec_statment().
   **************************************************************/
  if(G__RETURN_IMMEDIATE>=G__return) G__return=G__RETURN_NON;
  
#ifndef G__NEWINHERIT
  /* recover prev local variable */
  G__p_local = G_local.prev_local; /* same as G__p_local->prev_local */
#endif

#ifdef G__ASM_WHOLEFUNC
  /**************************************************************
   * whole function bytecode compile end
   **************************************************************/
  if(G__ASM_FUNC_COMPILE&G__asm_wholefunction) {
    if(G__NOERROR!=G__security_error) {
#ifndef G__OLDIMPLEMENTATION1164
      G__resetbytecode();
#else
      G__abortbytecode();
#endif
    }
    if(G__asm_noverflow) {
      int pc=0;
      if(G__asm_dbg) {
#ifdef G__ASM_DBG
	fprintf(G__serr,"%x : RTN_FUNC",G__asm_cp);
#endif
	fprintf(G__serr,"Bytecode compilation of %s successful"
		,p_ifunc->funcname[ifn]);
	G__printlinenum();
      }
      G__asm_inst[G__asm_cp] = G__RTN_FUNC;
      G__asm_inst[G__asm_cp+1] = 0;
      G__inc_cp_asm(2,0);
      G__asm_inst[G__asm_cp] = G__RETURN;
#ifndef G__OLDIMPLEMENTATION842
      G__resolve_jumptable_bytecode();
#endif
      if(G__asm_loopcompile>=2) G__asm_optimize(&pc);
#ifndef G__OLDIMPLEMENTATION1164
      G__resetbytecode();
#else
      G__abortbytecode();
#endif
      G__no_exec_compile = store_no_exec_compile;
      G__asm_storebytecodefunc(p_ifunc,ifn,localvar
			       ,G__asm_stack,G__asm_dt
			       ,G__asm_inst,G__asm_cp);
    }
    else {
#ifndef G__OLDIMPLEMENTATION509
      /* destroy temp object, before restoreing G__no_exec_compile */
      if(G__p_tempbuf->level>=G__templevel && G__p_tempbuf->prev) 
	G__free_tempobject();
#endif
#ifndef G__OLDIMPLEMENTATION513
      free(G__asm_name);
#endif
#ifndef G__OLDIMPLEMENTATION1164
      G__resetbytecode();
#else
      G__abortbytecode();
#endif
      G__no_exec_compile = store_no_exec_compile;
      /* destroy local memory area */
      G__destroy(localvar,G__BYTECODELOCAL_VAR) ;
      free((void*)localvar);
      if(G__asm_dbg) {
	fprintf(G__serr
		,"Warning: Bytecode compilation of %s failed. Maybe slow"
		,p_ifunc->funcname[ifn]);
	G__printlinenum();
      }
      if(G__return>=G__RETURN_IMMEDIATE) G__return=G__RETURN_NON;
      G__security_error=G__NOERROR;
    }
  }
  else {
    /**************************************************************
     * destroy malloced local memory area
     **************************************************************/
    G__destroy(&G_local,G__LOCAL_VAR) ;
  }
#else /* G__ASM_WHOLEFUNC */
  /**************************************************************
   * destroy malloced local memory area
   **************************************************************/
  G__destroy(&G_local,G__LOCAL_VAR) ;
#endif /* G__ASM_WHOLEFUNC */
  
  if(G__TRYDESTRUCTOR==memfunc_flag) {
    /* destructor for base calss and class members */
    G__basedestructor();
  }

#ifdef G__NEWINHERIT
  /* recover prev local variable */
  G__p_local = G_local.prev_local; /* same as G__p_local->prev_local */
#endif
  
  G__tagnum = store_inherit_tagnum;
  G__store_struct_offset = store_inherit_offset;
  
  /* recover line number and filename*/
  G__ifile.line_number = G_local.prev_line_number;
  G__ifile.filenum=G_local.prev_filenum;
  if(-1!=G__ifile.filenum) 
    strcpy(G__ifile.name,G__srcfile[G__ifile.filenum].filename);
  else {
    G__ifile.name[0]='\0';
  }

#ifndef G__OLDIMPLEMENTATION854
  if(G__dispsource && G__ifile.name && G__ifile.name[0]) 
    fprintf(G__serr,"\n# %s   ",G__ifile.name);
#endif
  
  /* recover file pointer and fpos */
  G__ifile.fp = prev_fp;
  if(G__ifile.fp) fsetpos(G__ifile.fp,&prev_pos);
  if(G__dispsource) {
    if((G__debug||G__break)&& ((G__prerun!=0)||(G__no_exec==0))&&
       (G__disp_mask==0)){
      fprintf(G__serr,"\n");
    }
  }
  
  if(G__break_exit_func!=0) {
    G__break=1;
    G__break_exit_func=0;
    G__setdebugcond();
  }
  G__break_exit_func=break_exit_func;
  
  /**********************************************
   * decrement busy flag
   **********************************************/
  p_ifunc->busy[ifn]--;
  
  if('P'==store_var_typeB) G__val2pointer(result7);


#ifndef G__OLDIMPLEMENTATION927
  G__func_page = store_func_page;
#endif
  G__func_now=store_func_now;
#ifdef G__ASM_IFUNC
  /* Pop loop compilation environment */
  G__asm_inst = store_asm_inst;
  G__asm_stack = store_asm_stack;
  G__asm_name = store_asm_name;
  G__asm_name_p = store_asm_name_p;
  G__asm_param  = store_asm_param ;
  G__asm_exec  = store_asm_exec ;
  G__asm_noverflow  = store_asm_noverflow ;
  G__asm_cp  = store_asm_cp ;
  G__asm_dt  = store_asm_dt ;
  G__asm_index  = store_asm_index ;
#endif /* G__ASM_IFUNC */

  G__exec_memberfunc=store_exec_memberfunc;
  G__security = store_security;

  return(1);
}





/**************************************************************************
* G__ifunc_exist
*
*  compare  hash,funcname,type,p_tagtype,p_typetable,reftype,para_nu
*           para_type[],para_p_tagtable[],para_p_typetable[],para_reftype[]
*           para_default[]
*
**************************************************************************/
struct G__ifunc_table *G__ifunc_exist(ifunc_now,allifunc,ifunc,piexist)
struct G__ifunc_table *ifunc_now;
int allifunc;
struct G__ifunc_table *ifunc;
int *piexist;
{
  int i,j,paran;
  while(ifunc) {
    for(i=0;i<ifunc->allifunc;i++) {
      if('~'==ifunc_now->funcname[allifunc][0] &&
	 '~'==ifunc->funcname[i][0]) { /* destructor matches with ~ */
	*piexist = i;
	return(ifunc);
      }
      if(ifunc_now->hash[allifunc]!=ifunc->hash[i] ||
	 strcmp(ifunc_now->funcname[allifunc],ifunc->funcname[i]) != 0 ||
	 (ifunc_now->para_nu[allifunc]!=ifunc->para_nu[i] && 
	  ifunc_now->para_nu[allifunc]>=0 && ifunc->para_nu[i]>=0)
#ifndef G__OLDIMPLEMENTATION1258
	 || (ifunc_now->isconst[allifunc]!=ifunc->isconst[i]) 
#endif
	 ) continue; /* unmatch */

      
      if(ifunc_now->para_nu[allifunc]>=0 && ifunc->para_nu[i]>=0)
	paran=ifunc_now->para_nu[allifunc];
      else
	paran = 0;
      for(j=0;j<paran;j++) {
	if(ifunc_now->para_type[allifunc][j]!=ifunc->para_type[i][j] ||
	   ifunc_now->para_p_tagtable[allifunc][j]!=ifunc->para_p_tagtable[i][j]
#ifndef G__OLDIMPLEMENTATION1120
	   || ifunc_now->para_reftype[allifunc][j]!=ifunc->para_reftype[i][j]
#endif
	   ) {
	  break; /* unmatch */
	}
      }
      if(j==paran) { /* all matched */
	*piexist = i;
	return(ifunc);
      }
    }
    ifunc=ifunc->next;
  }
  return(ifunc); /* not found case */
}

/**************************************************************************
* G__ifunc_ambiguous
*
*
**************************************************************************/
struct G__ifunc_table *G__ifunc_ambiguous(ifunc_now,allifunc,ifunc,piexist
					  ,derivedtagnum)
struct G__ifunc_table *ifunc_now;
int allifunc;
struct G__ifunc_table *ifunc;
int *piexist;
int derivedtagnum;
{
  int i,j,paran;
  while(ifunc) {
    for(i=0;i<ifunc->allifunc;i++) {
      if('~'==ifunc_now->funcname[allifunc][0] &&
	 '~'==ifunc->funcname[i][0]) { /* destructor matches with ~ */
	*piexist = i;
	return(ifunc);
      }
      if(ifunc_now->hash[allifunc]!=ifunc->hash[i] ||
	 strcmp(ifunc_now->funcname[allifunc],ifunc->funcname[i]) != 0
	 ) continue; /* unmatch */
      if(ifunc_now->para_nu[allifunc] < ifunc->para_nu[i])
	paran=ifunc_now->para_nu[allifunc];
      else
	paran = ifunc->para_nu[i];
      if(paran<0) paran=0;
      for(j=0;j<paran;j++) {
	if(ifunc_now->para_type[allifunc][j]!=ifunc->para_type[i][j]) 
	  break; /* unmatch */
	if(ifunc_now->para_p_tagtable[allifunc][j]
	   ==ifunc->para_p_tagtable[i][j]) continue; /* match */
#ifdef G__VIRTUALBASE
	if(-1==G__ispublicbase(ifunc_now->para_p_tagtable[allifunc][j]
			       ,derivedtagnum,0) ||
	   -1==G__ispublicbase(ifunc->para_p_tagtable[i][j],derivedtagnum,0))
#else
	if(-1==G__ispublicbase(ifunc_now->para_p_tagtable[allifunc][j]
			       ,derivedtagnum) ||
	   -1==G__ispublicbase(ifunc->para_p_tagtable[i][j],derivedtagnum))
#endif
	  break; /* unmatch */
	/* else match */
      }
      if((ifunc_now->para_nu[allifunc] < ifunc->para_nu[i] &&
	  ifunc->para_default[i][paran]) ||
	 (ifunc_now->para_nu[allifunc] > ifunc->para_nu[i] &&
	  ifunc_now->para_default[allifunc][paran])) {
	*piexist = i;
	return(ifunc);
      }
      else if(j==paran) { /* all matched */
	*piexist = i;
	return(ifunc);
      }
    }
    ifunc=ifunc->next;
  }
  return(ifunc); /* not found case */
}

/**************************************************************************
* G__get_ifunchandle
*
*
**************************************************************************/
struct G__ifunc_table *G__get_ifunchandle(funcname,libp,hash,p_ifunc,pifn
					  ,access,funcmatch)
char *funcname;
struct G__param *libp;
int hash;
struct G__ifunc_table *p_ifunc; 
long *pifn;
int access;
int funcmatch;
{
  int ifn=0;
  int ipara=0;
  int itemp=0;

  if(-1!=p_ifunc->tagnum) G__incsetup_memfunc(p_ifunc->tagnum);

  /*******************************************************
   * while interpreted function list exists
   *******************************************************/
  while(p_ifunc) {
    while((ipara==0)&&(ifn<p_ifunc->allifunc)) {
      /* if hash (sum of funcname char) matchs */
      if(hash==p_ifunc->hash[ifn]&&strcmp(funcname,p_ifunc->funcname[ifn])==0 
	 && (p_ifunc->access[ifn]&access)) {
	/**************************************************
	 * for overloading of function and operator
	 **************************************************/
	/**************************************************
	 * check if parameter type matchs
	 **************************************************/
	/* set(reset) match flag ipara temporarily */
	itemp=0;
	ipara=1;
	
	if(p_ifunc->ansi[ifn]==0) break; /* K&R C style header */
	/* main() no overloading */
	if(G__HASH_MAIN==hash && strcmp(funcname,"main")==0) break; 
	
	/* if more actual parameter than formal parameter, unmatch */
	if(p_ifunc->para_nu[ifn]<libp->paran) {
	  ipara=0;
	  itemp=p_ifunc->para_nu[ifn]; /* end of this parameter */
	  ++ifn; /* next function */
	}
	else {
	  /* scan each parameter */
	  while(itemp<p_ifunc->para_nu[ifn]) {
	    if((G__value*)NULL==p_ifunc->para_default[ifn][itemp] && 
	       itemp>libp->paran) {
	      ipara = 0;
	    }
#ifndef G__FONS41
	    else if (p_ifunc->para_default[ifn][itemp] && itemp>=libp->paran) {
	      ipara = 2; /* I'm not sure what this is, Fons. */
	    }
#endif
	    else {   
	      ipara=G__param_match(p_ifunc->para_type[ifn][itemp]
				   ,p_ifunc->para_p_tagtable[ifn][itemp]
				   ,p_ifunc->para_default[ifn][itemp]
				   ,libp->para[itemp].type
				   ,libp->para[itemp].tagnum
				   ,&(libp->para[itemp])
				   ,libp->parameter[itemp]
				   ,funcmatch
				   ,p_ifunc->para_nu[ifn]-itemp-1
#ifndef G__OLDIMPLEMENTATION1120
				   ,p_ifunc->para_reftype[ifn][itemp]
#endif
#ifndef G__OLDIMPLEMENTATION1208
				   ,p_ifunc->para_isconst[ifn][itemp]
#endif
#ifndef G__OLDIMPLEMENTATION1250
				   ,p_ifunc->isexplicit[ifn]
#endif
				   );
	    }
	    switch(ipara) {
	    case 2: /* default parameter */
#ifdef G__ASM_DBG
	      if(G__asm_dbg) {
		fprintf(G__serr," default%d %c tagnum%d %d : %c tagnum%d %d\n"
			,itemp
			,p_ifunc->para_type[ifn][itemp]
			,p_ifunc->para_p_tagtable[ifn][itemp]
			,p_ifunc->para_default[ifn][itemp]
			,libp->para[itemp].type
			,libp->para[itemp].tagnum
			,funcmatch);
	      }
#endif
	      itemp=p_ifunc->para_nu[ifn]; /* end of this parameter */
	      break;
	    case 1: /* match this one, next parameter */
#ifdef G__ASM_DBG
	      if(G__asm_dbg) {
		fprintf(G__serr," match%d %c tagnum%d %d : %c tagnum%d %d\n"
			,itemp
			,p_ifunc->para_type[ifn][itemp]
			,p_ifunc->para_p_tagtable[ifn][itemp]
			,p_ifunc->para_default[ifn][itemp]
			,libp->para[itemp].type
			,libp->para[itemp].tagnum
			,funcmatch);
	      }
#endif
#ifndef G__OLDIMPLEMENTATION1003
	      if(G__EXACT!=funcmatch)
		G__warn_refpromotion(p_ifunc,ifn,itemp,libp);
#endif
	      ++itemp; /* next function parameter */
	      break;
	    case 0: /* unmatch, next function */
#ifdef G__ASM_DBG
	      if(G__asm_dbg) {
		fprintf(G__serr," unmatch%d %c tagnum%d %d : %c tagnum%d %d\n"
			,itemp
			,p_ifunc->para_type[ifn][itemp]
			,p_ifunc->para_p_tagtable[ifn][itemp]
			,p_ifunc->para_default[ifn][itemp]
			,libp->para[itemp].type
			,libp->para[itemp].tagnum
			,funcmatch);
	      }
#endif
	      itemp=p_ifunc->para_nu[ifn]; 
	      /* exit from while loop */
	      break;
	    }
	    
	  } /* end of while(itemp<p_ifunc->para_nu[ifn]) */
	  if(ipara==0) { /* parameter doesn't match */
	    ++ifn; /* next function */
	  }
	}
      }
      else {  /* funcname doesn't match */
	++ifn;
      }
    }  /* end of while((ipara==0))&&(ifn<p_ifunc->allifunc)) */
    /******************************************************************
     * next page of interpreted function list
     *******************************************************************/
    if(ifn>=p_ifunc->allifunc) {
      p_ifunc=p_ifunc->next;
      ifn=0;
    }
    else {
      break; /* get out from while(p_ifunc) loop */
    }
  } /* end of while(p_ifunc) */


  *pifn = ifn;
  return(p_ifunc);
}

/**************************************************************************
* G__get_ifunchandle_base
*
*
**************************************************************************/
struct G__ifunc_table *G__get_ifunchandle_base(funcname,libp,hash,p_ifunc,pifn
					       ,poffset
					       ,access,funcmatch)
char *funcname;
struct G__param *libp;
int hash;
struct G__ifunc_table *p_ifunc; 
long *pifn;
long *poffset;
int access;
int funcmatch;
{
  int tagnum;
  struct G__ifunc_table *ifunc;
  int basen=0;
  struct G__inheritance *baseclass;

  /* Search for function */
  *poffset = 0;
  ifunc=G__get_ifunchandle(funcname,libp,hash,p_ifunc,pifn,access,funcmatch);
  if(ifunc) return(ifunc);

  /* Search for base class function if member function */
  tagnum = p_ifunc->tagnum;
  if(-1!=tagnum) {
    baseclass = G__struct.baseclass[tagnum];
    while(basen<baseclass->basen) {
      if(baseclass->baseaccess[basen]&G__PUBLIC) {
#ifdef G__VIRTUALBASE
	/* Can not handle virtual base class member function for ERTTI
	 * because pointer to the object is not given  */
#endif
	*poffset = baseclass->baseoffset[basen];
	p_ifunc = G__struct.memfunc[baseclass->basetagnum[basen]];
	ifunc=G__get_ifunchandle(funcname,libp,hash,p_ifunc,pifn
				 ,access,funcmatch);
	if(ifunc) return(ifunc);
      }
      ++basen;
    }
  }

  /* Not found , ifunc=NULL */
  return(ifunc);
}

/**************************************************************************
* G__argtype2param()
*
*
**************************************************************************/
void G__argtype2param(argtype,libp)
char *argtype;
struct G__param *libp;
{
  char typenam[G__MAXNAME*2];
  int p=0;
  int c;
  char *endmark=",);";
  
  libp->paran=0;
  libp->para[0]=G__null;
#ifndef G__OLDIMPLEMENTATION834
  libp->next = (struct G__param*)NULL;
#endif

  do {
    c=G__getstream_template(argtype,&p,typenam,endmark);
    if(typenam[0]) {
      libp->para[libp->paran] = G__string2type(typenam);
      ++libp->paran;
    }
  } while(','==c);
}

/**************************************************************************
* G__get_methodhandle
*
*
**************************************************************************/
struct G__ifunc_table *G__get_methodhandle(funcname,argtype,p_ifunc
					   ,pifn,poffset)
char *funcname;
char *argtype;
struct G__ifunc_table *p_ifunc; 
long *pifn;
long *poffset;
{
  int match;
  struct G__ifunc_table *ifunc;
  struct G__param para;
  int hash;
  int temp;
#ifndef G__OLDIMPLEMENTATION1313
  struct G__funclist *funclist = (struct G__funclist*)NULL;
#endif

  G__argtype2param(argtype,&para);
  G__hash(funcname,hash,temp);

#ifndef G__OLDIMPLEMENTATION1313
  /* first, search for exact match */
  ifunc=G__get_ifunchandle_base(funcname,&para,hash,p_ifunc,pifn,poffset
				,G__PUBLIC_PROTECTED_PRIVATE,G__EXACT);
  if(ifunc) return(ifunc);

  /* if no exact match, try to instantiate template function */
  funclist = G__add_templatefunc(funcname,&para,hash,funclist,p_ifunc,0);
  if(funclist && funclist->rate==G__EXACTMATCH) {
    ifunc = funclist->ifunc;
    *pifn = funclist->ifn;
    G__funclist_delete(funclist);
    return(ifunc);
  }
  G__funclist_delete(funclist);

#endif /* 1313 */
  for(match=G__EXACT;match<=G__STDCONV;match++) {
    ifunc=G__get_ifunchandle_base(funcname,&para,hash,p_ifunc,pifn,poffset
#ifndef G__OLDIMPLEMENTATION912
				  ,G__PUBLIC_PROTECTED_PRIVATE
#else
				  ,G__PUBLIC
#endif
				  ,match);
    if(ifunc) return(ifunc);
  }
  return(ifunc);
}



/*
 * Local Variables:
 * c-tab-always-indent:nil
 * c-indent-level:2
 * c-continued-statement-offset:2
 * c-brace-offset:-2
 * c-brace-imaginary-offset:0
 * c-argdecl-indent:0
 * c-label-offset:-2
 * compile-command:"make -k"
 * End:
 */
