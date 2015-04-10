#ifndef MQUtility_H_20080508
#define MQUtility_H_20080508


template <typename T1, typename T2>
class MQComplexItem
{
public:
	T1 Part1;
	T2 Part2;
	bool operator<(const MQComplexItem& ROP) const;

public:
	MQComplexItem();
	MQComplexItem(T1 IPart1, T2 IPart2);
	~MQComplexItem();
};


#ifndef MQUtility_CPP_20080508
#include "MQUtility.cpp"
#endif


#endif	//MQUtility_H_20080508