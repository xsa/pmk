/* $Id$ */

/* Public Domain */

/* path tools test */

#include <sys/param.h>
#include <stdio.h>

#include "../compat.h"
#include "../compat/pmk_stdbool.h"
#include "../pathtools.h"

char	*chk_paths[] = {
		"/usr/./local/bin/../lib//",
		"/usr/../../local",
		"/usr////lib/..",
		"./youpi/youpla/../paf",
		"blah/../odidonk",
		"/var/."
	},
	*rel_paths[][2] = {
		{"/home/me/tmp/src/youpi", "/home/me/tmp/build/"},
		{"/usr/local/lib/", "/var/named"},
		{"/","/etc/ppp"},
		{"/usr/local", "/usr/local"}
	};


int main(void) {
	char			buf[MAXPATHLEN];
	unsigned int	i,
					s;

	s = (unsigned int) (sizeof(chk_paths) / sizeof(char *));

	printf("* Checking paths :\n");
	for (i = 0 ; i < s ; i++) {
		chkpath(chk_paths[i], buf); /* XXX check ? */
		printf("\t'%s' => '%s'\n", chk_paths[i], buf);
	}

	s = (unsigned int) (sizeof(rel_paths) / (sizeof(char *) * 2));

	printf("* Test relative paths :\n");
	for (i = 0 ; i < s ; i++) {
		printf("\tfrom = '%s', to = '%s'\n", rel_paths[i][0], rel_paths[i][1]);
		relpath(rel_paths[i][0], rel_paths[i][1], buf);
		printf("\trelpath = '%s'\n\n", buf);
	}

	return(0);
}

