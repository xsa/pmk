#include <sys/types.h>
#include <stdio.h>

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
	uint32_t	 cputype;
	struct utsname	 utsname;

#ifdef ARCH_X86
	printf("ARCH_X86 is defined\n");
#else
	printf("ARCH_X86 is NOT defined\n");
#endif

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


	pstr = get_x86_std_cpu_vendor(pdata);
	printf("cpu vendor = '%s'\n", pstr);

	pstr = get_x86_cpu_name();
	printf("cpu name = '%s'\n", pstr);

	cputype = get_x86_cpu_type();

	ui = (unsigned int) ((cputype & MASK_X86_CPU_FAMILY) >> 8);
	printf("family = %d\n", ui);

	if (ui == 15) {
		ui = (unsigned int) ((cputype & MASK_X86_CPU_EXTFAM) >> 20);
		printf("extended family = %d\n", ui);
	}

	ui = (unsigned int) ((cputype & MASK_X86_CPU_MODEL) >> 4);
	printf("model = %d\n", ui);


	prsdata_destroy(pdata);


	return(0);
}

