/* $Id$ */

/* Public Domain */

/* c parser test */
#include <stdio.h>
#include <sys/param.h>

#include "../parse_lang.h"

#define FNAME	"plang_test.c"

void ppro(char *pstr, prseng_t *ppe) {
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
}

void proc(char *pstr, prseng_t *ppe) {
	printf("PROC: found function call '%s'\n", pstr);
}

void type(char *pstr, prseng_t *ppe) {
	printf("TYPE: found type '%s'\n", pstr);
}

int main() {
	FILE		*fp;
	prs_cmn_t	 pcmn;

	pcmn.func_ppro = &ppro;
	pcmn.func_proc = &proc;
	pcmn.func_type = &type;

	fp = fopen(FNAME, "r");
	if (fp == NULL) {
		printf("cannot open '%s'\n", FNAME);
	}

	prs_c_file(&pcmn, fp);

	fclose(fp);
}

