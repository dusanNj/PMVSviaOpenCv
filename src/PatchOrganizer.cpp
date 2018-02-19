#define _USE_MATH_DEFINES
#include <cmath>

#include <string>

#include "DetectFeatures.h"
#include "PatchOrganizer.h"
using namespace Patch;

Ppatch m_MAXDEPTH(new Cpatch());
Ppatch m_BACKGROUND(new Cpatch());

PatchOrganizer::PatchOrganizer(DetectFeatures& detectFeatures) : m_df(detectFeatures) {
}

PatchOrganizer::~PatchOrganizer() {
}

void PatchOrganizer::init() {
    // std::cout << "width:" << m_df.getWidtByIndex(0, m_df.getmLevel()) << std::endl;
    m_pgrids.clear();
    m_pgrids.resize(m_df.getNumOfImages());
    m_vpgrids.clear();
    m_vpgrids.resize(m_df.getNumOfImages());
    m_dpgrids.clear();
    m_dpgrids.resize(m_df.getNumOfImages());
    m_counts.clear();
    m_counts.resize(m_df.getNumOfImages());

    m_gwidths.clear();
    m_gwidths.resize(m_df.getNumOfImages());
    m_gheights.clear();
    m_gheights.resize(m_df.getNumOfImages());

    for (int index = 0; index < m_df.getNumOfImages(); ++index) {
        const int gwidth = (m_df.getWidtByIndex(index, m_df.getmLevel())
                            + m_df.getmCsize() - 1) / m_df.getmCsize();
        // std::cout << "width:" << gwidth << std::endl;
        const int gheight = (m_df.getHeightByIndex(index, m_df.getmLevel())
                             + m_df.getmCsize() - 1) / m_df.getmCsize();
        m_gwidths[index] = gwidth;
        m_gheights[index] = gheight;

        if (index < m_df.getNumOfImages()) {
            m_pgrids[index].resize(gwidth * gheight);
            m_vpgrids[index].resize(gwidth * gheight);
            m_dpgrids[index].resize(gwidth * gheight);
            m_counts[index].resize(gwidth * gheight);
            fill(m_dpgrids[index].begin(), m_dpgrids[index].end(), m_MAXDEPTH);
        }
    }
}

void PatchOrganizer::clearCounts(void) {
    for (int index = 0; index < m_df.getNumOfImages(); ++index) {
        std::vector<unsigned char>::iterator begin = m_counts[index].begin();
        std::vector<unsigned char>::iterator end = m_counts[index].end();
        while (begin != end) {
            *begin = (unsigned char)0;
            ++begin;
        }
    }
}

void PatchOrganizer::setScales(Patch::Cpatch& patch) const {
    const float unit = m_df.m_optim.getUnit(patch.m_images[0], patch.m_coord);
    const float unit2 = 2.0f * unit;
    Vec4f ray = patch.m_coord - m_df.getPhoto(patch.m_images[0]).m_center;
    unitize(ray);

    const int inum = (std::min)(m_df.m_tau, (int)patch.m_images.size());

    // First compute, how many pixel difference per unit along vertical
    // for (int i = 1; i < (int)patch.m_images.size(); ++i) {
    for (int i = 1; i < inum; ++i) {
        Vec3f diff = m_df.getPhoto(patch.m_images[i]).project(patch.m_images[i], patch.m_coord, m_df.getmLevel()) -
            m_df.getPhoto(patch.m_images[i]).project(patch.m_images[i], patch.m_coord - ray * unit2, m_df.getmLevel());
        patch.m_dscale += norm(diff);
    }

    // set m_dscale to the vertical distance where average pixel move is half pixel
    // patch.m_dscale /= (int)patch.m_images.size() - 1;
    patch.m_dscale /= inum - 1;
    patch.m_dscale = unit2 / patch.m_dscale;

    patch.m_ascale = atan(patch.m_dscale / (unit * m_df.m_wsize / 2.0f));
}

void PatchOrganizer::setGrids(Patch::Cpatch& patch) const {
    patch.m_grids.clear();
    for (int i = 0; i < (int)patch.m_images.size(); ++i) {
        const int image = patch.m_images[i];
        Vec3f icoord = m_df.getPhoto(image).project(image, patch.m_coord, m_df.getmLevel());
        const int ix = ((int)floor(icoord[0] + 0.5f)) / m_df.getmCsize();
        const int iy = ((int)floor(icoord[1] + 0.5f)) / m_df.getmCsize();
        patch.m_grids.push_back(TVec2<int>(ix, iy));
    }
}

void PatchOrganizer::addPatch(Patch::Ppatch& ppatch) {
    // First handle m_vimages
    std::vector<int>::iterator bimage = ppatch->m_images.begin();
    std::vector<int>::iterator eimage = ppatch->m_images.end();
    std::vector<Vec2i>::iterator bgrid = ppatch->m_grids.begin();
    while (bimage != eimage) {
        const int index = *bimage;
        if (m_df.m_tnum <= index) {
            ++bimage;
            ++bgrid;
            continue;
        }

        const int index2 = (*bgrid)[1] * m_gwidths[index] + (*bgrid)[0];
        // m_df.m_imageLocks[index].wrlock();
        m_pgrids[index][index2].push_back(ppatch);
        // m_df.m_imageLocks[index].unlock();
        ++bimage;
        ++bgrid;
    }

    // If depth, set vimages
    if (m_df.m_depth == 0) {
        return;
    }

    bimage = ppatch->m_vimages.begin();
    eimage = ppatch->m_vimages.end();
    bgrid = ppatch->m_vgrids.begin();

    while (bimage != eimage) {
        const int index = *bimage;
        const int index2 = (*bgrid)[1] * m_gwidths[index] + (*bgrid)[0];
        // m_df.m_imageLocks[index].wrlock();
        m_vpgrids[index][index2].push_back(ppatch);
        // m_df.m_imageLocks[index].unlock();
        ++bimage;
        ++bgrid;
    }
    // TODO 4: odkomentarisati kada se doda metoda updateDepthMap
    // updateDepthMaps(ppatch);
}

void PatchOrganizer::collectPatches(const int target) {
    m_ppatches.clear();

    for (int index = 0; index < m_df.m_tnum; ++index) {
        for (int i = 0; i < (int)m_pgrids[index].size(); ++i) {
            std::vector<Ppatch>::iterator begin = m_pgrids[index][i].begin();
            while (begin != m_pgrids[index][i].end()) {
                (*begin)->m_id = -1;
                begin++;
            }
        }
    }

    int count = 0;
    for (int index = 0; index < m_df.m_tnum; ++index) {
        for (int i = 0; i < (int)m_pgrids[index].size(); ++i) {
            std::vector<Ppatch>::iterator begin = m_pgrids[index][i].begin();
            while (begin != m_pgrids[index][i].end()) {
                if ((*begin)->m_id == -1) {
                    (*begin)->m_id = count++;

                    if ((target == 0) || ((*begin)->m_fix == 0)) {
                        m_ppatches.push_back(*begin);
                    }
                }
                ++begin;
            }
        }
    }
}

void PatchOrganizer::collectPatches(std::priority_queue<Patch::Ppatch,
                                                        std::vector<Patch::Ppatch>,
                                                        P_compare>& pqpatches) {
    for (int index = 0; index < m_df.m_tnum; ++index) {
        for (int i = 0; i < (int)m_pgrids[index].size(); ++i) {
            std::vector<Ppatch>::iterator begin = m_pgrids[index][i].begin();
            while (begin != m_pgrids[index][i].end()) {
                if ((*begin)->m_flag == 0) {
                    (*begin)->m_flag = 1;
                    pqpatches.push(*begin);
                }
                ++begin;
            }
        }
    }
}

void PatchOrganizer::setGridsImages(Patch::Cpatch& patch, const std::vector<int>& images) {
    patch.m_images.clear();
    patch.m_grids.clear();
    std::vector<int>::const_iterator bimage = images.begin();
    std::vector<int>::const_iterator eimage = images.end();
    int t = 0;
    while (bimage != eimage) {
        Vec3f icoord = m_df.getPhoto(t).project(t, patch.m_coord, m_df.getmLevel());
        const int ix = ((int)floor(icoord[0] + 0.5f)) / m_df.getmCsize();
        const int iy = ((int)floor(icoord[1] + 0.5f)) / m_df.getmCsize();
        if ((0 <= ix) && (ix < m_gwidths[*bimage]) &&
            (0 <= iy) && (iy < m_gheights[*bimage])) {
            patch.m_images.push_back(*bimage);
            patch.m_grids.push_back(Vec2i(ix, iy));
        }
        ++bimage;
        t++;
    }
}

void PatchOrganizer::findNeighbors(const Patch::Cpatch& patch,
                                   std::vector<Patch::Ppatch>& neighbors,
                                   const int lock,
                                   const float scale,
                                   const int margin,
                                   const int skipvis) {
    const float radius = 1.5 * margin * m_df.m_exp.computeRadius(patch);

    std::vector<int>::const_iterator bimage = patch.m_images.begin();
    std::vector<int>::const_iterator eimage = patch.m_images.end();
    std::vector<Vec2i>::const_iterator bgrid = patch.m_grids.begin();

    #ifdef DEBUG
        if (patch.m_images.empty()) {
            cerr << "Empty patches in findCloses" << endl;
            exit(1);
        }
    #endif
    float unit = 0.0f;
    for (int i = 0; i < (int)patch.m_images.size(); ++i) {
        unit += m_df.m_optim.getUnit(patch.m_images[i], patch.m_coord);
    }
    unit /= (int)patch.m_images.size();
    unit *= m_df.getmCsize();

    while (bimage != eimage) {
        if (m_df.m_tnum <= *bimage) {
            ++bimage;
            ++bgrid;
            continue;
        }
        const int image = *bimage;
        const int& ix = (*bgrid)[0];
        const int& iy = (*bgrid)[1];
        for (int j = -margin; j <= margin; ++j) {
            const int ytmp = iy + j;
            if ((ytmp < 0) || (m_df.m_pos.m_gheights[image] <= ytmp)) {
                continue;
            }
            for (int i = -margin; i <= margin; ++i) {
                const int xtmp = ix + i;
                if ((xtmp < 0) || (m_df.m_pos.m_gwidths[image] <= xtmp)) {
                    continue;
                }
                const int index = ytmp * m_df.m_pos.m_gwidths[image] + xtmp;
                std::vector<Patch::Ppatch>::const_iterator bpatch =
                    m_df.m_pos.m_pgrids[image][index].begin();
                std::vector<Patch::Ppatch>::const_iterator epatch =
                    m_df.m_pos.m_pgrids[image][index].end();
                while (bpatch != epatch) {
                    if (m_df.isNeighborRadius(patch, **bpatch, unit,
                                              m_df.m_neighborThreshold * scale,
                                              radius)) {
                        neighbors.push_back(*bpatch);
                    }
                    ++bpatch;
                }
                bpatch = m_df.m_pos.m_vpgrids[image][index].begin();
                epatch = m_df.m_pos.m_vpgrids[image][index].end();
                while (bpatch != epatch) {
                    if (m_df.isNeighborRadius(patch, **bpatch, unit,
                                              m_df.m_neighborThreshold * scale,
                                              radius)) {
                        neighbors.push_back(*bpatch);
                    }
                    ++bpatch;
                }
            }
        }

        ++bimage;
        ++bgrid;
    }

    if (skipvis == 0) {
        bimage = patch.m_vimages.begin();
        eimage = patch.m_vimages.end();
        bgrid = patch.m_vgrids.begin();

        while (bimage != eimage) {
            const int image = *bimage;
            const int& ix = (*bgrid)[0];
            const int& iy = (*bgrid)[1];
            for (int j = -margin; j <= margin; ++j) {
                const int ytmp = iy + j;
                if ((ytmp < 0) || (m_df.m_pos.m_gheights[image] <= ytmp)) {
                    continue;
                }
                for (int i = -margin; i <= margin; ++i) {
                    const int xtmp = ix + i;
                    if ((xtmp < 0) || (m_df.m_pos.m_gwidths[image] <= xtmp)) {
                        continue;
                    }
                    const int index = ytmp * m_df.m_pos.m_gwidths[image] + xtmp;
                    std::vector<Patch::Ppatch>::const_iterator bpatch =
                        m_df.m_pos.m_pgrids[image][index].begin();
                    std::vector<Patch::Ppatch>::const_iterator epatch =
                        m_df.m_pos.m_pgrids[image][index].end();
                    while (bpatch != epatch) {
                        if (m_df.isNeighborRadius(patch, **bpatch, unit,
                                                  m_df.m_neighborThreshold * scale,
                                                  radius)) {
                            neighbors.push_back(*bpatch);
                        }
                        ++bpatch;
                    }
                    bpatch = m_df.m_pos.m_vpgrids[image][index].begin();
                    epatch = m_df.m_pos.m_vpgrids[image][index].end();
                    while (bpatch != epatch) {
                        if (m_df.isNeighborRadius(patch, **bpatch, unit,
                                                  m_df.m_neighborThreshold * scale,
                                                  radius)) {
                            neighbors.push_back(*bpatch);
                        }
                        ++bpatch;
                    }
                }
            }

            ++bimage;
            ++bgrid;
        }
    }

    sort(neighbors.begin(), neighbors.end());
    neighbors.erase(unique(neighbors.begin(), neighbors.end()), neighbors.end());
}

void PatchOrganizer::writePLY(const std::vector<Ppatch>& patches, const std::string filename) {
    std::ofstream ofstr;
    ofstr.open(filename.c_str());
    ofstr << "ply" << '\n'
    << "format ascii 1.0" << '\n'
    << "element vertex " << (int)patches.size() << '\n'
    << "property float x" << '\n'
    << "property float y" << '\n'
    << "property float z" << '\n'
    << "property float nx" << '\n'
    << "property float ny" << '\n'
    << "property float nz" << '\n'
    << "property uchar diffuse_red" << '\n'
    << "property uchar diffuse_green" << '\n'
    << "property uchar diffuse_blue" << '\n'
    << "end_header" << '\n';

    std::vector<Ppatch>::const_iterator bpatch = patches.begin();
    std::vector<Ppatch>::const_iterator bend = patches.end();

    while (bpatch != bend) {
        // Get color
        Vec3i color;

        const int mode = 0;
        // 0: color from images
        // 1: fix
        // 2: angle
        if (mode == 0) {
            int denom = 0;
            Vec3f colorf;
            for (int i = 0; i < (int)(*bpatch)->m_images.size(); ++i) {
                const int image = (*bpatch)->m_images[i];
                colorf += m_df.getColor((*bpatch)->m_coord, image, m_df.getmLevel());
                denom++;
            }
            colorf /= denom;
            color[0] = min(255, (int)floor(colorf[0] + 0.5f));
            color[1] = min(255, (int)floor(colorf[1] + 0.5f));
            color[2] = min(255, (int)floor(colorf[2] + 0.5f));
        } else if (mode == 1) {
            if ((*bpatch)->m_tmp == 1.0f) {
                color[0] = 255;
                color[1] = 0;
                color[2] = 0;
            } else {
                color[0] = 255;
                color[1] = 255;
                color[2] = 255;
            }
        } else if (mode == 2) {
            float angle = 0.0f;
            std::vector<int>::iterator bimage = (*bpatch)->m_images.begin();
            std::vector<int>::iterator eimage = (*bpatch)->m_images.end();

            while (bimage != eimage) {
                const int index = *bimage;
                Vec4f ray = m_df.getPhoto(index).m_center - (*bpatch)->m_coord;
                ray[3] = 0.0f;
                unitize(ray);

                angle += acos(ray * (*bpatch)->m_normal);
                ++bimage;
            }

            angle = angle / (M_PI / 2.0f);
            float r, g, b;
            Image im;
            im.gray2rgb(angle, r, g, b);
            color[0] = (int)(r * 255.0f);
            color[1] = (int)(g * 255.0f);
            color[2] = (int)(b * 255.0f);
        }

        ofstr << (*bpatch)->m_coord[0] << ' '
        << (*bpatch)->m_coord[1] << ' '
        << (*bpatch)->m_coord[2] << ' '
        << (*bpatch)->m_normal[0] << ' '
        << (*bpatch)->m_normal[1] << ' '
        << (*bpatch)->m_normal[2] << ' '
        << color[0] << ' ' << color[1] << ' ' << color[2] << '\n';
        ++bpatch;
    }
    ofstr.close();
}

void PatchOrganizer::writePatchesAndImageProjections(const std::string prefix, int numOfImgs) {
    collectPatches(1);

    std::ofstream file;
    file.open(prefix + "patches.txt");
    file << "Number_of_patches ";
    file << m_ppatches.size() << std::endl;
    file << "Number_of_cameras ";
    file << numOfImgs << std::endl;
    int counter = 0;

    for (int p = 0; p < m_ppatches.size(); p++) {
        file << "Patch_ID " << counter << std::endl;
        file << "Position_x " << m_ppatches[p]->m_coord[0] << std::endl;
        file << "Position_y " << m_ppatches[p]->m_coord[1] << std::endl;
        file << "Position_z " << m_ppatches[p]->m_coord[2] << std::endl;

        for (int i = 0; i < m_ppatches[p]->m_images.size(); i++) {
            file << "Camera " << m_ppatches[p]->m_images[i] << std::endl;
            file << "Camera_position_x " << m_ppatches[p]->m_grids[i][0] << std::endl;
            file << "Camera_position_y " << m_ppatches[p]->m_grids[i][1] << std::endl;
            file << "Camera_end" << std::endl;
        }
        file << "Patch_end" << std::endl;
        counter++;
    }
}

void PatchOrganizer::writePatches2(const std::string prefix, bool bExportPLY, bool bExportPatch, bool bExportPSet) {
    collectPatches(1);

    if (bExportPLY) {
        char buffer[1024];
        sprintf(buffer, "%s.ply", prefix.c_str());
        writePLY(m_ppatches, buffer);
    }

    // if (bExportPatch)
    // {
    // char buffer[1024];
    // sprintf(buffer, "%s.patch", prefix.c_str());
    // std::ofstream ofstr;
    // ofstr.open(buffer);
    // ofstr << "PATCHES" << std::endl
    // << (int)m_ppatches.size() << std::endl;
    // for (int p = 0; p < (int)m_ppatches.size(); ++p) {
    // Cpatch patch = *m_ppatches[p];
    // index2image(patch);
    // ofstr << patch << "\n";
    // }
    // ofstr.close();
    // }

    // if (bExportPSet)
    // {
    // char buffer[1024];
    // sprintf(buffer, "%s.pset", prefix.c_str());
    // std::ofstream ofstr;
    // ofstr.open(buffer);
    // for (int p = 0; p < (int)m_ppatches.size(); ++p)
    // ofstr << m_ppatches[p]->m_coord[0] << ' '
    // << m_ppatches[p]->m_coord[1] << ' '
    // << m_ppatches[p]->m_coord[2] << ' '
    // << m_ppatches[p]->m_normal[0] << ' '
    // << m_ppatches[p]->m_normal[1] << ' '
    // << m_ppatches[p]->m_normal[2] << "\n";
    // ofstr.close();
    // }
}

void PatchOrganizer::removePatch(const Patch::Ppatch& ppatch) {
    for (int i = 0; i < (int)ppatch->m_images.size(); ++i) {
        const int image = ppatch->m_images[i];
        if (m_df.m_tnum <= image) {
            continue;
        }

        const int& ix = ppatch->m_grids[i][0];
        const int& iy = ppatch->m_grids[i][1];
        const int index = iy * m_gwidths[image] + ix;
        m_pgrids[image][index].erase(remove(m_pgrids[image][index].begin(),
                                            m_pgrids[image][index].end(),
                                            ppatch),
                                     m_pgrids[image][index].end());
    }

    for (int i = 0; i < (int)ppatch->m_vimages.size(); ++i) {
        const int image = ppatch->m_vimages[i];
        #ifdef DEBUG
            if (m_fm.m_tnum <= image) {
                cerr << "Impossible in removePatch. m_vimages must be targetting images" << endl;
                exit(1);
            }
        #endif

        const int& ix = ppatch->m_vgrids[i][0];
        const int& iy = ppatch->m_vgrids[i][1];
        const int index = iy * m_gwidths[image] + ix;
        m_vpgrids[image][index].erase(remove(m_vpgrids[image][index].begin(),
                                             m_vpgrids[image][index].end(),
                                             ppatch),
                                      m_vpgrids[image][index].end());
    }
}
