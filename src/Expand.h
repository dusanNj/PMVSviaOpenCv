#pragma once
#ifndef _EXPAND_
#define _EXPAND_

#include <vector>
#include <queue>
#include <list>

class DetectFeatures;

class Expand
{
public:
	Expand(DetectFeatures& detectFeatures);
	~Expand();
	void init(void);
protected:
	DetectFeatures& m_df;
};

#endif // !_EXPAND_



