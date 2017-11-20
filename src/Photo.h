#pragma once
#ifndef _PHOTO_
#define _PHOTO_
#include"vec4.h"
#include"Camera.h"

class Photo : public Camera
{
public:
	Photo(int numOfimages);
	virtual ~Photo();
	virtual void init(/*const std::string name, const std::string mname,
		const std::string ename,*/
		const std::string cname, const int maxLevel = 1);
	// pairwise distance based on optical center and viewing direction
	int m_num;
	void setDistances(int m_num, std::vector<Photo> photos);

	Vec3f project(const int index, const Vec4f& coord,
		const int level) const;
	std::vector<std::vector<float> > m_distances;

	//------------------------------------------
	
};


#endif // !_PHOTO_




