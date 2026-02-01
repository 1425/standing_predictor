#ifndef DECLINES_H
#define DECLINES_H

template<typename T>
void diff(int n,T const& a,T const& b){
	if(a!=b){
		indent(n);
		std::cout<<"a:"<<a<<"\n";
		indent(n);
		std::cout<<"b:"<<b<<"\n";
	}
}

template<typename T>
void diff(int n,std::vector<T> const& a,std::vector<T> const& b){
	/*if(a!=b){
		indent(n++);
		cout<<"vector\n";
	}*/
	for(auto [i,p]:enumerate(zip(a,b))){
		auto [a1,b1]=p;
		if(a1!=b1){
			indent(n);
			std::cout<<i<<"\n";
			diff(n+1,a1,b1);
		}
	}
	if(a.size()>b.size()){
		for(auto [i,x]:skip(b.size(),enumerate(a))){
			indent(n);
			std::cout<<"Only in a:"<<i<<" "<<x<<"\n";
		}
	}
	if(a.size()<b.size()){
		for(auto [i,x]:skip(a.size(),enumerate(b))){
			indent(n);
			std::cout<<"Only in b:"<<i<<" "<<x<<"\n";
		}
	}
}

template<typename A,typename B>
void diff(int n,std::pair<A,B> const& a,std::pair<A,B> const& b){
	/*if(a!=b){
		indent(n++);
		cout<<"pair\n";
	}*/
	if(a.first!=b.first){
		indent(n);
		std::cout<<"first\n";
		diff(n+1,a.first,b.first);
	}
	if(a.second!=b.second){
		indent(n);
		std::cout<<"second\n";
		diff(n+1,a.second,b.second);
	}
}

template<typename K,typename V>
void diff(int n,std::map<K,V> a,std::map<K,V> b){
	if(a!=b){
		indent(n++);
		std::cout<<"map\n";
	}
	auto ka=keys(a);
	auto kb=keys(b);
	for(auto elem:ka&kb){
		auto an=a[elem],bn=b[elem];
		if(an!=bn){
			indent(n);
			std::cout<<elem<<"\n";
			diff(n+1,a[elem],b[elem]);
		}
	}
	for(auto elem:ka-kb){
		indent(n);
		std::cout<<"Only in a:"<<elem<<"\n";
	}
	for(auto elem:kb-ka){
		indent(n);
		std::cout<<"Only in b:"<<elem<<"\n";
	}
}

template<typename T>
void diff(int n,std::optional<T> const& a,std::optional<T> const& b){
	if(a!=b){
		indent(n++);
		std::cout<<"optional\n";
	}
	if(a){
		if(b){
			return diff(n,*a,*b);
		}
		nyi
	}
	if(b){
		indent(n);
		std::cout<<"No a\n";
	}
}

template<typename T>
void diff(T const& a,T const& b){
	return diff(0,a,b);
}

#endif
