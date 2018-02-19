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
            void filterOutside(void);

            std::vector<float> m_gains;

        protected:
            DetectFeatures& m_df;
    };


#endif  // !_FILTER_
