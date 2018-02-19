#pragma once
#ifndef _PATCH_ORGANIZER_
    #define _PATCH_ORGANIZER_
    #include "Detector.h"
    #include "patch.h"
    #include <fstream>
    #include <queue>
    class DetectFeatures;

    class P_compare {
        public:
            bool operator()(const Patch::Ppatch& lhs, const Patch::Ppatch& rhs) const {
                return lhs->m_tmp < rhs->m_tmp;
            }
    };


    class PatchOrganizer {
        public:
            PatchOrganizer(DetectFeatures& detectFeatures);
            void init();

            ~PatchOrganizer();

            void clearCounts(void);

            Patch::Ppatch m_MAXDEPTH;
            Patch::Ppatch m_BACKGROUND;

            // getMetode
            int getmgHeights(int index) {
                return m_gheights[index];
            }

            int getmgWidths(int index) {
                return m_gwidths[index];
            }

            void setScales(Patch::Cpatch& patch) const;
            void setGrids(Patch::Cpatch& patch) const;
            void addPatch(Patch::Ppatch& ppatch);
            void removePatch(const Patch::Ppatch& ppatch);
            void writePatches2(const std::string prefix, bool bExportPLY, bool bExportPatch, bool bExportPSet);
            void writePatchesAndImageProjections(const std::string prefix, int numOfImgs);
            void collectPatches(std::priority_queue<Patch::Ppatch,
                                                    std::vector<Patch::Ppatch>,
                                                    P_compare>& pqpatches);
            void collectPatches(const int target = 0);
            void findNeighbors(const Patch::Cpatch& patch,
                               std::vector<Patch::Ppatch>& neighbors,
                               const int lock,
                               const float scale,
                               const int margin,
                               const int skipvis);
            void setGridsImages(Patch::Cpatch& patch, const std::vector<int>& images);
            void writePLY(const std::vector<Patch::Ppatch>& patches, const std::string filename);
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

            // Widths of grids
            std::vector<int> m_gwidths;
            std::vector<int> m_gheights;
    };


#endif  // !_PATCH_ORGANIZER_
