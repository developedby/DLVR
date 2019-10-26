#ifndef street_section_h_
#define street_section_h_

#include <vector>
#include <utility>

namespace street_lines
{
    enum class Color {blue, green, yellow, red};

    class StreetSection
    {
        public:
            Color type;
            cv::Vec2f line;
            cv::Vec4f end_points; 
            std::vector<StreetSection *> connects_end_point1;
            std::vector<StreetSection *> connects_end_point2;
            StreetSection() {}
            StreetSection(Color type_, cv::Vec2f line_, cv::Vec4f end_points_) : type(type_), line(line_), end_points(end_points_) {}
    };
}

#endif
