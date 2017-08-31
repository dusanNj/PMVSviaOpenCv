#include "Cpoint.h"



Cpoint::Cpoint(void)
{
	m_response = -1.0;
	m_type = 1;
}


Cpoint::~Cpoint()
{
}

std::istream& operator >> (std::istream& istr, Cpoint& cps) {

	std::string header;
	char str[1024];
	istr >> str;
	header = std::string(str);
	istr >> cps.m_icoord[0] >> cps.m_icoord[1] >> cps.m_response >> cps.m_type;
	cps.m_icoord[2] = 1.0f;
	return istr;
}

std::ostream& operator <<(std::ostream& ostr, const Cpoint& cps) {
	ostr << "POINT0" << std::endl
		<< cps.m_icoord[0] << ' ' << cps.m_icoord[1] << ' ' << cps.m_response << ' '
		<< cps.m_type;
	return ostr;
}

bool SortCpoint(const Cpoint& a, const Cpoint& b)
{
	return a.m_response < b.m_response;
}
