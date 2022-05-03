#ifndef TBA_H
#define TBA_H

#include<set>
#include<memory>
#include "../tba/data.h"
#include "../tba/db.h"
#include "output.h"

class Argument_parser;

struct No_data{
	std::string url;
};

std::ostream& operator<<(std::ostream&,No_data const&);

struct TBA_fetcher_base{
	virtual std::pair<tba::HTTP_Date,tba::Data> fetch(tba::URL const& url)const=0;
};

template<typename T>
class TBA_fetcher_impl:public TBA_fetcher_base{
	std::unique_ptr<T> t;

	public:
	TBA_fetcher_impl(T* t1):
		t(t1)
	{}

	virtual std::pair<tba::HTTP_Date,tba::Data> fetch(tba::URL const& url)const{
		return t->fetch(url);
	}
};

class TBA_fetcher{
	std::unique_ptr<TBA_fetcher_base> data;

	public:
	template<typename T>
	TBA_fetcher(T *t):
		data(new TBA_fetcher_impl<T>{t})
	{}

	std::pair<tba::HTTP_Date,tba::Data> fetch(tba::URL const& url)const;
};

struct TBA_fetcher_config{
	std::string auth_key_path,cache_path;
	bool local_only;

	TBA_fetcher_config();

	void add(Argument_parser&);
	TBA_fetcher get()const;
};

//tba::Cached_fetcher get_tba_fetcher(std::string const& auth_key_path,std::string const& cache_path);

std::set<tba::Team_key> chairmans_winners(TBA_fetcher&,tba::District_key const&);
std::map<Point,Pr> dcmp_distribution(TBA_fetcher&);
std::map<Point,Pr> historical_event_pts(TBA_fetcher&);

#endif
