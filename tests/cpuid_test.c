/* $Id$ */

/* Public Domain */

/* CPU ID detection test */

#include <sys/utsname.h>
#include "../compat/pmk_sys_types.h"
#include <stdio.h>
#include <stdlib.h>

#include "../common.h"
#include "../compat/compat.h"
#include "../detect_cpu.h"
#include "../detect_cpu_asm.h"
#include "../dynarray.h"
#include "../hash.h"
#include "../parse.h"
#include "../pmk_obj.h"

#define PMKCPU_DATA	"../data/pmkcpu.dat"

int main(void) {
	char		*pstr;
	hkeys		*phk;
	htable		*pht;
	int		 ui,
			 i;
	prsdata		*pdata;
	uint32_t	 cpuidflag,
			 cputype;
	struct utsname	 utsname;
	uint32_t	 buffer[13];

	if (uname(&utsname) == -1) {
		printf("uname failed.\n");
		exit(EXIT_FAILURE);
	}

	pdata = parse_cpu_data(PMKCPU_DATA);
	if (pdata == NULL) {
		/* XXX msg ? */
		exit(EXIT_FAILURE);
	}

	pstr = check_cpu_arch(utsname.machine, pdata); /* XXX check ? */
	printf("arch = '%s'\n", pstr);

	pht = arch_wrapper(pdata, pstr);
	if (pht != NULL) {
		phk = hash_keys(pht);
		if (phk != NULL) {
			for(i = 0 ; i < phk->nkey ; i++) {
				pstr = hash_get(pht, phk->keys[i]);
				printf("%s = '%s'\n", phk->keys[i], pstr);
			}

			hash_free_hkeys(phk);
		}
		hash_destroy(pht);
	}

	prsdata_destroy(pdata);


	return(0);
}

