#include "boggle.h"

#include <stdlib.h>

int main( int argc, char* argv[] )
{
	boggle_t* bog;
	const char best[] = { 'S', 'E', 'R', 'S',
			      'P', 'A', 'T', 'G',
			      'L', 'I', 'N', 'E',
			      'S', 'E', 'R', 'S' };
	const char test[] = { 'D', 'E', 'A', 'D',
			      'X', 'X', 'D', 'X',
			      'X', 'X', 'X', 'X',
			      'X', 'X', 'X', 'X' };

	/* constant seed for debug-ability */
	srand( 0xDEADBEEF );
	/* random seed */
	/* srand( time( NULL ) ); */

	bog = boggle_new();
	boggle_init_static( bog, test, 4, 4 );
	boggle_solve( bog );
	boggle_fini( bog );
	boggle_delete( bog );

	bog = boggle_new();
	boggle_init( bog );
	boggle_solve( bog );
	boggle_fini( bog );
	boggle_delete( bog );

	bog = boggle_new();
	boggle_init_static( bog, best, 4, 4 );
	boggle_solve( bog );
	boggle_fini( bog );
	boggle_delete( bog );

	return 0;
}
