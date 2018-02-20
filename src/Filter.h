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

            std::vector<float> m_gains;

        protected:
            DetectFeatures& m_df;

            std::vector<std::vector<int> > m_newimages, m_removeimages;
            std::vector<std::vector<TVec2<int> > > m_newgrids, m_removegrids;
    };


#endif  // !_FILTER_
