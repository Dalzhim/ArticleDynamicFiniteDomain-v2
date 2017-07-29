//
//  main.cpp
//  ArticleDynamicFiniteDomain
//
//  Created by Gabriel Aubut-Lussier on 2015-07-30.
//

#include <boost/icl/interval_map.hpp>
#include <boost/icl/concept/interval.hpp>
#include <boost/icl/type_traits/interval_type_default.hpp>
#include <iostream>
#include <set>

struct some_payload
{
};

class dynamic_domain
{
public:
	using identifier_type = unsigned int;
	using payload_type = some_payload;
	
public:
	dynamic_domain(identifier_type identifier) : identifier(identifier) {}
	
	bool operator<(const dynamic_domain& rhs) const
	{
		return identifier < rhs.identifier;
	}
	
	payload_type& get_payload()
	{
		return payload;
	}
	
	const payload_type& get_payload() const
	{
		return payload;
	}
	
	int get_identifier() const
	{
		return identifier;
	}
	
private:
	identifier_type identifier;
	payload_type payload;
};

using container_type = std::map<dynamic_domain::identifier_type, dynamic_domain>;
using iterator_type = container_type::const_iterator;

class dynamic_domain_iterator
{
public:
	dynamic_domain_iterator() = default;
	dynamic_domain_iterator(const iterator_type& it) : it(it) {}
	
	const dynamic_domain& operator*() const
	{
		return it->second;
	}
	
	const dynamic_domain* operator->() const
	{
		return std::addressof(it->second);
	}
	
	dynamic_domain_iterator& operator--()
	{
		--it;
		return *this;
	}
	
	dynamic_domain_iterator& operator++()
	{
		++it;
		return *this;
	}
	
	bool operator==(const dynamic_domain_iterator& rhs) const
	{
		return it == rhs.it;
	}
	
	bool operator!=(const dynamic_domain_iterator& rhs) const
	{
		return it != rhs.it;
	}
	
	bool operator<(const dynamic_domain_iterator& rhs) const
	{
		return it->second.get_identifier() < rhs.it->second.get_identifier();
	}
	
private:
	iterator_type it;
};

namespace std
{
	template <>
	struct iterator_traits<dynamic_domain_iterator>
	{
		using difference_type = iterator_traits<iterator_type>::difference_type;
		using value_type = iterator_type;
		using pointer = iterator_type*;
		using reference = const dynamic_domain_iterator&;
		using iterator_category = std::bidirectional_iterator_tag;
	};
}

namespace boost { namespace icl {
	template<>
	struct interval_type_default<dynamic_domain_iterator>
	{
		using type = boost::icl::closed_interval<dynamic_domain_iterator>;
	};
}}

namespace boost { namespace icl {
	template<>
	bool is_empty<closed_interval<dynamic_domain_iterator>>(const closed_interval<dynamic_domain_iterator>& object)
	{
		const dynamic_domain_iterator emptyIterator{};
		const auto& lower = boost::icl::lower(object);
		const auto& upper = boost::icl::upper(object);
		return lower == emptyIterator || upper == emptyIterator || domain_less<closed_interval<dynamic_domain_iterator>>(upper, lower);
	}
	
	template <>
	struct unit_element<dynamic_domain_iterator>
	{
		static dynamic_domain_iterator value()
		{
			return dynamic_domain_iterator{};
		}
	};
}}

using MapType = boost::icl::interval_map<dynamic_domain_iterator, int>;

int main(int argc, const char * argv[])
{
	container_type elements;
	elements.insert(std::make_pair(7, dynamic_domain{7}));
	elements.insert(std::make_pair(1, dynamic_domain{1}));
	elements.insert(std::make_pair(2, dynamic_domain{2}));
	elements.insert(std::make_pair(6, dynamic_domain{6}));
	elements.insert(std::make_pair(3, dynamic_domain{3}));
	elements.insert(std::make_pair(4, dynamic_domain{4}));
	elements.insert(std::make_pair(9001, dynamic_domain{9001}));
	elements.insert(std::make_pair(5, dynamic_domain{5}));
	
	MapType intervalMap;
	auto it = dynamic_domain_iterator{elements.cbegin()};
	intervalMap.add(std::make_pair(MapType::interval_type{it, std::next(it, 1)}, 1));
	intervalMap.add(std::make_pair(MapType::interval_type{std::next(it, 2), std::next(it, 3)}, 1));
	intervalMap.add(std::make_pair(MapType::interval_type{std::next(it, 1), std::next(it, 3)}, 1));
	for (auto element : intervalMap) {
		std::cout << "Domain: [" << element.first.lower()->get_identifier() << ", " << element.first.upper()->get_identifier() << "] = " << element.second << std::endl;
	}
}
