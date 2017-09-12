#pragma once
#ifndef _CAMERA_
#define _CAMERA_

#include <vector>
#include <string>
#include <climits>
#include <algorithm>
#include "vec4.h"
#include "mat4.h"
#include "mat3.h"

class Camera
{
public:
	Camera(void);
	virtual ~Camera();

	virtual void init(const std::string cname, const int maxLevel);
	void updateCamera(void);
	Vec4f getOpticalCenter(void) const;
	void updateProjection(void);
	static void setProjection(const std::vector<float>& intrinsics,
		const std::vector<float>& extrinsics,
		std::vector<Vec4f>& projection,
		const int txtType);
	static void q2proj(const double q[6], Mat4& mat);
	static void setProjectionSub(double params[], std::vector<Vec4f>& projection,
		const int level);

	//----------------------------------------------------------------------
	// txt file name
	std::string m_cname;
	// Optical center
	Vec4f m_center;
	// Optical axis
	Vec4f m_oaxis;

	float m_ipscale;
	// 3x4 projection matrix
	std::vector<std::vector<Vec4f> > m_projection;
	Vec3f m_xaxis;
	Vec3f m_yaxis;
	Vec3f m_zaxis;

	//cuvanje osa i centara kamera.
	std::vector<Vec4f> cam_m_center;
	std::vector<Vec4f> cam_m_oaxis;
	
	// intrinsic and extrinsic camera parameters. Compact form.
	std::vector<float> m_intrinsics;
	std::vector<float> m_extrinsics;
	// camera parameter type
	int m_txtType;

protected:
	int m_maxLevel;
	float m_axesScale;
};

#endif // !_CAMERA_



