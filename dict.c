#include <stdio.h>

#define DICT_FLAG_ALLOCED 0x1

typedef struct _dict_node_t {
	int leafs[26];
	unsigned int flags;
} dict_node_t;

typedef struct _dict_t {
	dict_node_t* nodes;
	dict_node_t* root;
} dict_t;

void dict_init( dict_t* dict )
{

}

int main( int argc, char* argv[] )
{
	FILE* in;
	FILE* out;

	if( argc < 3 )
	{
		fprintf( stderr, "Fool!\n" );
		return -1;
	}

	in = fopen( argv[1], "r" );

	if( in == NULL )
	{
		fprintf( stderr, "Dunce! I can't open '%s'\n", argv[1] );
		return -1;
	}

	out = fopen( argv[2], "w" );

	if( out == NULL )
	{
		fclose( in );
		fprintf( stderr, "Ass! I can't open '%s'\n", argv[2] );
		return -1;
	}

	fclose( out );
	fclose( in );

	return 0;
}
