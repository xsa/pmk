/* $Id$ */

/* Public Domain */

/* CPU ID detection test */

#include <sys/utsname.h>
#include "../compat/pmk_sys_types.h"
#include <stdio.h>
#include <stdlib.h>

#include "../common.c"
#include "../compat.c"
#include "../detect_cpu.c"
#include "../detect_cpu_asm.h"
#include "../dynarray.c"
#include "../hash.c"
#include "../parse.c"
#include "../pmk_obj.c"

#define PMKCPU_DATA	"../data/pmkcpu.dat"

#define MASK_X86_CPU_EXTFAM	0x0ff00000
#define MASK_X86_CPU_EXTMOD	0x000f0000
#define MASK_X86_CPU_TYPE	0x0000f000
#define MASK_X86_CPU_FAMILY	0x00000f00
#define MASK_X86_CPU_MODEL	0x000000f0


int main(void) {
	char		*pstr;
	hkeys	*phk;
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

	pstr = check_cpu_arch(utsname.machine, pdata); /* check */
	printf("arch = '%s'\n", pstr);

	pht = arch_wrapper(pdata, pstr);
	if (pht != NULL) {
		phk = hash_keys(pht);
		if (phk != NULL) {
			for(i = 0 ; i < phk->nkey ; i++) {
				/*pstr = po_get_str(hash_get(pht, phk->keys[i]));*/
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

