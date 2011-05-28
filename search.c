#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <assert.h>

#define elementsof(x) (sizeof(x)/sizeof(x[0]))

int search( const char** haystack, int size, const char* needle )
{
	int low;
	int high;
	int mid;
	int dir;

	assert( size > 0 );

	low = 0;
	high = size - 1;

	while( low <= high )
	{
		mid = ( low + high ) / 2;
		dir = strcmp( needle, haystack[mid] );

		if( dir < 0 )
			high = mid - 1;
		else if( dir > 0 )
			low = mid + 1;
		else
			return mid;
	}

	return -1;
}

int main( int argc, char* argv[] )
{
	const char* test[] = {
		"aaa", "bbb", "ccc", "ddd", "eee", "fff", "ggg", "hhh", "iii", "jjj"
	};
	int r;
	int i;

	r = search( test, 1, "aaa" );
	assert( r >= 0 );
	r = search( test, 1, "bbb" );
	assert( r < 0 );

	r = search( test, 2, "aaa" );
	assert( r >= 0 );
	r = search( test, 2, "bbb" );
	assert( r >= 0 );
	r = search( test, 2, "ccc" );
	assert( r < 0 );

	r = search( test, 3, "aaa" );
	assert( r >= 0 );
	r = search( test, 3, "bbb" );
	assert( r >= 0 );
	r = search( test, 3, "ccc" );
	assert( r >= 0 );
	r = search( test, 3, "ddd" );
	assert( r < 0 );

	r = search( test, 4, "aaa" );
	assert( r >= 0 );
	r = search( test, 4, "bbb" );
	assert( r >= 0 );
	r = search( test, 4, "ccc" );
	assert( r >= 0 );
	r = search( test, 4, "ddd" );
	assert( r >= 0 );
	r = search( test, 4, "eee" );
	assert( r < 0 );

	r = search( test, 5, "aaa" );
	assert( r >= 0 );
	r = search( test, 5, "bbb" );
	assert( r >= 0 );
	r = search( test, 5, "ccc" );
	assert( r >= 0 );
	r = search( test, 5, "ddd" );
	assert( r >= 0 );
	r = search( test, 5, "eee" );
	assert( r >= 0 );
	r = search( test, 5, "fff" );
	assert( r < 0 );

	for( i = 0; i < elementsof( test ); ++i )
	{
		r = search( test, 10, test[i] );
		assert( r >= 0 );
	}

	r = search( test, 10, "xxx" );
	assert( r < 0 );

	return 0;
}
