#ifndef CMP_REASON_H
#define CMP_REASON_H

#include<map>
#include<iosfwd>
#include<set>
#include<vector>

namespace tba{
	class Year;
	class Team_key;
	class District_key;
	struct Award;
};

class TBA_fetcher;

#define CMP_REASON_ITEMS(X)\
	X(PRE_QUALIFIED)\
	X(REGIONAL)\
	X(DISTRICT)\
	X(PRIORITY_WAITLIST)\
	X(WAITLIST)

enum class Cmp_reason{
	#define X(A) A,
	CMP_REASON_ITEMS(X)
	#undef X
};

std::ostream& operator<<(std::ostream& o,Cmp_reason);

std::vector<tba::District_key> district_keys(TBA_fetcher&,tba::Year);

std::set<tba::Team_key> pre_qualifying(TBA_fetcher&,tba::Year current_year);

std::set<tba::Team_key> cmp_teams(TBA_fetcher &,tba::Year);

std::map<tba::Team_key,Cmp_reason> cmp_reasons(TBA_fetcher &tba_fetcher,tba::Year year);

std::vector<tba::Team_key> team_winners(tba::Award const&);
std::vector<tba::Team_key> team_winners(std::vector<tba::Award> const&);

#endif
