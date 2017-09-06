#pragma once
#ifndef _SEED_
#define _SEED_

#include <boost/shared_ptr.hpp>
#include <vector>
#include "patch.h"
#include "Cpoint.h"
#include"DetectFeatures.h"

class DetectFeatures;

typedef boost::shared_ptr<Cpoint> Ppoint;

class Seed
{
public:
	Seed(DetectFeatures& detectFeatures);
	virtual ~Seed() {}
	
	void init(const std::vector<std::vector<Cpoint> >& points);/*Priprema prostor za potrebne podatke*/
	void readPoints(const std::vector<std::vector<Cpoint> >& points);/*uciava i premesta prosledjene tacke(ficere) u niz koji je umanjenih dimenzija za csize*/

protected:

	std::vector<std::vector<std::vector<Ppoint> > > m_ppoints;
	DetectFeatures& m_df;
};


#endif // !_SEED_




