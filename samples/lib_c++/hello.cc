/* $Id$ */

#include <iostream>

#include "libhello.hh"

using namespace std;

int main(void) {
	print_hello();
	cout << " ";
	print_world();
	cout << " !" << endl;

	return(0);
}
