#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <assert.h>

#define elementsof(x) (sizeof(x)/sizeof(x[0]))

int strcmp_wrap( const void* a, const void* b )
{
	const char* ap;
	const char** bp;
	int r;

	ap = (const char*) a;
	bp = (const char**) b;

	r = strcmp( ap, *bp );

	return r;
}

int search( const char** haystack, int size, const char* needle )
{
	const char* r;

	r = (const char*) bsearch( needle, haystack, size, sizeof( const char* ), strcmp_wrap );

	return r != NULL;
}

int search1( const char** haystack, int size, const char* needle )
{
	const char* test;
	int low;
	int high;
	int mid;
	int found;
	int dir;
	int dist;

	assert( size > 0 );

	low = 0;
	high = size;
	mid = high / 2;
	found = -1;

	while( 1 )
	{
		test = haystack[mid];
		dir = strcmp( needle, test );

		if( dir < 0 )
		{
			high = mid;
		}
		else if( dir > 0 )
		{
			low = mid;
		}
		else
		{
			found = mid;
			break;
		}

		dist = high - low;

		if( dist == 0 )
		{
			break;
		}

		mid = dist / 2;
	}

	return ( found != -1 );
}

int main( int argc, char* argv[] )
{
	const char* test[] = {
		"aaa", "bbb", "ccc", "ddd", "eee", "fff", "ggg", "hhh", "iii", "jjj"
	};
	int r;
	int i;

	r = search( test, 1, "aaa" );
	assert( r );
	r = search( test, 1, "bbb" );
	assert( !r );

	r = search( test, 2, "aaa" );
	assert( r );
	r = search( test, 2, "bbb" );
	assert( r );
	r = search( test, 2, "ccc" );
	assert( !r );

	r = search( test, 3, "aaa" );
	assert( r );
	r = search( test, 3, "bbb" );
	assert( r );
	r = search( test, 3, "ccc" );
	assert( r );
	r = search( test, 3, "ddd" );
	assert( !r );

	r = search( test, 4, "aaa" );
	assert( r );
	r = search( test, 4, "bbb" );
	assert( r );
	r = search( test, 4, "ccc" );
	assert( r );
	r = search( test, 4, "ddd" );
	assert( r );
	r = search( test, 4, "eee" );
	assert( !r );

	r = search( test, 5, "aaa" );
	assert( r );
	r = search( test, 5, "bbb" );
	assert( r );
	r = search( test, 5, "ccc" );
	assert( r );
	r = search( test, 5, "ddd" );
	assert( r );
	r = search( test, 5, "eee" );
	assert( r );
	r = search( test, 5, "fff" );
	assert( !r );

	for( i = 0; i < elementsof( test ); ++i )
	{
		r = search( test, 10, test[i] );
		assert( r );
	}

	r = search( test, 10, "xxx" );
	assert( !r );

	return 0;
}
