#pragma once
#ifndef _PATCH_ORGANIZER_
#define _PATCH_ORGANIZER_
#include"Detector.h"
#include"patch.h"
class DetectFeatures;

class PatchOrganizer
{
public:
	PatchOrganizer(DetectFeatures& detectFeatures);
	void init();

	~PatchOrganizer();

	void clearCounts(void);

	 Patch::Ppatch m_MAXDEPTH;
	 Patch::Ppatch m_BACKGROUND;

	//getMetode
	int getmgHeights(int index) {
		return m_gheights[index];
	}
	
	int getmgWidths(int index) {
		return m_gwidths[index];
	}
	

	DetectFeatures& m_df;
	// image, grid
	std::vector<std::vector<std::vector<Patch::Ppatch> > > m_pgrids;
	// image, grid
	std::vector<std::vector<std::vector<Patch::Ppatch> > > m_vpgrids;
	// Closest patch
	std::vector<std::vector<Patch::Ppatch> > m_dpgrids;
	// Check how many times patch optimization was performed for expansion
	std::vector<std::vector<unsigned char> > m_counts;
	// all the patches in the current level of m_pgrids 
	std::vector<Patch::Ppatch> m_ppatches;
protected:
	// Widths of grids
	std::vector<int> m_gwidths;
	std::vector<int> m_gheights;
	





	
};


#endif // !_PATCH_ORGANIZER_



