// Copyright (C) 2014-2016 Sean Middleditch.

#if !defined(SG14_FLAT_MAP_H__)
#define SG14_FLAT_MAP_H__

#include <vector>
#include <algorithm>

namespace std
{
	template <typename, typename, typename, typename> class flat_map;
	template <typename, typename, typename, typename> class flat_multimap;
	namespace _detail { template <typename, typename, typename, typename> class flat_map_base; }
	namespace _detail { template <typename, typename> class flat_map_iterator; }
}

template <typename KeyIterator, typename MappedIterator>
class std::_detail::flat_map_iterator
{
public:
	flat_map_iterator() = default;
	flat_map_iterator(KeyIterator k, MappedIterator m) : _key(k), _mapped(m) {}

private:
	KeyIterator _key;
	MappedIterator _mapped;
};

template <typename KeyT, typename MappedT, typename CompareT, typename AllocatorT>
class std::_detail::flat_map_base
{
protected:
	using key_vector = std::vector<KeyT, typename std::allocator_traits<AllocatorT>::template rebind_alloc<KeyT>>;
	using value_vector = std::vector<MappedT, typename std::allocator_traits<AllocatorT>::template rebind_alloc<MappedT>>;

	key_vector _keys;
	value_vector _values;

public:
	using key_type = KeyT;
	using mapped_type = MappedT;
	using value_type = std::pair<KeyT const&, MappedT&>;
	using iterator = flat_map_iterator<typename key_vector::const_iterator, typename value_vector::iterator>;
	using const_iterator = flat_map_iterator<typename key_vector::const_iterator, typename value_vector::const_iterator>;
	using size_type = size_t;

protected:
	template <typename T, typename U>
	static T cast_iterator(U&& src) { return reinterpret_cast<T>(std::addressof(*src)); }

public:
	flat_map_base() = default;

	flat_map_base(flat_map_base const&) = delete;
	flat_map_base& operator=(flat_map_base const&) = delete;

	flat_map_base(flat_map_base&&) = default;
	flat_map_base& operator=(flat_map_base&&) = default;

	iterator begin() { return iterator(_keys.data(), _values.data()); }
	const_iterator begin() const { return const_iterator(_keys.data(), _values.data()); }
	const_iterator cbegin() const { return const_iterator(_keys.data(), _values.data()); }

	iterator end() { return iterator(_keys.data() + _keys.size(), _values.data() + _values.size()); }
	const_iterator end() const { return const_iterator(_keys.data() + _keys.size(), _values.data() + _values.size()); }
	const_iterator cend() const { return const_iterator(_keys.data() + _keys.size(), _values.data() + _values.size()); }

	value_type const& at(size_t index) const { return _values[index]; }

	size_type size() const { return _keys.size(); }
	bool empty() const { return _keys.empty(); }

	template <typename FindKeyT>
	inline pair<iterator, iterator> equal_range(FindKeyT const& key);
	template <typename FindKeyT>
	inline pair<const_iterator, const_iterator> equal_range(FindKeyT const& key) const;

	template <typename FindKeyT>
	inline iterator find(FindKeyT const& key);
	template <typename FindKeyT>
	inline const_iterator find(FindKeyT const& key) const;

	inline iterator erase(const_iterator iter);
	inline void erase(const_iterator begin_iter, const_iterator end_iter);
	template <typename EraseKeyT>
	inline size_type erase(EraseKeyT const& key);

	void clear();
};

template <typename KeyT, typename MappedT, typename CompareT = std::less<>, typename AllocatorT = std::allocator<std::pair<KeyT const, MappedT>>>
class std::flat_map : public std::_detail::flat_map_base<KeyT, MappedT, CompareT, AllocatorT>
{
public:
	using flat_map_base::flat_map_base;

	template <typename FindKeyT>
	inline mapped_type& operator[](FindKeyT const& key);

	inline std::pair<iterator, bool> insert(std::pair<KeyT, MappedT>&& element);

	template <typename EmplaceKeyT, typename... P0toN>
	inline std::pair<iterator, bool> emplace(EmplaceKeyT&& key, P0toN&&... params);
};

template <typename KeyT, typename MappedT, typename CompareT = std::less<>, typename AllocatorT = std::default_allocator>
class std::flat_multimap : public std::_detail::flat_map_base<KeyT, MappedT, CompareT, AllocatorT>
{
public:
	using flat_map_base::flat_map_base;

	template <typename FindKeyT>
	inline mapped_type& operator[](FindKeyT const& key);

	inline iterator insert(std::pair<KeyT, MappedT>&& element);

	template <typename EmplaceKeyT, typename... P0toN>
	inline iterator emplace(EmplaceKeyT&& key, P0toN&&... params);
};

template <typename KeyT, typename MappedT, typename CompareT, typename AllocatorT>
template <typename FindKeyT>
auto std::_detail::flat_map_base<KeyT, MappedT, CompareT, AllocatorT>::equal_range(FindKeyT const& key) -> pair<iterator, iterator>
{
	return std::equal_range(_keys.begin(), _keys.end(), key, inner_compare_type());
}

template <typename KeyT, typename MappedT, typename CompareT, typename AllocatorT>
template <typename FindKeyT>
auto std::_detail::flat_map_base<KeyT, MappedT, CompareT, AllocatorT>::equal_range(FindKeyT const& key) const -> pair<const_iterator, const_iterator>
{
	return std::equal_range(_keys.begin(), _keys.end(), key, inner_compare_type());
}

template <typename KeyT, typename MappedT, typename CompareT, typename AllocatorT>
template <typename FindKeyT>
auto std::_detail::flat_map_base<KeyT, MappedT, CompareT, AllocatorT>::find(FindKeyT const& key) -> iterator
{
	auto compare = inner_compare_type();
	auto it = std::lower_bound(_keys.begin(), _keys.end(), key, compare);
	if (it != _keys.end() && !compare(key, *it))
		return iterator(it);
	else
		return end();
}

template <typename KeyT, typename MappedT, typename CompareT, typename AllocatorT>
template <typename FindKeyT>
auto std::_detail::flat_map_base<KeyT, MappedT, CompareT, AllocatorT>::find(FindKeyT const& key) const -> const_iterator
{
	auto compare = inner_compare_type();
	auto const it = std::lower_bound(_keys.begin(), _keys.end(), key, compare);
	if (it != _keys.end() && !compare(key, *it))
		return const_iterator(it.begin());
	else
		return end();
}

template <typename KeyT, typename MappedT, typename CompareT, typename AllocatorT>
auto std::_detail::flat_map_base<KeyT, MappedT, CompareT, AllocatorT>::erase(const_iterator iter) -> iterator
{
	return iterator(_data.erase(_data.begin() + (iter - begin())));
}

template <typename KeyT, typename MappedT, typename CompareT, typename AllocatorT>
void std::_detail::flat_map_base<KeyT, MappedT, CompareT, AllocatorT>::erase(const_iterator begin_iter, const_iterator end_iter)
{
	_data.erase(_data.begin() + (begin_iter - begin()), _data.begin() + (end_iter - begin()));
}

template <typename KeyT, typename MappedT, typename CompareT, typename AllocatorT>
template <typename EraseKeyT>
auto std::_detail::flat_map_base<KeyT, MappedT, CompareT, AllocatorT>::erase(EraseKeyT const& key) -> size_type
{
	auto it = equal_range(key);
	erase(it.first, it.second);
	return it.second - it.first;
}

template <typename KeyT, typename MappedT, typename CompareT, typename AllocatorT>
void std::_detail::flat_map_base<KeyT, MappedT, CompareT, AllocatorT>::clear()
{
	_keys.clear();
	_values.clear();
}

template <typename KeyT, typename MappedT, typename CompareT, typename AllocatorT>
template <typename FindKeyT>
auto std::flat_map<KeyT, MappedT, CompareT, AllocatorT>::operator[](FindKeyT const& key) -> mapped_type&
{
	auto compare = inner_compare_type();
	auto const it = std::lower_bound(_keys.begin(), _keys.end(), key, compare);
	if (it != _keys.end() && !compare(key, *it))
		return it->second;

	return emplace(key, MappedT()).first->second;
}

template <typename KeyT, typename MappedT, typename CompareT, typename AllocatorT>
auto std::flat_map<KeyT, MappedT, CompareT, AllocatorT>::insert(std::pair<KeyT, MappedT>&& element) -> std::pair<iterator, bool>
{
	auto compare = CompareT();
	auto const it = std::lower_bound(_keys.begin(), _keys.end(), element.first, compare);
	if (it != _keys.end() && !compare(element.first, *it))
	{
		return std::make_pair(iterator(it, _values.begin() + (it - _keys.begin())), false);
	}
	else
	{
		_keys.push_back(std::move(element).first);
		_values.push_back(std::move(element).second);
		return std::make_pair(iterator(_keys.end() - 1, _values.end() - 1), true);
	}
}

template <typename KeyT, typename MappedT, typename CompareT, typename AllocatorT>
template <typename EmplaceKeyT, typename... P0toN>
auto std::flat_map<KeyT, MappedT, CompareT, AllocatorT>::emplace(EmplaceKeyT&& key, P0toN&&... params) -> std::pair<iterator, bool>
{
	auto compare = CompareT();
	auto const it = std::lower_bound(_keys.begin(), _keys.end(), key, compare);
	if (it != _keys.end() && !compare(key, *it))
		return std::make_pair(iterator(it.begin(), _values.data() + (it - _keys.begin())), false);
	else
		return std::make_pair(iterator(_data.emplace(it.begin(), std::forward<EmplaceKeyT>(key), MappedT(std::forward<P0toN>(params)...))), true);
}

template <typename KeyT, typename MappedT, typename CompareT, typename AllocatorT>
template <typename FindKeyT>
auto std::flat_multimap<KeyT, MappedT, CompareT, AllocatorT>::operator[](FindKeyT const& key) -> mapped_type&
{
	auto compare = CompareT();
	auto const it = std::lower_bound(_keys.begin(), _keys.end(), key, compare);
	if (it != _keys.end() && !compare(key, *it))
		return it->second;

	return emplace(key, MappedT()).first->second;
}

template <typename KeyT, typename MappedT, typename CompareT, typename AllocatorT>
auto std::flat_multimap<KeyT, MappedT, CompareT, AllocatorT>::insert(std::pair<KeyT, MappedT>&& element) -> iterator
{
	auto const it = std::lower_bound(_keys.begin(), _keys.end(), element.first, inner_compare_type());
	return iterator(_data.emplace(it.begin(), std::move(element)));
}

template <typename KeyT, typename MappedT, typename CompareT, typename AllocatorT>
template <typename EmplaceKeyT, typename... P0toN>
auto std::flat_multimap<KeyT, MappedT, CompareT, AllocatorT>::emplace(EmplaceKeyT&& key, P0toN&&... params) -> iterator
{
	auto compare = inner_compare_type();
	auto const it = std::lower_bound(_keys.begin(), _keys.end(), key, compare);
	return iterator(_data.emplace(it.begin(), std::forward<EmplaceKeyT>(key), MappedT(std::forward<P0toN>(params)...)));
}

#endif // !defined(SG14_FLAT_MAP_H__)
