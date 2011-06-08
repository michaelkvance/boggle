#include "boggle.h"

#include <stdlib.h>
#include <stdio.h>
#include <mach/mach_time.h>

static mach_timebase_info_data_t tb_info;

void timer_init()
{
	mach_timebase_info( &tb_info );
}

uint64_t timer_ticks()
{
	return mach_absolute_time();
}

uint64_t timer_elapsed_ns( uint64_t then, uint64_t now )
{
	uint64_t raw;
	uint64_t delta;

	raw = now - then;
	delta = raw * tb_info.numer;
	delta = delta / tb_info.denom;

	return delta;
}

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
	uint64_t then;
	uint64_t now;
	uint64_t delta;

	timer_init();
	/* constant seed for debug-ability */
	srand( 0xDEADBEEF );
	/* random seed */
	/* srand( time( NULL ) ); */

	bog = boggle_new();
	boggle_init_static( bog, test, 4, 4 );
	then = timer_ticks();
	boggle_solve( bog );
	now = timer_ticks();
	delta = timer_elapsed_ns( then, now );
	printf( "solve time: %lld ns\n", delta );
	boggle_fini( bog );
	boggle_delete( bog );

	bog = boggle_new();
	boggle_init( bog );
	then = timer_ticks();
	boggle_solve( bog );
	now = timer_ticks();
	delta = timer_elapsed_ns( then, now );
	printf( "solve time: %lld ns\n", delta );
	boggle_fini( bog );
	boggle_delete( bog );

	bog = boggle_new();
	boggle_init_static( bog, best, 4, 4 );
	then = timer_ticks();
	boggle_solve( bog );
	now = timer_ticks();
	delta = timer_elapsed_ns( then, now );
	printf( "solve time: %lld ns\n", delta );
	boggle_fini( bog );
	boggle_delete( bog );

	return 0;
}
