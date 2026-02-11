#ifndef SET_LIMITED_H
#define SET_LIMITED_H

template<typename T,size_t N>
class set_limited{
	using Data=std::array<T,N>;
	alignas(Data) char buf[sizeof(Data)];
	size_t size_=0;

	Data& data(){
		return *(Data*)buf;
	}

	Data const& data()const{
		return *(Data*)buf;
	}

	public:
	set_limited()=default;

	set_limited(set_limited const& a){
		size_=a.size_;
		size_t i=0;
		for(auto elem:a){
			new(&data()[i]) T(elem);
			i++;
		}
	}

	set_limited(set_limited && a){
		size_=a.size_;
		for(size_t i=0;i<size_;i++){
			new(&data()[i]) T(std::move(a.data()[i]));
		}
		a.size_=0;
	}

	set_limited& operator=(set_limited&& a){
		if(size()<a.size()){
			size_t i=0;
			for(;i<size();i++){
				data()[i]=std::move(a.data()[i]);
			}
			for(;i<a.size();i++){
				new(&data()[i]) T(std::move(a.data()[i]));
			}
			size_=a.size_;
			a.size_=0;
		}else{
			size_t i=0;
			for(;i<a.size();i++){
				data()[i]=a.data()[i];
			}
			for(;i<size();i++){
				data()[i].~T();
			}
			size_=a.size_;
			a.size_=0;
		}
		return *this;
	}

	set_limited& operator=(set_limited const& a){
		if(size()<a.size()){
			size_t i=0;
			for(;i<size();i++){
				data()[i]=a.data()[i];
			}
			for(;i<a.size();i++){
				new(&data()[i]) T(a.data()[i]);
			}
			size_=a.size();
		}else{
			size_t i=0;
			for(;i<a.size();i++){
				data()[i]=a.data()[i];
			}
			for(auto j=i;j<size();j++){
				data()[i].~T();
			}
			size_=a.size();
		}
		return *this;
	}

	set_limited(std::set<T> a){
		if(a.size()>N){
			print_r(a);
		}
		assert(a.size()<=N);
		size_=a.size();
		size_t i=0;
		for(auto elem:a){
			new(&data()[i]) T(elem);
			i++;
		}
	}

	set_limited(std::vector<T> a):
		set_limited(to_set(a))
	{}

	~set_limited(){
		//for(auto i:range(size_)){
		for(size_t i=0;i<size_;i++){
			data()[i].~T();
		}
	}

	auto size()const{
		return size_;
	}

	using const_iterator=Data::const_iterator;

	const_iterator begin()const{
		return data().begin();
	}

	const_iterator end()const{
		return data().begin()+size_;
	}

	bool count(T const& t)const{
		for(auto const& elem:*this){
			if(elem==t){
				return 1;
			}
		}
		return 0;
	}

	bool empty()const{
		return size()==0;
	}

	set_limited& operator|=(T const& t){
		assert(size_<N);
		new(&data()[size_]) T(t);
		size_++;
		return *this;
	}
};

template<typename T,size_t N>
std::ostream& operator<<(std::ostream& o,set_limited<T,N> const& a){
	o<<"{ ";
	for(auto const& x:a){
		o<<x<<" ";
	}
	return o<<"}";
}

template<typename T,size_t N>
set_limited<T,N> operator&(set_limited<T,N> const& a,set_limited<T,N> const& b){
	set_limited<T,N> r;
	for(auto x:a){
		if(b.count(x)){
			r|=x;
		}
	}
	return r;
}

template<typename T,size_t N>
bool contains(set_limited<T,N> const& a,T const& b){
	for(auto elem:a){
		if(elem==b){
			return 1;
		}
	}
	return 0;
}

template<typename T,size_t N>
std::set<T> to_set(set_limited<T,N> const& a){
	return std::set<T>{a.begin(),a.end()};
}

template<typename Func,typename T,size_t N>
auto mapf(Func f,set_limited<T,N> const& a){
	return mapf(f,to_set(a));
}

template<typename T,size_t N,size_t M>
std::vector<T> flatten(std::array<set_limited<T,N>,M> const& a){
	std::vector<T> r;
	for(auto x:a){
		for(auto elem:x){
			r|=elem;
		}
	}
	return r;
}

template<typename T,size_t N>
set_limited<T,N> rand(set_limited<T,N> const*);

template<typename T,size_t N>
std::set<T> operator-(std::set<T> a,set_limited<T,N> const& b){
	for(auto const& elem:b){
		a.erase(elem);
	}
	return a;
}

#endif
