#ifndef streets_h_
#define streets_h_

#include <string>
#include <vector>
#include <tuple>
#include <utility>
#include <opencv2/core.hpp>

namespace streets
{
	enum class Color {none ,blue, green, yellow, red};

    class StreetSection
    {
        public:
            Color color;
            cv::Vec2f line;
            cv::Vec4f seg; 
            std::vector<StreetSection*> connects_end_pt1;
            std::vector<StreetSection*> connects_end_pt2;
            StreetSection() {}
            StreetSection(Color color_, cv::Vec2f line_, cv::Vec4f seg_) :
                color(color_), line(line_), seg(seg_), connects_end_pt1(), connects_end_pt2() {}
            void print() const;
            std::string as_str() const;
            bool operator==(const StreetSection& other) const;
    };
    
    // Big functions
    std::tuple<std::vector<cv::Vec4i>, std::vector<Color>> findTapeLines(const cv::Mat& img);
    std::vector<StreetSection> undoProjectionDistortion(const std::vector<cv::Vec4i>& img_lines, const std::vector<Color>& lines_colors);
    std::vector<StreetSection> findPossibleStreetSections(const std::vector<StreetSection>& tape_secs);
    std::vector<StreetSection> groupIntoLongSections(const std::vector<StreetSection>& short_secs);
    std::vector<StreetSection> breakIntersectingSections(const std::vector<StreetSection>& long_secs);
    void buildSectionGraph(std::vector<StreetSection>& secs);
    
    // Auxiliary functions
    std::vector<cv::Vec4i> getStreetLines(const cv::Mat& lines_mask);
    std::vector<cv::Vec4i> reduceSegments (const std::vector<cv::Vec4i>& segs, const cv::Mat& label_image, const int n_labels);
    cv::Vec4f imgSegToRealSeg(const cv::Vec4i& line);
    // Grouping functions
    std::vector<std::vector<unsigned int>> groupLinesByDistance(const std::vector<cv::Vec2f>& lines, const float max_rho_diff);
    std::vector<std::vector<unsigned int>> groupCollinearLines(const std::vector<cv::Vec2f>& lines, const float max_theta_diff, const float max_rho_diff);
    std::vector<std::vector<unsigned int>> groupCollinearSections(const std::vector<StreetSection>& secs, const float max_theta_diff, const float max_dist);
    // Ordering functions
    void orderCollinearSections(std::vector<StreetSection>& sections, const int used_axis);
    // Mask Functions
    std::tuple<cv::Mat, cv::Mat, cv::Mat> getTapeMasks(const cv::Mat& img);
    cv::Mat getColorMask(const cv::Mat& img, const cv::Scalar min, const cv::Scalar max);
    cv::Mat getTapeMask(const cv::Mat& img, const cv::Scalar min, const cv::Scalar max);
    cv::Mat getRedTapeMask(const cv::Mat& img);
    cv::Mat getGreenTapeMask(const cv::Mat& img);
    cv::Mat getBlueTapeMask(const cv::Mat& img);
    cv::Mat getYellowTapeMask(const cv::Mat& img);
    cv::Mat getWhiteTapeMask(const cv::Mat& img);
    cv::Mat getGroundMask(const cv::Mat& img);
    // Section creating functions
    void insertParallelSection(const StreetSection& sec, const Color color, std::vector<StreetSection>& sec_vec);
    void insertPerpendicularSection(const StreetSection& sec, const Color color, std::vector<StreetSection>& sec_vec);
    void insertMiddleSection(const StreetSection& sec1, const StreetSection& sec2, const Color color, std::vector<StreetSection>& sec_vec);
    
    // Drawing functions
    cv::Mat drawSegments(const std::vector<cv::Vec4i>& lines, const cv::Mat& img, const bool cvt_color=true);
    void drawLine(const cv::Vec2f& line, const cv::Mat& img);
    void drawLabelImage(const cv::Mat& labelImage, int nLabels);
}

#endif
