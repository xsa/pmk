/* $Id$ */

/* Public Domain */

/* c parser test */
#include <sys/param.h>
#include <stdlib.h>

#include "../compat/pmk_stdio.h"
#include "../compat/pmk_string.h"
#include "../parse_lang.h"

bool ppro(void *, char *, prseng_t *);
bool proc(void *, char *, prseng_t *);
bool decl(void *, char *, prseng_t *);
bool type(void *, char *, prseng_t *);
	
	
bool ppro(void *data, char *pstr, prseng_t *ppe) {
	char	iname[MAXPATHLEN],
		c;

	printf("PPRO: found preprocessor directive '%s'\n", pstr);

	if (strncmp(pstr, RKW_PP_INCL, strlen(pstr) + 1) == 0) {
		prs_c_skip(ppe);

		c = prseng_get_char(ppe);

		prseng_next_char(ppe); /* XXX */

		prseng_get_idtf(ppe, iname, sizeof(iname), PRS_C_IDTF_FNAME);

		switch(c) {
			case '"' :
				printf("PPRO: local include '%s'\n", iname);
				break;

			case '<' :
				printf("PPRO: system include '%s'\n", iname);
				break;
		}

	}

	prs_c_line_skip(ppe);

	return(true);
}

bool proc(void *data, char *pstr, prseng_t *ppe) {
	printf("PROC: found function call '%s'\n", pstr);

	return(true);
}

bool decl(void *data, char *pstr, prseng_t *ppe) {
	printf("PROC: found function declarator '%s'\n", pstr);

	return(true);
}

bool type(void *data, char *pstr, prseng_t *ppe) {
	printf("TYPE: found type '%s'\n", pstr);

	return(true);
}

int main(int argc, char **argv) {
	FILE		*fp;
	prs_cmn_t	 pcmn;

	if (argc != 2) {
		printf("Expecting a filename as argument\n");
		exit(1); /* XXX */
	}

	pcmn.func_ppro = &ppro;
	pcmn.func_proc = &proc;
	pcmn.func_decl = &decl;
	pcmn.func_type = &type;
	pcmn.data = NULL;

	fp = fopen(argv[1], "r");
	if (fp == NULL) {
		printf("cannot open '%s'\n", argv[1]);
	} else {
		prs_c_file(&pcmn, fp);
	}

	fclose(fp);

	return(0);
}

