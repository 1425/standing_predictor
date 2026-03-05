#ifndef SLOTS_H
#define SLOTS_H

#include<vector>

namespace tba{
	class District_key;
};

std::vector<int> dcmp_slots(tba::District_key const&);
int worlds_slots(tba::District_key const&);

#endif
