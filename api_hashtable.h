//TODO(Ray):DELETE
#if 0

#if !defined(API_HASHTABLE_H)
#include "types.h"
#include "api_vector.h"
#define MAX_ALLOWED_COLLISIONS 4

//NOTE(Ray):At the moment this hashtable only allows for fixed size tables.
//resizing not implemented yet.
struct HashKeyEntry
{
	u64 value_index;
	char* key;
	u32 collision_count;
};

struct HashValueEntry
{
	void* value;
	struct HashKeyEntry hash_collisions[MAX_ALLOWED_COLLISIONS];

};


struct HashTable
{
	vector keys;
	vector values;
};


HashTable CreateHashTable(u32 start_count);

u64 UHashMod(const char *Key, unsigned TableSize);

u64 HashFunction(HashTable* h_table,const char* in);

void AddElementToHashTable(HashTable* h_table,char* key,void* element);

#define GetElementByHash(Type,table,in) (Type*)GetElementByHash_(table,in)
void* GetElementByHash_(HashTable* h_table,char* in);

#ifdef YOYOIMPL

HashTable CreateHashTable(u32 start_count)
{
	HashTable result;
	result.keys = CreateVector(start_count,sizeof(HashKeyEntry));
	result.keys.allow_resize = false;
	result.values = CreateVector(start_count,sizeof(HashValueEntry));
	result.values.allow_resize = false;
	return result;
}

// Hashing function from Program 14.2 in the Sedgewick book
//http://cecilsunkure.blogspot.com/2012/07/hash-tables.html
u64 UHashMod(const char *Key, unsigned TableSize)
{
	u64 hash = 0;      // Initial value of hash
	u64 rand1 = 31415; // "Random" 1
	u64 rand2 = 27183; // "Random" 2

	while (*Key)
	{
		hash = hash * rand1;     // Multiply hash by random
		hash = (hash + *Key);    // Add in current char, keep within TableSize
		rand1 = (rand1 * rand2); // Update rand1 for next "random" number
		Key++;
	}
	return hash % TableSize;
}

 u64 HashFunction(HashTable* h_table,const char* in)
{
	u64 result;
	result = UHashMod(in, h_table->keys.TotalCount);
	return result;
}

 void AddElementToHashTable(HashTable* h_table,char* key,void* element)
{
	u64 hash_index = HashFunction(h_table,key);
	HashKeyEntry e{};
	e.collision_count = 0;
	e.key = key;
	e.value_index = hash_index;
	HashValueEntry v{};
	v.value = element;

	HashKeyEntry* lu = GetVectorElement(HashKeyEntry, &h_table->keys, hash_index);
	if(lu->key != nullptr)
	{
		//TODO(Ray):Handle collisions.
		Assert(false);
	}
	else
	{
		//TODO(Ray):Compare if this same keyelement is already added.
		//if yes return cant add because key already exists
		//if no add element
		SetVectorElement(&h_table->keys, hash_index, &e);
		SetVectorElement(&h_table->values, hash_index, &v);
	}
}

 void* GetElementByHash_(HashTable* h_table,char* in)
{
	void* result;
	u64 hash_index = HashFunction(h_table,in);
	HashKeyEntry* e = GetVectorElement(HashKeyEntry,&h_table->keys,hash_index);
	if(e->collision_count == 0)
	{
		HashValueEntry* ve = GetVectorElement(HashValueEntry,&h_table->values, hash_index);
		result = ve->value;
	}
	else
	{
		//Do string comparisons on key values till we get the proper entry.
	}
	return result;
}
#endif
#define API_HASHTABLE
#endif
#endif
