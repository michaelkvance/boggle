#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

#include "dict.h"

#define DICT_NODE_NUM_CHILDREN 26

#define DICT_NODE_FLAG_WORD 0x1
#define DICT_NODE_FLAG_MARK 0x2

#define DICT_CHILD_IDX(c) ((c) - 'A')

#define DAWG_NODE_FLAG_WORD DICT_NODE_FLAG_WORD
#define DAWG_NODE_FLAG_MARK DICT_NODE_FLAG_MARK

#define DAWG_CHILD_IDX(c) ((c) - 'A')

#define DAWG_NODE_CHILDLESS 0xfffff

typedef struct _dict_node_t {
	int children[DICT_NODE_NUM_CHILDREN];
	unsigned int set;
	unsigned int flags;
} dict_node_t;

struct _dict_t {
	dict_node_t* nodes;
	unsigned int size;
	unsigned int avail;
	unsigned int root;
	unsigned int heap;
};

typedef struct _dawg_node_t {
	unsigned int letter : 5;
	unsigned int flags : 2;
	unsigned int childcnt : 5;
	unsigned int childidx : 20;
} dawg_node_t;

struct _dawg_t {
	dawg_node_t* nodes;
	unsigned int size;
	unsigned int avail;
	unsigned int root;
	unsigned int heap;
};

dict_t* dict_new( void )
{
	dict_t *dict;

	dict = (dict_t*)malloc( sizeof( dict_t ) );
	memset( dict, 0, sizeof( dict_t ) );
	dict->heap += sizeof( dict_t );

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

	node->set = 0;
	node->flags = 0;
}

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
		node->set += 1;
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
		return ( node->flags & DICT_NODE_FLAG_WORD ) ? node - dict->nodes : -1;
	}

	idx = node->children[DICT_CHILD_IDX(c)];

	if( idx < 0 )
	{
		return -1;
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

int dict_mark( dict_t* dict, int idx )
{
	dict_node_t* node;
	int old;

	node = &dict->nodes[idx];
	old = node->flags;
	node->flags |= DICT_NODE_FLAG_MARK;

	return ( old & DICT_NODE_FLAG_MARK );
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
	dict->heap += dict->size * sizeof( dict_node_t );
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

void dict_analyze( dict_t* dict )
{
	int empty;
	int i;
	int nonempty;

	printf( "dict->heap = %d\n", dict->heap );
	printf( "dict->size = %d (%ld) dict->avail = %d (%ld)\n", dict->size, dict->size * sizeof( dict_node_t ), dict->avail, dict->avail * sizeof( dict_node_t ) );

	nonempty = 0;

	for( i = 0; i < dict->avail; i++ )
	{
		nonempty += dict->nodes[i].set;
	}

	empty = dict->avail * 26 - nonempty;
	printf( "dict has %d empty and %d non-empty child indices (%ld wasted bytes)\n", empty, nonempty, empty * sizeof(int) );
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
		dict->nodes = final;
	}
}

dawg_t* dawg_new( void )
{
	dawg_t* dawg;

	dawg = (dawg_t*)malloc( sizeof( dawg_t ) );
	memset( dawg, 0, sizeof( dawg_t ) );
	dawg->heap += sizeof( dawg_t );

	return dawg;
}

void dawg_delete( dawg_t *dawg )
{
	free( dawg->nodes );
	free( dawg );
}

void dict_finalize_to_dawg_r( dict_t* dict, dict_node_t* inode, dawg_t* dawg, dawg_node_t* anode )
{
	unsigned int i;
	dict_node_t* ichild;
	dawg_node_t* achild;

	anode->childidx = dawg->avail;

	for( i = 0; i < DICT_NODE_NUM_CHILDREN; i++ )
	{
		if( inode->children[i] >= 0 )
		{
			achild = &dawg->nodes[dawg->avail++];
			assert( dawg->avail <= dawg->size );
			achild->letter = i;
			achild->flags = dict->nodes[inode->children[i]].flags;
			anode->childcnt += 1;
		}
	}

	assert( anode->childcnt == inode->set );

	if( anode->childcnt == 0 )
	{
		anode->childidx = DAWG_NODE_CHILDLESS;
		return;
	}

	for( i = 0; i < anode->childcnt; i++ )
	{
		achild = &dawg->nodes[anode->childidx + i];
		ichild = &dict->nodes[inode->children[achild->letter]];
		dict_finalize_to_dawg_r( dict, ichild, dawg, achild );
	}
}

void dict_finalize_to_dawg( dict_t* dict, dawg_t* dawg )
{
	unsigned int i;
	dict_node_t* inode;
	unsigned int nonempty;

	nonempty = 0;

	for( i = 0; i < dict->avail; i++ )
	{
		inode = &dict->nodes[i];
		nonempty += inode->set;
	}

	dawg->size = dict->avail;
	dawg->nodes = (dawg_node_t*)malloc( sizeof( dawg_node_t ) * dawg->size );
	memset( dawg->nodes, 0, sizeof( dawg_node_t ) * dawg->size );
	dawg->heap += sizeof( dawg_node_t ) * dawg->size;
	dawg->root = 0;
	dawg->avail += 1;

	dict_finalize_to_dawg_r( dict, &dict->nodes[dict->root], dawg, &dawg->nodes[dawg->root] );
}

static int dawg_find_r( dawg_t* dawg, dawg_node_t* node, const char* word )
{
	char c;
	unsigned int i;
	dawg_node_t* children;
	dawg_node_t* child;

	c = *word;

	if( c == '\0' )
		return node->flags & DAWG_NODE_FLAG_WORD ? node - dawg->nodes : -1;

	children = &dawg->nodes[node->childidx];
	child = NULL;

	for( i = 0; i < node->childcnt; i++ )
	{
		if ( children[i].letter == DAWG_CHILD_IDX(c) )
		{
			child = &children[i];
			break;
		}
	}

	if ( child )
		return dawg_find_r( dawg, child, word + 1 );
	else
		return -1;
}

int dawg_find( dawg_t* dawg, const char* word )
{
	dawg_node_t* node;

	node = &dawg->nodes[dawg->root];

	return dawg_find_r( dawg, node, word );
}

int dawg_mark( dawg_t* dawg, int idx )
{
	dawg_node_t* node;
	int old;

	node = &dawg->nodes[idx];
	old = node->flags;
	node->flags |= DAWG_NODE_FLAG_MARK;

	return ( old & DAWG_NODE_FLAG_MARK );
}

int dawg_root( dawg_t* dawg )
{
	return dawg->root;
}

int dawg_child( dawg_t* dawg, int idx, char c )
{
	int child;
	dawg_node_t* node;
	dawg_node_t* children;
	int i;

	node = &dawg->nodes[idx];
	children = &dawg->nodes[node->childidx];
	child = -1;

	for( i = 0; i < node->childcnt; i++ )
	{
		if( children[i].letter == DAWG_CHILD_IDX(c) )
		{
			child = &children[i] - dawg->nodes;
			break;
		}
	}

	return child;
}

void dawg_analyze( dawg_t* dawg )
{
	printf( "dawg->heap = %d\n", dawg->heap );
	printf( "dawg->size = %d, dawg->avail = %d\n", dawg->size, dawg->avail );
}
