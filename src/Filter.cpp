#include "DetectFeatures.h"
#include "Filter.h"
#include <ctime>
#include <time.h>

Filter::Filter(DetectFeatures& detectFeatures) : m_df(detectFeatures) {
}

Filter::~Filter() {
}

void Filter::init(void) {
}

void Filter::run(void) {
    setDepthMapsVGridsVPGridsAddPatchV(0);
    filterOutside();
    setDepthMapsVGridsVPGridsAddPatchV(1);
    filterExact();
    setDepthMapsVGridsVPGridsAddPatchV(1);
}

void Filter::filterExactThread(void) {
    const int psize = (int)m_df.m_pos.m_ppatches.size();
    std::vector<std::vector<int> > newimages, removeimages;
    std::vector<std::vector<TVec2<int> > > newgrids, removegrids;
    newimages.resize(psize);
    removeimages.resize(psize);
    newgrids.resize(psize);
    removegrids.resize(psize);

    while (1) {
        const int image = m_df.m_count++;
        if (m_df.m_tnum <= image) {
            break;
        }

        std::cerr << '*' << std::flush;

        const int& w = m_df.m_pos.m_gwidths[image];
        const int& h = m_df.m_pos.m_gheights[image];
        int index = -1;
        for (int y = 0; y < h; ++y) {
            for (int x = 0; x < w; ++x) {
                ++index;
                for (int i = 0; i < (int)m_df.m_pos.m_pgrids[image][index].size(); ++i) {
                    const Patch::Cpatch& patch = *m_df.m_pos.m_pgrids[image][index][i];
                    if (patch.m_fix) {
                        continue;
                    }

                    int safe = 0;

                    if (m_df.m_pos.isVisible(patch, image, x, y, m_df.m_neighborThreshold1, 0)) {
                        safe = 1;
                    }
                    // use 4 neighbors?
                    else if ((0 < x) && m_df.m_pos.isVisible(patch, image, x - 1, y, m_df.m_neighborThreshold1, 0)) {
                        safe = 1;
                    } else if ((x < w - 1) && m_df.m_pos.isVisible(patch, image, x + 1, y, m_df.m_neighborThreshold1, 0)) {
                        safe = 1;
                    } else if ((0 < y) && m_df.m_pos.isVisible(patch, image, x, y - 1, m_df.m_neighborThreshold1, 0)) {
                        safe = 1;
                    } else if ((y < h - 1) && m_df.m_pos.isVisible(patch, image, x, y + 1, m_df.m_neighborThreshold1, 0)) {
                        safe = 1;
                    }

                    if (safe) {
                        newimages[patch.m_id].push_back(image);
                        newgrids[patch.m_id].push_back(TVec2<int>(x, y));
                    } else {
                        removeimages[patch.m_id].push_back(image);
                        removegrids[patch.m_id].push_back(TVec2<int>(x, y));
                    }
                }
            }
        }
    }

    for (int p = 0; p < psize; ++p) {
        m_newimages[p].insert(m_newimages[p].end(),
                              newimages[p].begin(), newimages[p].end());
        m_newgrids[p].insert(m_newgrids[p].end(),
                             newgrids[p].begin(), newgrids[p].end());
        m_removeimages[p].insert(m_removeimages[p].end(),
                                 removeimages[p].begin(), removeimages[p].end());
        m_removegrids[p].insert(m_removegrids[p].end(),
                                removegrids[p].begin(), removegrids[p].end());
    }
}

void Filter::filterExact(void) {
    time_t tv;
    time(&tv);
    time_t curtime = tv;
    std::cerr << "Filter Exact: " << std::flush;

    // ??? cannot use (1) because we use patch.m_id to set newimages,....
    m_df.m_pos.collectPatches();
    const int psize = (int)m_df.m_pos.m_ppatches.size();

    // dis associate images
    m_newimages.clear();
    m_newgrids.clear();
    m_removeimages.clear();
    m_removegrids.clear();
    m_newimages.resize(psize);
    m_newgrids.resize(psize);
    m_removeimages.resize(psize);
    m_removegrids.resize(psize);

    m_df.m_count = 0;

    filterExactThread();

    // ----------------------------------------------------------------------
    for (int p = 0; p < psize; ++p) {
        if (m_df.m_pos.m_ppatches[p]->m_fix) {
            continue;
        }

        for (int i = 0; i < (int)m_removeimages[p].size(); ++i) {
            const int index = m_removeimages[p][i];
            if (m_df.m_tnum <= index) {
                std::cerr << "MUST NOT COME HERE" << std::endl;
                exit(1);
            }
            const int ix = m_removegrids[p][i][0];
            const int iy = m_removegrids[p][i][1];
            const int index2 = iy * m_df.m_pos.m_gwidths[index] + ix;

            m_df.m_pos.m_pgrids[index][index2].erase(remove(m_df.m_pos.m_pgrids[index][index2].begin(),
                                                            m_df.m_pos.m_pgrids[index][index2].end(),
                                                            m_df.m_pos.m_ppatches[p]),
                                                     m_df.m_pos.m_pgrids[index][index2].end());
        }
    }

    // m_df.m_debug = 1;

    int count = 0;
    for (int p = 0; p < psize; ++p) {
        if (m_df.m_pos.m_ppatches[p]->m_fix) {
            continue;
        }

        Patch::Cpatch& patch = *m_df.m_pos.m_ppatches[p];

        // This should be images in targetting images. Has to come before the next for-loop.
        patch.m_timages = (int)m_newimages[p].size();

        for (int i = 0; i < (int)patch.m_images.size(); ++i) {
            const int& index = patch.m_images[i];
            if (m_df.m_tnum <= index) {
                m_newimages[p].push_back(patch.m_images[i]);
                m_newgrids[p].push_back(patch.m_grids[i]);
            }
        }

        patch.m_images.swap(m_newimages[p]);
        patch.m_grids.swap(m_newgrids[p]);

        if (m_df.m_minimagenumthresho <= (int)patch.m_images.size()) {
            m_df.m_optim.setRefImage(patch, 0);
            m_df.m_pos.setGrids(patch);
        }

        if ((int)patch.m_images.size() < m_df.m_minimagenumthresho) {
            m_df.m_pos.removePatch(m_df.m_pos.m_ppatches[p]);
            count++;
        }
    }
    time(&tv);
    std::cerr << (int)m_df.m_pos.m_ppatches.size() << " -> "
    << (int)m_df.m_pos.m_ppatches.size() - count << " ("
    << 100 * ((int)m_df.m_pos.m_ppatches.size() - count) / (float)m_df.m_pos.m_ppatches.size()
    << "%)\t" << (tv - curtime) / CLOCKS_PER_SEC << " secs" << std::endl;
}

void Filter::filterOutside(void) {
    time_t tv;
    time(&tv);
    time_t curtime = tv;
    std::cerr << "FilterOutside" << std::endl;
    // ??? notice (1) here to avoid removing m_fix=1
    m_df.m_pos.collectPatches(1);

    const int psize = (int)m_df.m_pos.m_ppatches.size();
    m_gains.resize(psize);

    std::cerr << "mainbody: " << std::flush;

    m_df.m_count = 0;
    // filter outside ///////////////////////////////////////////////////////////////////
    const int id = m_df.m_count++;


    const int size = (int)m_df.m_pos.m_ppatches.size();
    const int itmp = (int)ceil(size / (float)1);
    const int begin = id * itmp;
    const int end = min(size, (id + 1) * itmp);

    for (int p = begin; p < end; ++p) {
        Patch::Ppatch& ppatch = m_df.m_pos.m_ppatches[p];
        m_gains[p] = ppatch->score2(m_df.m_nccThreshold);

        const int size = (int)ppatch->m_images.size();
        for (int i = 0; i < size; ++i) {
            const int& index = ppatch->m_images[i];
            if (m_df.m_tnum <= index) {
                continue;
            }

            const int& ix = ppatch->m_grids[i][0];
            const int& iy = ppatch->m_grids[i][1];
            const int index2 = iy * m_df.m_pos.m_gwidths[index] + ix;

            float maxpressure = 0.0f;
            for (int j = 0; j < (int)m_df.m_pos.m_pgrids[index][index2].size(); ++j) {
                if (!m_df.isNeighbor(*ppatch, *m_df.m_pos.m_pgrids[index][index2][j],
                                     m_df.m_neighborThreshold1)) {
                    maxpressure = max(maxpressure, m_df.m_pos.m_pgrids[index][index2][j]->m_ncc -
                                      m_df.m_nccThreshold);
                }
            }

            m_gains[p] -= maxpressure;
        }

        const int vsize = (int)ppatch->m_vimages.size();
        for (int i = 0; i < vsize; ++i) {
            const int& index = ppatch->m_vimages[i];
            if (m_df.m_tnum <= index) {
                continue;
            }
            // m_df.getPhoto(t).project(t, patch.m_coord, m_df.getmLevel());

            const float pdepth = m_df.getPhoto(index).computeDepth(index, ppatch->m_coord);

            const int& ix = ppatch->m_vgrids[i][0];
            const int& iy = ppatch->m_vgrids[i][1];
            const int index2 = iy * m_df.m_pos.m_gwidths[index] + ix;
            float maxpressure = 0.0f;

            for (int j = 0; j < (int)m_df.m_pos.m_pgrids[index][index2].size(); ++j) {
                const float bdepth = m_df.getPhoto(index).computeDepth(index, m_df.m_pos.m_pgrids[index][index2][j]->m_coord);
                if ((pdepth < bdepth) &&
                    !m_df.isNeighbor(*ppatch, *m_df.m_pos.m_pgrids[index][index2][j],
                                     m_df.m_neighborThreshold1)) {
                    maxpressure = max(maxpressure,
                                      m_df.m_pos.m_pgrids[index][index2][j]->m_ncc -
                                      m_df.m_nccThreshold);
                }
            }
            m_gains[p] -= maxpressure;
        }
    }

    //////////////////////////////////////

    // delete patches with positive m_gains
    int count = 0;

    double ave = 0.0f;
    double ave2 = 0.0f;
    int denom = 0;

    for (int p = 0; p < psize; ++p) {
        ave += m_gains[p];
        ave2 += m_gains[p] * m_gains[p];
        ++denom;

        if (m_gains[p] < 0.0) {
            m_df.m_pos.removePatch(m_df.m_pos.m_ppatches[p]);
            count++;
        }
    }

    if (denom == 0) {
        denom = 1;
    }
    ave /= denom;
    ave2 /= denom;
    ave2 = sqrt(max(0.0, ave2 - ave * ave));
    std::cerr << "Gain (ave/var): " << ave << ' ' << ave2 << std::endl;

    time(&tv);
    std::cerr << (int)m_df.m_pos.m_ppatches.size() << " -> "
    << (int)m_df.m_pos.m_ppatches.size() - count << " ("
    << 100 * ((int)m_df.m_pos.m_ppatches.size() - count) / (float)m_df.m_pos.m_ppatches.size()
    << "%)\t" << (tv - curtime) / CLOCKS_PER_SEC << " secs" << std::endl;
}

void Filter::setDepthMaps(void) {
    // initialize
    for (int index = 0; index < m_df.m_tnum; ++index) {
        std::fill(m_df.m_pos.m_dpgrids[index].begin(), m_df.m_pos.m_dpgrids[index].end(),
                  m_df.m_pos.m_MAXDEPTH);
    }

    while (1) {
        const int index = m_df.m_count++;
        if (m_df.m_tnum <= index) {
            break;
        }

        const int gwidth = m_df.m_pos.m_gwidths[index];
        const int gheight = m_df.m_pos.m_gheights[index];

        std::vector<Patch::Ppatch>::iterator bpatch = m_df.m_pos.m_ppatches.begin();
        std::vector<Patch::Ppatch>::iterator epatch = m_df.m_pos.m_ppatches.end();

        while (bpatch != epatch) {
            Patch::Ppatch& ppatch = *bpatch;
            const Vec3f icoord =
                m_df.getPhoto(index).project(index, ppatch->m_coord, m_df.getmLevel());

            const float fx = icoord[0] / m_df.getmCsize();
            const int xs[2] = {
                (int)floor(fx), (int)ceil(fx)
            };
            const float fy = icoord[1] / m_df.getmCsize();
            const int ys[2] = {
                (int)floor(fy), (int)ceil(fy)
            };

            const float depth =
                m_df.getPhoto(index).m_oaxis * ppatch->m_coord;

            for (int j = 0; j < 2; ++j) {
                for (int i = 0; i < 2; ++i) {
                    if ((xs[i] < 0) || (gwidth <= xs[i]) || (ys[j] < 0) || (gheight <= ys[j])) {
                        continue;
                    }
                    const int index2 = ys[j] * gwidth + xs[i];

                    if (m_df.m_pos.m_dpgrids[index][index2] == m_df.m_pos.m_MAXDEPTH) {
                        m_df.m_pos.m_dpgrids[index][index2] = ppatch;
                    } else {
                        const float dtmp = m_df.getPhoto(index).m_oaxis *
                            m_df.m_pos.m_dpgrids[index][index2]->m_coord;

                        if (depth < dtmp) {
                            m_df.m_pos.m_dpgrids[index][index2] = ppatch;
                        }
                    }
                }
            }
            ++bpatch;
        }
    }
}

void Filter::setDepthMapsVGridsVPGridsAddPatchV(const int additive) {
    m_df.m_pos.collectPatches();
    setDepthMaps();

    // clear m_vpgrids
    for (int index = 0; index < m_df.m_tnum; ++index) {
        std::vector<std::vector<Patch::Ppatch> >::iterator bvvp = m_df.m_pos.m_vpgrids[index].begin();
        std::vector<std::vector<Patch::Ppatch> >::iterator evvp = m_df.m_pos.m_vpgrids[index].end();
        while (bvvp != evvp) {
            (*bvvp).clear();
            ++bvvp;
        }
    }

    if (additive == 0) {
        // initialization
        std::vector<Patch::Ppatch>::iterator bpatch = m_df.m_pos.m_ppatches.begin();
        std::vector<Patch::Ppatch>::iterator epatch = m_df.m_pos.m_ppatches.end();
        while (bpatch != epatch) {
            (*bpatch)->m_vimages.clear();
            (*bpatch)->m_vgrids.clear();
            ++bpatch;
        }
    }
    m_df.m_count = 0;
    setVGridsVPGridsThread();

    m_df.m_count = 0;
    addPatchVThread();
}

void Filter::addPatchVThread(void) {
    while (1) {
        const int index = m_df.m_count++;
        if (m_df.m_tnum <= index) {
            break;
        }

        std::vector<Patch::Ppatch>::iterator bpatch = m_df.m_pos.m_ppatches.begin();
        std::vector<Patch::Ppatch>::iterator epatch = m_df.m_pos.m_ppatches.end();
        while (bpatch != epatch) {
            Patch::Ppatch& ppatch = *bpatch;
            std::vector<int>::iterator bimage = ppatch->m_vimages.begin();
            std::vector<int>::iterator eimage = ppatch->m_vimages.end();
            std::vector<Vec2i>::iterator bgrid = ppatch->m_vgrids.begin();
            while (bimage != eimage) {
                if (*bimage == index) {
                    const int& ix = (*bgrid)[0];
                    const int& iy = (*bgrid)[1];
                    const int index2 = iy * m_df.m_pos.m_gwidths[index] + ix;
                    m_df.m_pos.m_vpgrids[index][index2].push_back(ppatch);
                    break;
                }
                ++bimage;
                ++bgrid;
            }
            ++bpatch;
        }
    }
}

void Filter::setVGridsVPGridsThread(void) {
    const int noj = 1000;
    const int size = (int)m_df.m_pos.m_ppatches.size();
    const int job = max(1, size / (noj - 1));

    while (1) {
        const int id = m_df.m_count++;
        const int begin = id * job;
        const int end = min(size, (id + 1) * job);

        if (size <= begin) {
            break;
        }

        // add patches to m_vpgrids
        for (int p = begin; p < end; ++p) {
            Patch::Ppatch& ppatch = m_df.m_pos.m_ppatches[p];
            m_df.m_pos.setVImagesVGrids(ppatch);
        }
    }
}
