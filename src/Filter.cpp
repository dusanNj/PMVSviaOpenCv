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
    filterOutside();
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
