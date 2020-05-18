constexpr auto INITIAL_HASH_MAP_SIZE = 256;
constexpr auto VACANT_HASH_KEY_SENTINEL = -1;

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
void HashTableInsert(HashTable<K, V> *table, const K &key, const V &value)
{
	Assert(ArrayLength(table->buckets));
	auto hash = Hash(key);
	auto index = hash % ArrayLength(table->buckets);
	for (auto i = (index + 1) % ArrayLength(table->buckets); i != index; i = (i + 1) % ArrayLength(table->buckets))
	{
		if (table->buckets[index].keyHash == VACANT_HASH_KEY_SENTINEL || table->buckets[index].keyHash == hash)
		{
			table->buckets[index].keyHash = hash;
			table->buckets[index].value = value;
			return;
		}
	}
	// @TODO: Grow the table? Or the table should already have grown and we never get to this case.
	Abort("Ran out of room in the hash table...");
}

template <typename V>
void HashTableInsert(HashTable<String, V> *table, const String &key, const V &value)
{
	HashTableInsert<String, V>(table, key, value);
}

template <typename K, typename V>
bool HashTableInsertIfNonExistent(HashTable<K, V> *table, const K &key, const V &value)
{
	auto hash = Hash(key);
	auto index = hash % ArrayLength(table->buckets);
	for (auto i = index + 1 % ArrayLength(table->buckets); i != index; i = index + 1 % ArrayLength(table->buckets))
	{
		if (table->buckets[index].keyHash == VACANT_HASH_KEY_SENTINEL)
		{
			table->buckets[index].keyHash = hash;
			table->buckets[index].value = value;
			return true;
		}
		else if (table->buckets[index].keyHash == hash)
		{
			return false;
		}
	}
	// @TODO: Grow the table? Or the table should already have grown and we never get to this case.
	Abort("Ran out of room in the hash table...");
}

template <typename K, typename V>
V *HashTableLookup(HashTable<K, V> *table, const K &key)
{
	auto tableSize = ArrayLength(table->buckets);
	if (tableSize == 0)
	{
		return NULL;
	}
	auto hash = Hash(key);
	auto index = hash % tableSize;
	auto lastIndex = index;
	do
	{
		if (table->buckets[index].keyHash == VACANT_HASH_KEY_SENTINEL)
		{
			return NULL;
		}
		else if (table->buckets[index].keyHash == hash)
		{
			return &table->buckets[index].value;
		}
		index = (index + 1) % tableSize;
	} while (index != lastIndex);
	return NULL;
}

template <typename K, typename V>
void HashTableRemove(HashTable<K, V> *table, const K &key)
{
	// @TODO
}