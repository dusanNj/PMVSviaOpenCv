#include"Filter.h"
#include"DetectFeatures.h"

Filter::Filter(DetectFeatures& detectFeatures) : m_df(detectFeatures) {
	
}

Filter::~Filter() {

}

void Filter::init(void){}