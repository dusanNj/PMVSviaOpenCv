#pragma once
#ifndef _EXPAND_
    #define _EXPAND_

    #include <list>
    #include <queue>
    #include <vector>

    #include "patch.h"
    #include "PatchOrganizer.h"

    class DetectFeatures;

    class Expand {
        public:
            Expand(DetectFeatures& detectFeatures);
            ~Expand();
            void init(void);
            void run(void);
            void findEmptyBlocks(const Patch::Ppatch& ppatch, std::vector<std::vector<Vec4f> >& canCoords);
            float computeRadius(const Patch::Cpatch& patch);
            int expandSub(const Patch::Ppatch& orgppatch, const int id, const Vec4f& canCoord);
            int checkCounts(Patch::Cpatch& patch);
            int updateCounts(const Patch::Cpatch& patch);

        protected:
            DetectFeatures& m_df;
            std::priority_queue<Patch::Ppatch, std::vector<Patch::Ppatch>, P_compare>
            m_queue;

            // Number of trials
            std::vector<int> m_ecounts;
            // Number of failures in the prep
            std::vector<int> m_fcounts0;
            // Number of failures in the post processing
            std::vector<int> m_fcounts1;
            // Number passes
            std::vector<int> m_pcounts;
    };


#endif  // !_EXPAND_
