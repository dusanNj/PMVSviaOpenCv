#pragma once
#ifndef _FILTER_
    #define _FILTER_

    #include "patch.h"
    #include "vec2.h"
    #include <list>
    // #include"DetectFeatures.h"

    class DetectFeatures;

    class Filter {
        public:
            Filter(DetectFeatures& detectFeatures);
            ~Filter();

            void init(void);
            void run(void);
            void setDepthMaps(void);
            void setDepthMapsVGridsVPGridsAddPatchV(const int additive);
            void filterOutside(void);
            void filterExact(void);
            void setVGridsVPGridsThread(void);
            void addPatchVThread(void);
            void filterExactThread(void);
            void filterNeighbor(const int times);
            void filterNeighborThread(void);
            int filterQuad(const Patch::Cpatch& patch, const std::vector<Patch::Ppatch>& neighbors);
            void filterSmallGroups(void);
            void filterSmallGroupsSub(const int pid, const int id, std::vector<int>& label, std::list<int>& ltmp);
            std::vector<float> m_gains;

        protected:
            DetectFeatures& m_df;

            int m_time;
            std::vector<int> m_rejects;
            std::vector<std::vector<int> > m_newimages, m_removeimages;
            std::vector<std::vector<TVec2<int> > > m_newgrids, m_removegrids;
    };


#endif  // !_FILTER_
