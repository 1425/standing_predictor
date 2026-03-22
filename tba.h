#ifndef TBA_H
#define TBA_H

#include<set>
#include<memory>
#include<any>
#include "../tba/data.h"
#include "../tba/db.h"
#include "../tba/match.h"
#include "probability.h"

class Argument_parser;

struct No_data{
	std::string url;
};

std::ostream& operator<<(std::ostream&,No_data const&);

struct TBA_fetcher_base{
	virtual std::pair<tba::HTTP_Date,tba::Data> fetch(tba::URL const& url)const=0;
	virtual ~TBA_fetcher_base();
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
	bool local_only,log,refresh=0;
	std::optional<std::string> fuzz;

	TBA_fetcher_config();

	void add(Argument_parser&);
	TBA_fetcher get()const;
};

#endif
