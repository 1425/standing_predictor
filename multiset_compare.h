#ifndef MULTISET_COMPARE_H
#define MULTISET_COMPARE_H

template<typename T>
bool operator==(std::multiset<T> const& a,multiset_flat<T> const& b){
	return to_vec(a)==to_vec(b);
}

template<typename T>
bool operator==(std::vector<T> const& a,multiset_flat<T> const& b){
	return sorted(a)==to_vec(b);
}

template<typename T>
struct multiset_compare{
	//The purpose of this is to compare two implementations of multisets and see if they do the same thing.
	//Not for general use.

	using B=std::multiset<T>;
	using A=multiset_flat<T>;
	A a;
	B b;

	public:

	multiset_compare()=default;

	multiset_compare(A a1,B b1):
		a(a1),
		b(b1)
	{
		assert(a==b);
	}

	auto size()const{
		auto a1=a.size();
		auto b1=b.size();
		assert(a1==b1);
		return a1;
	}

	bool empty()const{
		auto a1=a.empty();
		auto b1=b.empty();
		assert(a1==b1);
		return a1;
	}

	multiset_compare operator-(T t)const{
		auto a1=a-t;
		auto b1=b-t;
		assert(a1==b1);
		return multiset_compare{a1,b1};
	}

	using const_iterator=typename A::const_iterator;

	const_iterator begin()const{
		return a.begin();
	}

	const_iterator end()const{
		return a.end();
	}

	multiset_compare& operator|=(T t){
		a|=t;
		b|=t;
		assert(a==b);
		return *this;
	}
};

template<typename T>
std::ostream& operator<<(std::ostream& o,multiset_compare<T> const& a){
	return o<<a.a;
}

template<typename T>
auto to_multiset_compare(std::vector<T> const& a){
	return multiset_compare<T>{
		to_multiset_flat(a),
		to_multiset(a)
	};
}

template<typename Func,typename T>
auto mapf(Func f,multiset_compare<T> const& a){
	auto a1=mapf(f,a.a);
	auto b1=mapf(f,a.b);
	assert(a1==b1);
	return a1;
}

template<typename Func,typename T>
auto filter(Func f,multiset_compare<T> const& a){
	auto a1=filter(f,a.a);
	auto b1=filter(f,a.b);
	assert(a1==b1);
	return multiset_compare{a1,b1};
}

#endif
