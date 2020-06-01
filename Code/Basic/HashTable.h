#include "Code/Basic/Array.h"
#include "Code/Basic/Log.h"

const auto INITIAL_HASH_MAP_SIZE = 256;
const auto VACANT_HASH_KEY_SENTINEL = -1;

// @TODO: GROW THE HASH TABLE.

template <typename V>
struct KeyHashValuePair
{
	u64 keyHash = VACANT_HASH_KEY_SENTINEL;
	V value = {};
};

template <typename K, typename V>
struct KeyValuePair
{
	K key = {};
	V value = {};
};

template <typename K, typename V>
struct HashTable
{
	Array<KeyHashValuePair<V>> buckets = CreateArray<KeyHashValuePair<V>>(INITIAL_HASH_MAP_SIZE);
};

template <typename K, typename V>
void InsertIntoHashTable(HashTable<K, V> *table, const K &key, const V &value)
{
	Assert(table->buckets.count > 0);

	auto keyHash = Hash(key);
	auto startIndex = keyHash % table->buckets.count;
	auto index = startIndex;
	do
	{
		if (table->buckets[index].keyHash == VACANT_HASH_KEY_SENTINEL || table->buckets[index].keyHash == keyHash)
		{
			table->buckets[index].keyHash = keyHash;
			table->buckets[index].value = value;
			return;
		}
		index = (index + 1) % table->buckets.count;
	} while(index != startIndex);

	// @TODO: Grow the table? Or the table grew already and we should never get to this case.
	Abort("Ran out of room in the hash table...");
}

template <typename V>
void InsertIntoHashTable(HashTable<String, V> *table, const String &key, const V &value)
{
	InsertIntoHashTable<String, V>(table, key, value);
}

template <typename K, typename V>
bool InsertIntoHashTableIfNonExistent(HashTable<K, V> *table, const K &key, const V &value)
{
	Assert(table->buckets.count > 0);

	auto keyHash = Hash(key);
	auto startIndex = keyHash % table->buckets.count;
	auto index = startIndex;
	do
	{
		if (table->buckets[index].keyHash == VACANT_HASH_KEY_SENTINEL)
		{
			table->buckets[index].keyHash = keyHash;
			table->buckets[index].value = value;
			return true;
		}
		index = (index + 1) % table->buckets.count;
	} while(index != startIndex);
	return false;
}

template <typename K, typename V>
V *LookupInHashTable(HashTable<K, V> *table, const K &key)
{
	auto bucketCount = table->buckets.count;
	if (bucketCount == 0)
	{
		return NULL;
	}
	auto keyHash = Hash(key);
	auto index = keyHash % bucketCount;
	auto lastIndex = index;
	do
	{
		if (table->buckets[index].keyHash == VACANT_HASH_KEY_SENTINEL)
		{
			return NULL;
		}
		else if (table->buckets[index].keyHash == keyHash)
		{
			return &table->buckets[index].value;
		}
		index = (index + 1) % bucketCount;
	} while (index != lastIndex);
	return NULL;
}

template <typename K, typename V>
void RemoveFromHashTable(HashTable<K, V> *table, const K &key)
{
	// @TODO
}
