/* $Id$ */

/* Public Domain */

/* CPU ID detection test */

#include <sys/types.h>
#include <stdio.h>
#include <stdlib.h>

#include "../common.c"
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
	int		 ui;
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

#ifdef ARCH_X86
	printf("ARCH_X86 is defined.\n");

	cpuidflag = x86_check_cpuid_flag();
	printf("cpuid flag = %x\n", cpuidflag);
	if (cpuidflag == 0)
		exit(EXIT_SUCCESS);

	x86_get_cpuid_data(&x86cc);

	printf("cpu vendor = '%s'\n", x86cc.vendor);


	pstr = x86_get_std_cpu_vendor(pdata, x86cc.vendor);
	printf("standard cpu vendor = '%s'\n", pstr);


	if (x86cc.cpuname != NULL)
		printf("cpu name = '%s'\n", x86cc.cpuname);


	if (x86cc.family != 15) {
		printf("family = %d\n", x86cc.family);
	} else {
		printf("extended family = %d\n", x86cc.extfam);
	}

	printf("model = %d\n", x86cc.model);


#else
	printf("Arch not supported.\n");
#endif
	prsdata_destroy(pdata);


	return(0);
}

