#pragma once
#ifndef _SEED_
#define _SEED_

#include <boost/shared_ptr.hpp>
#include <vector>
#include "patch.h"
#include "Cpoint.h"
#include"mat3.h"
//#include"PatchOrganizer.h"
class DetectFeatures;


typedef boost::shared_ptr<Cpoint> Ppoint;

class Seed
{
public:
	Seed(DetectFeatures& detectFeatures);
	virtual ~Seed() {}
	
	void init(const std::vector<std::vector<Cpoint> >& points);/*Priprema prostor za potrebne podatke*/
	void readPoints(const std::vector<std::vector<Cpoint> >& points);/*uciava i premesta prosledjene tacke(ficere) u niz koji je umanjenih dimenzija za csize*/
	void run();//zapocinje kreiranje patch-eva i matching tacaka
	void initialMatchT(void); //pocetak mecovanja, prolazi kroz while petlju i prosledjuje u sledecu metodu index i id;
	void initialMatch(const int index, const int id);
	int canAdd(const int index, const int x, const int y);
	void collectCells(const int index0, const int index1,
		const Cpoint& p0, std::vector<Vec2i>& cells);
	void collectCandidates(const int index, const std::vector<int>& indexes,
		const Cpoint& point, std::vector<Ppoint>& vcp);
	void unproject(const int index0, const int index1,
		const Cpoint& p0, const Cpoint& p1,
		Vec4f& coord) const;
	
	int initialMatchSub(const int index0, const int index1,
		const int id, Patch::Cpatch& patch);
	DetectFeatures& m_df;
	// Number of trials
	std::vector<int> m_scounts;

	//My temp Point 3d
	std::vector<std::vector<Ppoint>> temp3Dpoint;
	std::vector<Ppoint> temp3DpointB;
	// Number of failures in the prep
	std::vector<int> m_fcounts0;

protected:

	std::vector<std::vector<std::vector<Ppoint> > > m_ppoints;
	
};


#endif // !_SEED_




