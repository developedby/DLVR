#include "street_section.hpp"
#include <algorithm>
#include <iostream>

namespace street_lines
{
	// Does a stable in-place sort of collinear street sections
    void orderCollinearSections(std::vector<StreetSection>& sections, const int used_axis)
    {
		for (auto& section: sections)
		{
			if (section.end_points[used_axis] > section.end_points[used_axis+2])
			{
				std::swap(section.end_points[0], section.end_points[2]);
				std::swap(section.end_points[1], section.end_points[3]);
			}
		}
        std::stable_sort(sections.begin(), sections.end(),
                         [used_axis](auto sec1, auto sec2){return sec1.end_points[used_axis] < sec2.end_points[used_axis];});
    }
}
