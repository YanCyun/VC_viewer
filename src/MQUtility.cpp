#define MQUtility_CPP_20080508

#include "MQUtility.h"


template <typename T1, typename T2>
bool MQComplexItem<T1, T2>::operator<(const MQComplexItem& ROP) const
{
	if(this->Part1 < ROP.Part1)
		return true;
	else
		return false;
}

template <typename T1, typename T2>
MQComplexItem<T1, T2>::MQComplexItem()
{

}

template <typename T1, typename T2>
MQComplexItem<T1, T2>::MQComplexItem(T1 IPart1, T2 IPart2)
:Part1(IPart1), Part2(IPart2)
{

}

template <typename T1, typename T2>
MQComplexItem<T1, T2>::~MQComplexItem()
{

}
