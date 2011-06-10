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

void print_scores( int* counts, int* scores )
{
	int i;
	int total_count;
	int total_score;

	total_count = 0;
	total_score = 0;

	for( i = 0; i < 17; i++ )
	{
		if ( !counts[i] )
			continue;

		printf( "%8d %2d-letter words (%4d points)\n", counts[i], i, scores[i] );
		total_count += counts[i];
		total_score += scores[i];
	}

	printf( "%8d total points, %4d total words.\n", total_score, total_count );
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
	int counts[17];
	int scores[17];

	timer_init();
	/* constant seed for debug-ability */
	srand( 0xDEADBEEF );
	/* random seed */
	/* srand( time( NULL ) ); */

	bog = boggle_new();
	boggle_init_static( bog, test, 4, 4 );
	then = timer_ticks();
	boggle_solve( bog, counts, scores );
	now = timer_ticks();
	delta = timer_elapsed_ns( then, now );
	printf( "solve time: %lld ns\n", delta );
	print_scores( counts, scores );
	boggle_fini( bog );
	boggle_delete( bog );

	bog = boggle_new();
	boggle_init( bog );
	then = timer_ticks();
	boggle_solve( bog, counts, scores );
	now = timer_ticks();
	delta = timer_elapsed_ns( then, now );
	printf( "solve time: %lld ns\n", delta );
	print_scores( counts, scores );
	boggle_fini( bog );
	boggle_delete( bog );

	bog = boggle_new();
	boggle_init_static( bog, best, 4, 4 );
	then = timer_ticks();
	boggle_solve( bog, counts, scores );
	now = timer_ticks();
	delta = timer_elapsed_ns( then, now );
	printf( "solve time: %lld ns\n", delta );
	print_scores( counts, scores );
	boggle_fini( bog );
	boggle_delete( bog );

	return 0;
}
