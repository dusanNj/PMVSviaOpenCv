#include "Camera.h"
#include <cstdlib>
#include <fstream>
#define _USE_MATH_DEFINES
#include <math.h>


Camera::Camera(void) {
    m_axesScale = 1.0f;
    m_maxLevel = 1;
}

Camera::~Camera() {
}

void Camera::init(const std::string cname, const int maxLevel) {
    m_cname = cname;
    m_maxLevel = maxLevel;

    // initialize camera
    m_intrinsics.resize(6);
    m_extrinsics.resize(6);

    std::ifstream ifstr;
    ifstr.open(cname.c_str());

    std::string header;
    ifstr >> header;
    if (header == "CONTOUR") {
        m_txtType = 0;
    } else if (header == "CONTOUR2") {
        m_txtType = 2;
    } else if (header == "CONTOUR3") {
        m_txtType = 3;
    } else {
        std::cerr << "Unrecognizable txt format" << std::endl;
        exit(1);
    }

    for (int i = 0; i < 6; ++i) {
        ifstr >> m_intrinsics[i];
    }
    for (int i = 0; i < 6; ++i) {
        ifstr >> m_extrinsics[i];
    }

    ifstr.close();

    // ----------------------------------------------------------------------
    m_projection.resize(maxLevel);
    for (int level = 0; level < maxLevel; ++level) {
        m_projection[level].resize(3);
    }

    updateCamera();
}

float Camera::computeDepth(const Vec4f& point) {
    // orthographic projection case
    if ((m_projection[0][2][0] == 0.0) && (m_projection[0][2][1] == 0.0) &&
        (m_projection[0][2][2] == 0.0)) {
        // cerr << "Because I'm using negative depth to represent flip. this could be a problem" << endl;
        return -m_center * point;
    } else {
        return m_oaxis * point;
    }
}

void Camera::updateCamera(void) {
    updateProjection();

    // ----------------------------------------------------------------------
    m_oaxis = m_projection[0][2];
    m_oaxis[3] = 0.0;
    const float ftmp = norm(m_oaxis);
    m_oaxis[3] = m_projection[0][2][3];
    m_oaxis /= ftmp;
    cam_m_oaxis.push_back(m_oaxis);
    m_center = getOpticalCenter();
    cam_m_center.push_back(m_center);
    m_zaxis = Vec3f(m_oaxis[0], m_oaxis[1], m_oaxis[2]);
    m_xaxis = Vec3f(m_projection[0][0][0],
                    m_projection[0][0][1],
                    m_projection[0][0][2]);
    m_yaxis = cross(m_zaxis, m_xaxis);
    unitize(m_yaxis);
    m_xaxis = cross(m_yaxis, m_zaxis);

    Vec4f xaxis = m_projection[0][0];
    xaxis[3] = 0.0f;
    Vec4f yaxis = m_projection[0][1];
    yaxis[3] = 0.0f;
    float ftmp2 = (norm(xaxis) + norm(yaxis)) / 2.0f;
    if (ftmp2 == 0.0f) {
        ftmp2 = 1.0f;
    }
    m_ipscale = ftmp2;
}

Vec4f Camera::getOpticalCenter(void) const {
    // orthographic case
    Vec4f ans;
    if ((m_projection[0][2][0] == 0.0) && (m_projection[0][2][1] == 0.0) &&
        (m_projection[0][2][2] == 0.0)) {
        Vec3f vtmp[2];
        for (int i = 0; i < 2; ++i) {
            for (int y = 0; y < 3; ++y) {
                vtmp[i][y] = m_projection[0][i][y];
            }
        }

        Vec3f vtmp2 = cross(vtmp[0], vtmp[1]);
        unitize(vtmp2);
        for (int y = 0; y < 3; ++y) {
            ans[y] = vtmp2[y];
        }
        ans[3] = 0.0;
    } else {
        Mat3 A;
        Vec3 b;
        for (int y = 0; y < 3; ++y) {
            for (int x = 0; x < 3; ++x) {
                A[y][x] = m_projection[0][y][x];
            }
            b[y] = -m_projection[0][y][3];
        }
        Mat3 iA;
        invert(iA, A);
        b = iA * b;

        for (int y = 0; y < 3; ++y) {
            ans[y] = b[y];
        }
        ans[3] = 1.0;
    }
    return ans;
}

void Camera::updateProjection(void) {
    // Set bottom level
    setProjection(m_intrinsics, m_extrinsics, m_projection[0], m_txtType);

    for (int level = 1; level < m_maxLevel; ++level) {
        for (int i = 0; i < 3; ++i) {
            m_projection[level][i] = m_projection[level - 1][i];
        }

        m_projection[level][0] /= 2.0;
        m_projection[level][1] /= 2.0;
    }
}

void Camera::setProjection(const std::vector<float>& intrinsics,
                           const std::vector<float>& extrinsics,
                           std::vector<Vec4f>& projection,
                           const int txtType) {
    projection.resize(3);
    double params[12];
    for (int i = 0; i < 6; ++i) {
        params[i] = intrinsics[i];
        params[6 + i] = extrinsics[i];
    }

    if (txtType == 0) {
        for (int y = 0; y < 3; ++y) {
            for (int x = 0; x < 4; ++x) {
                projection[y][x] = params[4 * y + x];
            }
        }
        // projection[1] = - projection[1];
    } else if (txtType == 2) {
        Mat4 K;
        for (int y = 0; y < 4; ++y) {
            for (int x = 0; x < 4; ++x) {
                K[y][x] = 0.0;
            }
        }

        K[0][0] = params[0];
        K[1][1] = params[1];
        K[0][1] = params[2];
        K[0][2] = params[3];
        K[1][2] = params[4];
        K[2][2] = 1.0;
        K[3][3] = 1.0;

        Mat4 mtmp;
        q2proj(&params[6], mtmp);
        mtmp = K * mtmp;

        for (int y = 0; y < 3; ++y) {
            for (int x = 0; x < 4; ++x) {
                projection[y][x] = mtmp[y][x];
            }
        }
    } else if (txtType == 3) {
        // parameters
        // # first intrinsics
        // fovx width height 0 0 0
        // # second extrinsics
        // tx ty tz rx ry rz
        double params2[9] = {
            params[0], params[1], params[2],
            params[6], params[7], params[8],
            params[9], params[10], params[11]
        };

        setProjectionSub(params2, projection, 0);

        /*
        cout << endl;
        for (int y = 0; y < 3; ++y) {
        if (y == 1)
        cout << -projection[y] << endl;
        else
        cout << projection[y] << endl;
        }
        cout << endl;
        */
    } else {
        std::cerr << "Impossible setProjection" << std::endl;
        exit(1);
    }
}

void Camera::q2proj(const double q[6], Mat4& mat) {
    const double a = q[0] * M_PI / 180.0;
    const double b = q[1] * M_PI / 180.0;
    const double g = q[2] * M_PI / 180.0;

    const double s1 = sin(a);
    const double s2 = sin(b);
    const double s3 = sin(g);
    const double c1 = cos(a);
    const double c2 = cos(b);
    const double c3 = cos(g);

    /*   Premiere colonne*/	/*   Seconde colonne	*/
    mat[0][0] = c2 * c3;
    mat[0][1] = c3 * s2 * s1 - s3 * c1;
    mat[1][0] = s3 * c2;
    mat[1][1] = s3 * s2 * s1 + c3 * c1;
    mat[2][0] = -s2;
    mat[2][1] = c2 * s1;

    /*   Troisieme colonne*/	/*  Quatrieme colonne	*/
    mat[0][2] = c3 * s2 * c1 + s3 * s1;
    mat[0][3] = q[3];
    mat[1][2] = s3 * s2 * c1 - c3 * s1;
    mat[1][3] = q[4];
    mat[2][2] = c2 * c1;
    mat[2][3] = q[5];

    mat[3][0] = mat[3][1] = mat[3][2] = 0.0;
    mat[3][3] = 1.0;
}

void Camera::setProjectionSub(double params[], std::vector<Vec4f>& projection, const int level) {
    const double rx = params[6] * M_PI / 180.0;
    const double ry = params[7] * M_PI / 180.0;
    const double rz = params[8] * M_PI / 180.0;

    const double fovx = params[0] * M_PI / 180.0;

    const double f = params[1] / 2.0 / tan(fovx / 2.0);
    Mat3 K;
    K[0] = Vec3(f, 0.0, 0.0);
    K[1] = Vec3(0.0, f, 0.0);
    K[2] = Vec3(0.0, 0.0, -1.0);

    Mat3 trans;
    trans[0] = Vec3(1.0, 0.0, params[1] / 2.0);
    trans[1] = Vec3(0.0, -1.0, params[2] / 2.0);
    trans[2] = Vec3(0.0, 0.0, 1.0);

    K = trans * K;

    Mat3 Rx;
    Rx[0] = Vec3(1.0, 0.0, 0.0);
    Rx[1] = Vec3(0.0f, cos(rx), -sin(rx));
    Rx[2] = Vec3(0.0, sin(rx), cos(rx));

    Mat3 Ry;
    Ry[0] = Vec3(cos(ry), 0, sin(ry));
    Ry[1] = Vec3(0.0, 1.0, 0.0);
    Ry[2] = Vec3(-sin(ry), 0, cos(ry));

    Mat3 Rz;
    Rz[0] = Vec3(cos(rz), -sin(rz), 0.0);
    Rz[1] = Vec3(sin(rz), cos(rz), 0.0);
    Rz[2] = Vec3(0.0, 0.0, 1.0);

    // ????????
    // Mat3 R = transpose(Rz) * transpose(Rx) * transpose(Ry);
    Mat3 R = transpose(Rx) * transpose(Ry) * transpose(Rz);

    Vec3 t(params[3], params[4], params[5]);

    Mat3 left = K * R;
    Vec3 right = -K * (R * t);

    for (int y = 0; y < 3; ++y) {
        for (int x = 0; x < 3; ++x) {
            projection[y][x] = left[y][x];
        }
        projection[y][3] = right[y];
    }

    const int scale = 0x0001 << level;
    projection[0] /= scale;
    projection[1] /= scale;
}
