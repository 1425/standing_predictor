#ifndef DECLINES_H
#define DECLINES_H

#include "io.h"
#include "vector_void.h"
#include "int_limited.h"
#include "interval.h"
#include "map_auto.h"

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

template<long long MIN1,long long MAX1,long long MIN2,long long MAX2>
auto diff(int n,Int_limited<MIN1,MAX1> const& a,Int_limited<MIN2,MAX2> const& b){
	if(a!=b){
		indent(n);
		std::cout<<"diff: "<<a<<"\t"<<b<<"\n";
	}
}

template<long long MIN,long long MAX>
auto diff(int n,int a,Int_limited<MIN,MAX> b){
	return diff(n,a,b.get());
}

template<typename T>
auto diff(int n,T a,Interval<T> b){
	if( !(a==b.min && a==b.max) ){
		indent(n);
		std::cout<<a<<"\t"<<b<<"\n";
	}
}

template<typename K1,typename V1,typename K2,typename V2>
auto diff(int n,map_auto<K1,V1> const& a,map_auto<K2,V2> const& b){
	bool shown=0;
	auto show=[&](){
		if(!shown){
			indent(n);
			n++;
			std::cout<<"map_auto\n";
			shown=1;
		}
	};

	auto k1=keys(a);
	auto k2=keys(b);
	auto a_only=k1-k2;
	if(!a_only.empty()){
		show();
		indent(n);
		std::cout<<"a only:"<<a_only<<"\n";
	}
	auto b_only=k2-k1;
	if(!b_only.empty()){
		show();
		indent(n);
		std::cout<<"b only:"<<b_only<<"\n";
	}
	for(auto k:k1&k2){
		auto a1=a[k];
		auto b1=b[k];
		if(a1!=b1){
			show();
			diff(n,a1,b1);
		}
	}
}

template<typename A,typename B>
auto diff(A const& a,B const& b){
	return diff(0,a,b);
}

template<typename T>
void diff(T const& a,T const& b){
	return diff(0,a,b);
}

#endif
