#include "cgic.h"
#include "ctemplate.h"

#include <stdio.h>
#include <stdlib.h>

typedef struct Table {
	char *name;
	char *pet;
} Table;

Table table[] = {
	{"harry","hedwig"},
	{"ron","scabbers"},
	{"hermione","crookshanks"}
};

int cgiMain() {

	cgiHeaderContentType("text/html");

	TMPL_varlist *vl=NULL;
	TMPL_loop *loop=NULL;
	TMPL_varlist *mainlist=NULL;

	char str[4];
	for(int i=0;i<3;i++) {
		sprintf(str,"%d",i);
		vl=TMPL_add_var(0,"id",str,"name",table[i].name,"pet",table[i].pet,0);
		loop=TMPL_add_varlist(loop,vl);
	}

	mainlist=TMPL_add_loop(0,"myloop",loop);

	TMPL_write("test.html",0,0,mainlist,stdout,stderr);
	TMPL_free_varlist(mainlist);

	return 0;
}
