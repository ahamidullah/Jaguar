const auto HashTableVacantKeySentinel = u64{-1};
const auto HashTableDeletedKeySentinel = u64{-2};
const auto HashTableDefaultInitialLength = 16;
const auto HashTableMaxLoadFactor = 0.75f;

// @TODO: Initializer list.
// @TODO: Handle zero type.
//        How to handle empty hash? Maybe try to guess the hash function based on the type of key and abort if we fail.

typedef u64 (*HashProcedure)(void *);

template <typename K, typename V>
struct KeyValuePair
{
	K key;
	V value;
};

template <typename K, typename V>
struct HashTable
{
	Array<u64> hashes;
	Array<KeyValuePair<K, V>> buckets;
	HashProcedure hashProcedure;
	s64 count;
	f32 loadFactor;

	void Insert(K k, V v);
	V *Lookup(K k);
	V *LookupPointer(K k);
	void Remove(K k);
	void Clear();
	void ClearAndResize(s64 len);
	void Resize();
};

template <typename K, typename V>
HashTable<K, V> NewHashTableIn(Allocator *a, s64 len, HashProcedure hp)
{
	auto ht = HashTable<K, V>
	{
		.hashes = NewArrayIn<u64>(a, len),
		.buckets = NewArrayIn<KeyValuePair<K, V>>(a, len),
		.hashProcedure = hp,
	};
	SetMemory(&ht.buckets[0], len * sizeof(KeyValuePair<K, V>), 0);
	for (auto &h : ht.hashes)
	{
		h = HashTableVacantKeySentinel;
	}
	return ht;
}

template <typename K, typename V>
HashTable<K, V> NewHashTable(s64 len, HashProcedure hp)
{
	return NewHashTableIn(ContextAllocator(), len, hp);
}

template <typename K, typename V>
void HashTable<K, V>::Insert(HashTable<K, V> *h, K k, V v)
{
	// The user has to set the hash procedure, even if the HashTable is zero-initialized.
	Assert(this->hashProcedure);
	if (this->buckets.count > 0)
	{
		*this = NewHashTable(HashTableDefaultInitialLength, h->hashProcedure);
	}
	h->Reserve(h->count + 1);
	// @TODO: Would a two-pass solution be faster? First pass figure which keys need to be inserted
	// and their indices, then resize the table if necessary, then second pass does the insertions.
	auto hash = h->hashProcedure(&k);
	if (hash == HashTableVacantKeySentinel)
	{
		hash = 0;
	}
	else if (hash == HashTableDeletedKeySentinel)
	{
		hash = 1;
	}
	auto startIndex = hash % h->buckets.count;
	auto index = startIndex;
	do
	{
		if (h->hashes[index] == HashTableVacantKeySentinel)
		{
			h->hashes[index] = hash;
			h->buckets[index].key = k;
			h->buckets[index].value = v;
			continue;
		}
		else if (h->hashes[index] == hash && h->buckets[index].key == k)
		{
			continue;
		}
		index = (index + 1) % h->buckets.count;
	} while (index != startIndex);
	// The table should have expanded before we ever run out of space.
	Assert(index != startIndex);
	h->count += 1;
	h->loadFactor = (f32)h->count / (f32)h->buckets.count;
}

template <typename K, typename V>
V *DoLookup(HashTable<K, V> *h, K k, V *notFound)
{
	if (h->buckets.count == 0)
	{
		return;
	}
	auto hash = h->hashProcedure(k);
	if (hash == HashTableVacantKeySentinel)
	{
		hash = 0;
	}
	else if (hash == HashTableDeletedKeySentinel)
	{
		hash = 1;
	}
	auto find = hash % h->buckets.count;
	auto lastFind = find;
	do
	{
		if (h->hashes[find] == HashTableVacantKeySentinel)
		{
			return notFound;
		}
		else if (h->hashes[find] == hash && h->buckets[find].key == k)
		{
			return &h->buckets[find].value;
		}
		find = (find + 1) % h->buckets.count;
	} while (find != lastFind);
	return NULL;
}

template <typename K, typename V>
V HashTable<K, V>::Lookup(K k, V notFound)
{
	return *DoLookup(this, k, &notFound);
}

template <typename K, typename V>
V *HashTable<K, V>::LookupPointer(K k)
{
	auto notFound = (V *){};
	return DoLookup(this, k, &notFound);
}

template <typename K, typename V>
void HashTable<K, V>::Remove(HashTable<K, V> *h, K k)
{
	// @TODO
}

template <typename K, typename V>
void HashTable<K, V>::Clear()
{
	this->count = 0;
	for (auto &hash : this->hashes)
	{
		hash = HashTableVacantKeySentinel;
	}
}

template <typename K, typename V>
void HashTable<K, V>::ClearAndResize(s64 len)
{
	this->count = 0;
	this->buckets.Resize(len);
	for (auto &hash : this->hashes)
	{
		hash = HashTableVacantKeySentinel;
	}
}

template <typename K, typename V>
void HashTable<K, V>::Reserve(s64 count)
{
	if (this->buckets.count > count)
	{
		return;
	}
	if (this->buckets.count == 0)
	{
		this->hashes.Resize(count * 2);
		this->buckets.Resize(count * 2);
		return;
	}
	if ((f32)(this->count + cap) / (f32)this->buckets.count > HashTableMaxLoadFactor)
	{
		auto newHashes = NewArray<u64>(this->hashes.allocator, (this->buckets.count + cap) * 2);
		auto newBuckets = NewArray<KeyValuePair<K, V>>(this->buckets.allocator, newHashes.count);
		for (auto i = 0; i < this->buckets.count; i++)
		{
			auto newHash = this->hashProcedure(this->buckets[i].key);
			auto ni = newHash % newBuckets.count;
			newHashes[ni] = newHash;
			newBuckets[ni].key = this->buckets[i].key;
			newBuckets[ni].value = this->buckets[i].value;
		}
		this->hashes.Free();
		this->buckets.Free();
		this->buckets = newBuckets;
		this->hashes = newHashes;
		this->loadFactor = (f32)this->count / (f32)this->buckets.count;
	}
}
