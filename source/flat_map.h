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

template <typename KeyT, typename MappedT>
class std::_detail::flat_map_iterator
{
public:
	flat_map_iterator() = default;
	flat_map_iterator(KeyT const* k, MappedT* v) : _key(k), _value(v) {}

private:
	KeyT const* _key = nullptr;
	MappedT* _value = nullptr;
};

template <typename KeyT, typename MappedT, typename CompareT, typename AllocatorT>
class std::_detail::flat_map_base
{
public:
	using key_type = KeyT;
	using mapped_type = MappedT;
	using value_type = std::pair<KeyT const&, MappedT&>;
	using iterator = flat_map_iterator<KeyT, MappedT>;
	using const_iterator = flat_map_iterator<KeyT, MappedT const>;
	using size_type = size_t;

protected:
	std::vector<KeyT, typename std::allocator_traits<AllocatorT>::template rebind_alloc<KeyT>> _keys;
	std::vector<MappedT, typename std::allocator_traits<AllocatorT>::template rebind_alloc<MappedT>> _values;

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

	template <typename KeyT2, typename MappedT2>
	inline std::pair<iterator, bool> insert(std::pair<KeyT2, MappedT2>&& element);

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

	template <typename KeyT2, typename MappedT2>
	inline iterator insert(std::pair<KeyT2, MappedT2>&& element);

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
	if (it != _keys.end() && !compare(key, it->first))
		return cast_iterator<iterator>(it);
	else
		return end();
}

template <typename KeyT, typename MappedT, typename CompareT, typename AllocatorT>
template <typename FindKeyT>
auto std::_detail::flat_map_base<KeyT, MappedT, CompareT, AllocatorT>::find(FindKeyT const& key) const -> const_iterator
{
	auto compare = inner_compare_type();
	auto const rng = std::lower_bound(_keys.begin(), _keys.end(), key, compare);
	if (!rng.empty() && !compare(key, rng.front().first))
		return cast_iterator<const_iterator>(rng.begin());
	else
		return end();
}

template <typename KeyT, typename MappedT, typename CompareT, typename AllocatorT>
auto std::_detail::flat_map_base<KeyT, MappedT, CompareT, AllocatorT>::erase(const_iterator iter) -> iterator
{
	return cast_iterator<iterator>(_data.erase(_data.begin() + (iter - begin())));
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
	auto rng = equal_range(key);
	erase(rng.first, rng.second);
	return rng.second - rng.first;
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
	auto const rng = std::lower_bound(_keys.begin(), _keys.end(), key, compare);
	if (!rng.empty() && !compare(key, rng.front().first))
		return rng.front().second;

	return emplace(key, MappedT()).first->second;
}

template <typename KeyT, typename MappedT, typename CompareT, typename AllocatorT>
template <typename KeyT2, typename MappedT2>
auto std::flat_map<KeyT, MappedT, CompareT, AllocatorT>::insert(std::pair<KeyT2, MappedT2>&& element) -> std::pair<iterator, bool>
{
	auto compare = inner_compare_type();
	auto const rng = std::lower_bound(_keys.begin(), _keys.end(), element.first, compare);
	if (!rng.empty() && !compare(element.first, rng.front().first))
		return std::make_pair(cast_iterator<iterator>(rng.begin()), false);
	else
		return std::make_pair(cast_iterator<iterator>(_data.emplace(rng.begin(), std::move(element))), true);
}

template <typename KeyT, typename MappedT, typename CompareT, typename AllocatorT>
template <typename EmplaceKeyT, typename... P0toN>
auto std::flat_map<KeyT, MappedT, CompareT, AllocatorT>::emplace(EmplaceKeyT&& key, P0toN&&... params) -> std::pair<iterator, bool>
{
	auto compare = inner_compare_type();
	auto const rng = std::lower_bound(_keys.begin(), _keys.end(), key, compare);
	if (!rng.empty() && !compare(key, rng.front().first))
		return std::make_pair(cast_iterator<iterator>(rng.begin()), false);
	else
		return std::make_pair(cast_iterator<iterator>(_data.emplace(rng.begin(), std::forward<EmplaceKeyT>(key), MappedT(std::forward<P0toN>(params)...))), true);
}

template <typename KeyT, typename MappedT, typename CompareT, typename AllocatorT>
template <typename FindKeyT>
auto std::flat_multimap<KeyT, MappedT, CompareT, AllocatorT>::operator[](FindKeyT const& key) -> mapped_type&
{
	auto compare = inner_compare_type();
	auto const rng = std::lower_bound(_keys.begin(), _keys.end(), key, compare);
	if (!rng.empty() && !compare(key, rng.front().first))
		return rng.front().second;

	return emplace(key, MappedT()).first->second;
}

template <typename KeyT, typename MappedT, typename CompareT, typename AllocatorT>
template <typename KeyT2, typename MappedT2>
auto std::flat_multimap<KeyT, MappedT, CompareT, AllocatorT>::insert(std::pair<KeyT2, MappedT2>&& element) -> iterator
{
	auto const rng = std::lower_bound(_keys.begin(), _keys.end(), element.first, inner_compare_type());
	return cast_iterator<iterator>(_data.emplace(rng.begin(), std::move(element)));
}

template <typename KeyT, typename MappedT, typename CompareT, typename AllocatorT>
template <typename EmplaceKeyT, typename... P0toN>
auto std::flat_multimap<KeyT, MappedT, CompareT, AllocatorT>::emplace(EmplaceKeyT&& key, P0toN&&... params) -> iterator
{
	auto compare = inner_compare_type();
	auto const rng = std::lower_bound(_keys.begin(), _keys.end(), key, compare);
	return cast_iterator<iterator>(_data.emplace(rng.begin(), std::forward<EmplaceKeyT>(key), MappedT(std::forward<P0toN>(params)...)));
}

#endif // !defined(SG14_FLAT_MAP_H__)
