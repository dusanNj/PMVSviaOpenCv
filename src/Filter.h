#pragma once
#ifndef _FILTER_
#define _FILTER_

#include "patch.h"
#include <list>
#include "vec2.h"
//#include"DetectFeatures.h"

class DetectFeatures;

class Filter {
public:
	Filter(DetectFeatures& detectFeatures);
	~Filter();

	void init(void);
protected:
	DetectFeatures& m_df;

};

#endif // !_FILTER_
