/* $Id$ */

/* Public Domain */

/* path tools test */

#include <sys/param.h>
#include <stdio.h>

#include "../compat/compat.h"
#include "../compat/pmk_stdbool.h"
#include "../pathtools.h"

char	*chk_paths[4] = {
		"/usr/./local/bin/../lib//",
		"/usr/../../local",
		"/usr////lib/..",
		"/var/."
	},
	*rel_paths[4][2] = {
		{"/home/me/tmp/src/youpi", "/home/me/tmp/build/"},
		{"/usr/local/lib/", "/var/named"},
		{"/","/etc/ppp"},
		{"/usr/local", "/usr/local"}
	};




int main(void) {
	char	buf[MAXPATHLEN];
	int	i;

	printf("* Checking paths :\n");
	for (i = 0 ; i < 4 ; i++) {
		chkpath(chk_paths[i], buf); /* XXX check ? */
		printf("\t'%s' => '%s'\n", chk_paths[i], buf);
	}

	printf("* Test relative paths :\n");
	for (i = 0 ; i < 4 ; i++) {
		printf("\tfrom = '%s', to = '%s'\n", rel_paths[i][0], rel_paths[i][1]);
		relpath(rel_paths[i][0], rel_paths[i][1], buf);
		printf("\trelpath = '%s'\n\n", buf);
	}

	return(0);
}
