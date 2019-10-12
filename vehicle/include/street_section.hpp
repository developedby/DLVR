#ifndef street_section_h_
#define street_section_h_

#include <vector>
#include <utility>

namespace street_lines
{
	enum StreetSectionType
	{
		SECTION_STREET, SECTION_HOUSE
	};

	class StreetSection
	{
		public:
			StreetSectionType type;
			float direction;
			float lane_offset;
			float section_len;
			std::vector<std::pair<StreetSection *,float>> connections;
	};
}

#endif
