#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <ctype.h>

#define DICT_NODE_NUM_CHILDREN 26

#define DICT_NODE_FLAG_WORD 0x1

typedef struct _dict_node_t {
	int children[DICT_NODE_NUM_CHILDREN];
	int flags;
} dict_node_t;

typedef struct _dict_t {
	dict_node_t* nodes;
	int size;
	int avail;
	int root;
} dict_t;

dict_t* dict_new( void )
{
	dict_t *dict;

	dict = (dict_t*)malloc( sizeof( dict_t ) );
	memset( dict, 0, sizeof( dict_t ) );

	return dict;
}

void dict_delete( dict_t* dict )
{
	free( dict->nodes );
	free( dict );
}

void dict_node_init( dict_t* dict, dict_node_t* node )
{
	int i;

	for( i = 0; i < DICT_NODE_NUM_CHILDREN; i++ )
	{
		node->children[i] = -1;
	}

	node->flags = 0;
}

#define DICT_CHILD_IDX(c) (toupper((c)) - 'A')

static void dict_add_r( dict_t* dict, dict_node_t* node, const char* word )
{
	char c;
	int idx;
	dict_node_t* child;

	c = *word;

	if( c == '\0' )
	{
		node->flags |= DICT_NODE_FLAG_WORD;
		return;
	}

	if( node->children[DICT_CHILD_IDX(c)] < 0 )
	{
		assert( dict->avail < dict->size );
		idx = dict->avail;
		child = &dict->nodes[idx];
		dict->avail += 1;
		dict_node_init( dict, child );
		node->children[DICT_CHILD_IDX(c)] = idx;
	}

	idx = node->children[DICT_CHILD_IDX(c)];
	assert( idx >= 0 );
	assert( idx < dict->size );
	dict_add_r( dict, &dict->nodes[idx], word + 1 );
}

void dict_add( dict_t* dict, const char* word )
{
	dict_node_t* node;

	node = &dict->nodes[dict->root];

	dict_add_r( dict, node, word );
}

static int dict_find_r( dict_t* dict, dict_node_t* node, const char* word )
{
	char c;
	int idx;
	dict_node_t* child;

	c = *word;

	if( c == '\0' )
	{
		return node->flags & DICT_NODE_FLAG_WORD;
	}

	idx = node->children[DICT_CHILD_IDX(c)];

	if( idx < 0 )
	{
		return 0;
	}
	else
	{
		child = &dict->nodes[idx];
		return dict_find_r( dict, child, word + 1 );
	}
}

int dict_find( dict_t* dict, const char* word )
{
	dict_node_t* node;

	node = &dict->nodes[dict->root];

	return dict_find_r( dict, node, word );
}

void dict_init_file( dict_t* dict, const char* filename, int hint )
{
	FILE* fp;
	int nwords;
	int c;
	char token[64];
	char* parsed;
	int l;

	fp = fopen( filename, "r" );

	if( !fp )
	{
		fprintf( stderr, "Boo!\n" );
		exit( -1 );
	}

	nwords = 0;

	while( !feof( fp ) )
	{
		c = fgetc( fp );
		nwords += ( c == '\n' ) ? 1 : 0;
	}

	assert( dict->avail == 0 );
	dict->size = hint;
	dict->nodes = (dict_node_t*)malloc( dict->size * sizeof( dict_node_t ) );
	dict->root = 0;
	dict_node_init( dict, &dict->nodes[dict->root] );
	dict->avail += 1;

	fseek( fp, SEEK_SET, 0 );

	while( !feof( fp ) )
	{
		parsed = fgets( token, sizeof( token ), fp );

		if( !parsed )
		{
			assert( !ferror( fp ) );
			break;
		}

		l = strlen( parsed );

		if( parsed[ l - 1 ] == '\n' )
		{
			parsed[ l - 1 ] = 0;
			l -= 1;
		}

		if( l < 3 )
		{
			continue;
		}

		dict_add( dict, parsed );
	}

	fclose( fp );
}

void dict_finalize( dict_t* dict )
{
	dict_node_t* final;

	if ( dict->avail == dict->size )
	{
		return;
	}

	final = (dict_node_t*)realloc( dict->nodes, dict->avail * sizeof( dict_node_t ) );
	if ( final )
	{
		printf( "shrinking dict from %d entries to %d entries\n", dict->size, dict->avail );
		dict->nodes = final;
	}
}

int main( int argc, char* argv[] )
{
	dict_t* dict;
	int r;

	dict = dict_new();
	//dict_init_file( dict, "yawl.dict" );
	dict_init_file( dict, "tiny.dict", 13 );
	r = dict_find( dict, "aaa" );
	assert( r );
	r = dict_find( dict, "aab" );
	assert( r );
	r = dict_find( dict, "aaaa" );
	assert( r );
	r = dict_find( dict, "bbb" );
	assert( r );
	r = dict_find( dict, "ababa" );
	assert( r );
	r = dict_find( dict, "a" );
	assert( !r );
	r = dict_find( dict, "aa" );
	assert( !r );
	r = dict_find( dict, "aabb" );
	assert( !r );
	r = dict_find( dict, "xxx" );
	assert( !r );
	dict_delete( dict );

	dict = dict_new();
	dict_init_file( dict, "yawl.dict", 1000000 );
	dict_finalize( dict );
	r = dict_find( dict, "overelaborate" );
	assert( r );
	r = dict_find( dict, "zdasfghk" );
	assert( !r );
	r = dict_find( dict, "zzz" );
	assert( r );
	r = dict_find( dict, "motherfuckermotherfucker" );
	assert( !r );
	dict_delete( dict );

	return 0;
}
