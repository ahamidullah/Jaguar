#pragma once

namespace map
{

// @TODO: Change the default values to 0 and 1 for easier intitialization.
const auto VacantHashSentinel = (u64)-1;
const auto DeletedHashSentinel = (u64)-2;
const auto DefaultInitialLength = 16;
const auto MaxLoadFactor = 0.75f;

template <typename K, typename V>
struct keyValue
{
	K key;
	V value;
};

template <typename K, typename V> struct Iterator;

template <typename K, typename V>
struct map
{
	arr::array<u64> hashes;
	arr::array<keyValue<K, V>> buckets;
	typedef u64 (*hashProcedure)(K);
	hashProcedure hashProcedure;
	s64 count;
	f32 loadFactor;

	iterator<K, V> begin();
	iterator<K, V> end();
	void DoInsert(u64 hash, K k, V v);
	void Insert(K k, V v);
	V *DoLookup(K k);
	V *Lookup(K k);
	void Remove(K k);
	void Clear();
	void ClearAndResize(s64 len);
	void Reserve(s64 count);
	void Resize();
};

template <typename K, typename V>
map<K, V> NewIn(mem::allocator *a, s64 cap, typename map<K, V>::hashProcedure hp)
{
	auto m = map<K, V>
	{
		.hashes = arr::NewIn<u64>(a, cap),
		.buckets = arr::NewIn<KeyValue<K, V>>(a, cap),
		.hashProcedure = hp,
	};
	for (auto &h : m.hashes)
	{
		h = VacantHashSentinel;
	}
	return m;
}

template <typename K, typename V>
map<K, V> New(s64 cap, typename map<K, V>::hashProcedure p)
{
	return NewIn<K, V>(mem::ContextAllocator(), cap, p);
}

template <typename K, typename V>
Iterator<K, V> Map<K, V>::begin()
{
	auto itr = Iterator<K, V>
	{
		.map = this,
		.index = -1,
	};
	return ++itr;
}

template <typename K, typename V>
Iterator<K, V> Map<K, V>::end()
{
	return
	{
		.map = this,
		.index = this->buckets.count,
	};
}

template <typename K, typename V>
void Map<K, V>::DoInsert(u64 hash, K k, V v)
{
	if (hash == VacantHashSentinel)
	{
		hash = 0;
	}
	else if (hash == DeletedHashSentinel)
	{
		hash = 1;
	}
	auto i = hash % this->buckets.count;
	auto end = i;
	do
	{
		if (this->hashes[i] == VacantHashSentinel || (this->hashes[i] == hash && this->buckets[i].key == k))
		{
			this->hashes[i] = hash;
			this->buckets[i].key = k;
			this->buckets[i].value = v;
			this->count += 1;
			this->loadFactor = (f32)this->count / (f32)this->buckets.count;
			return;
		}
		i = (i + 1) % this->buckets.count;
		// The table should expand before we run out of space, so we never wrap around to the start index.
		Assert(i != end);
	} while (i != end);
}

template <typename K, typename V>
void Map<K, V>::Insert(K k, V v)
{
	// The user has to set the hash procedure, even if the Map is zero-initialized.
	Assert(this->hashProcedure);
	this->Reserve(this->count + 1);
	auto h = this->hashProcedure(k);
	this->DoInsert(h, k, v);
}

template <typename K, typename V>
V *Map<K, V>::Lookup(K k)
{
	if (this->buckets.count == 0)
	{
		return NULL;
	}
	auto hash = this->hashProcedure(k);
	if (hash == VacantHashSentinel)
	{
		hash = 0;
	}
	else if (hash == DeletedHashSentinel)
	{
		hash = 1;
	}
	auto i = hash % this->buckets.count;
	auto end = i;
	do
	{
		if (this->hashes[i] == VacantHashSentinel)
		{
			return NULL;
		}
		else if (this->hashes[i] == hash && this->buckets[i].key == k)
		{
			return &this->buckets[i].value;
		}
		i = (i + 1) % this->buckets.count;
	} while (i != end);
	return NULL;
}

template <typename K, typename V>
void Map<K, V>::Remove(K k)
{
	// @TODO
}

template <typename K, typename V>
void Map<K, V>::Clear()
{
	this->count = 0;
	for (auto &hash : this->hashes)
	{
		hash = VacantHashSentinel;
	}
}

template <typename K, typename V>
void Map<K, V>::ClearAndResize(s64 len)
{
	this->count = 0;
	this->buckets.Resize(len);
	for (auto &hash : this->hashes)
	{
		hash = VacantHashSentinel;
	}
}

template <typename K, typename V>
void Map<K, V>::Reserve(s64 reserve)
{
	if (this->buckets.count > reserve)
	{
		return;
	}
	if (this->buckets.count == 0)
	{
		this->hashes.Resize(reserve * 2);
		for (auto &h : this->hashes)
		{
			h = VacantHashSentinel;
		}
		this->buckets.Resize(reserve * 2);
		return;
	}
	if ((f32)reserve / (f32)this->buckets.count <= MaxLoadFactor)
	{
		return;
	}
	auto newHT = NewIn<K, V>(this->buckets.allocator, reserve * 2, this->hashProcedure);
	for (auto i = 0; i < this->buckets.count; i += 1)
	{
		if (this->hashes[i] == VacantHashSentinel || this->hashes[i] == DeletedHashSentinel)
		{
			continue;
		}
		newHT.DoInsert(this->hashes[i], this->buckets[i].key, this->buckets[i].value);
	}
	*this = newHT;
}

template <typename K, typename V>
struct Iterator
{
	Map<K, V> *map;
	s64 index;

	Iterator<K, V> operator++();
	KeyValue<K, V> operator*();
	bool operator==(Iterator<K, V> itr);
};

template <typename K, typename V>
Iterator<K, V> Iterator<K, V>::operator++()
{
	this->index += 1;
	for (; this->index < this->map->buckets.count; this->index += 1)
	{
		if (this->map->hashes[this->index] != VacantHashSentinel && this->map->hashes[this->index] != DeletedHashSentinel)
		{
			break;
		}
	}
	return
	{
		.map = this->map,
		.index = this->index,
	};
}

template <typename K, typename V>
KeyValue<K, V> Iterator<K, V>::operator*()
{
	return this->map->buckets[this->index];
}

template <typename K, typename V>
bool Iterator<K, V>::operator==(Iterator<K, V> itr)
{
	return this->map == itr.map && this->index == itr.index;
}

}
