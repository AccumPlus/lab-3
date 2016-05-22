#include <vector>
#include <set>

extern "C" int getMinimum (const std::vector<int> &vec)
{
	int index = 0, i = 0;
	std::set<int> set;
	for(auto elem: vec)
	{
		if (set.find(elem) == set.end())
		{
			set.insert(elem);
			index = i;
		}
		++i;
	}
	return index;
}
