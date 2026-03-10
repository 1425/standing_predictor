#include "probability.h"

std::map<Point,Pr> operator+(std::map<Point,Pr> a,int i){
	std::map<Point,Pr> r;
	for(auto [k,v]:a){
		r[k+i]=v;
	}
	return r;
}

flat_map<Point,Pr> operator+(flat_map<Point,Pr> const& a,int i){
	flat_map<Point,Pr> r;
	for(auto [k,v]:a){
		r[k+i]=v;
	}
	return r;
}

flat_map2<Point,Pr> operator+(flat_map2<Point,Pr> const& a,int i){
	flat_map2<Point,Pr> r;
	//this is not an efficient way to do this with this data structure
	//should make a copy and then modify each of the keys
	for(auto [k,v]:a){
		r[k+i]=v;
	}
	return r;
}

double entropy(Team_dist const& a){
	return sum(mapf(
		[](auto x){ return -log2(x); },
		values(a)
	));
}

flat_map<Point,Pr> convolve(std::map<Point,Pr> const& a,std::map<Point,Pr> const& b){
	flat_map<Point,Pr> r;
	for(auto [a1,ap]:a){
		for(auto [b1,bp]:b){
			auto result=a1+b1;
			auto pr=ap*bp;
			auto f=r.find(result);
			if(f==r.end()){
				r[result]=pr;
			}else{
				f->second+=pr;
			}
		}
	}
	return r;
}

//flat_map<Point,Pr> convolve(flat_map2<Point,Pr> const& a,std::map<Point,flat_map2<Point,double>> const& b){
map_fixed<Int_limited<0,600>,Pr> convolve(flat_map2<Point,Pr> const& a,std::map<Point,flat_map2<Point,double>> const& b){
	//convolution may not be the correct name for this operation.
	//for each item in the first distribution, take a corresponding distribution out of 'b'
	//and and add the value from there

	//flat_map<Point,Pr> r;

	//curiosly, it does need to be this big.  I thought it might fit in 512, but it does not.
	map_fixed<Int_limited<0,600>,Pr> r;

	for(auto [k,v]:a){
		auto find_dist=[&](){
			auto f=b.find(k);
			if(f!=b.end()){
				return f->second;
			}
			auto m=max(keys(b));
			auto f2=b.find(m);
			assert(f2!=b.end());
			return f2->second;
		};
		for(auto [k2,v2]:find_dist()){
			auto k3=k+k2;
			auto v3=v*v2;
			r[k3]+=v3;
		}
	}

	return r;
}

flat_map<Point,Pr> convolve(flat_map<Point,Pr> const& a,std::map<Point,Pr> const& b){
	flat_map<Point,Pr> r;
	for(auto [a1,ap]:a){
		for(auto [b1,bp]:b){
			auto result=a1+b1;
			auto pr=ap*bp;
			auto f=r.find(result);
			if(f==r.end()){
				r[result]=pr;
			}else{
				f->second+=pr;
			}
		}
	}
	return r;
}

flat_map2<Point,Pr> convolve(flat_map2<Point,Pr> const& a,std::map<Point,Pr> const& b){
	flat_map2<Point,Pr> r;
	for(auto [a1,ap]:a){
		for(auto [b1,bp]:b){
			auto result=a1+b1;
			auto pr=ap*bp;
			auto f=r.find(result);
			if(f==r.end()){
				r[result]=pr;
			}else{
				f->second+=pr;
			}
		}
	}
	return r;
}

flat_map2<Point,Pr> convolve(flat_map2<Point,Pr> const& a,flat_map<Point,Pr> const& b){
	flat_map2<Point,Pr> r;
	for(auto [a1,ap]:a){
		for(auto [b1,bp]:b){
			auto result=a1+b1;
			auto pr=ap*bp;
			auto f=r.find(result);
			if(f==r.end()){
				r[result]=pr;
			}else{
				f->second+=pr;
			}
		}
	}
	return r;
}

flat_map2<Point,Pr> convolve(flat_map2<Point,Pr> const& a,flat_map2<Point,Pr> const& b){
	flat_map2<Point,Pr> r;
	for(auto [a1,ap]:a){
		for(auto [b1,bp]:b){
			auto result=a1+b1;
			auto pr=ap*bp;
			auto f=r.find(result);
			if(f==r.end()){
				r[result]=pr;
			}else{
				f->second+=pr;
			}
		}
	}
	return r;
}

