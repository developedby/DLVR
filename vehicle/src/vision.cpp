#include "vision.hpp"
#include <cassert>
#include <limits>
#include <algorithm>
#include <tuple>
#include <vector>
#include <opencv2/core.hpp>
#include <opencv2/imgproc.hpp>
#include <opencv2/aruco.hpp>
#include "reduce_lines.hpp"
#include "constants.hpp"
#include "street_lines.hpp"
#include "street_sections.hpp"

using std::vector;
using std::pair
using constants::lane_width;
using constants::max_theta_diff;
using street_lines::StreetSection;
using street_lines::Color;
using street_lines::segmentRTToXY;
using street_lines::segmentXYToRt;
using street_lines::distXYPoints;
using street_lines::distRTSegments;
using street_lines::undistortLine;
using street_lines::segmentHalfPoint;
using street_lines::groupCollinearLines;

Vision::Vision()
{
    this->cam.set(cv::CAP_PROP_FORMAT, CV_8UC3);
    this->cam.setRotation(3);
    this->cam.set(cv::CAP_PROP_FRAME_WIDTH, constants::img_width);
    this->cam.set(cv::CAP_PROP_FRAME_HEIGHT, constants::img_height);
    auto success = this->cam.open();
    assert(success);

    this->aruco_dict = cv::aruco::getPredefinedDictionary(cv::aruco::DICT_4X4_250);
}

void Vision::getCamImg()
{
    this->cam.grab();
    this->cam.retrieve(this->img);
    cv::cvtColor(this->img, this->img, cv::COLOR_BGR2HLS);
}

// Looks at the image and finds the streets
// Returns a graph of the StreetSections identified
vector<StreetSection> Vision::findStreets()
{
    // TODO: Cut the mask closer to the vehicle (~1 meter maybe), so that we don't waste time with far away things
    
    // Finds the binary mask for each marker type
    cv::Mat blue_tape_mask;
    this->getBlueTapeMask(blue_tape_mask);
    cv::Mat green_tape_mask;
    this->getGreenTapeMask(green_tape_mask);
    cv::Mat yellow_tape_mask;
    this->getYellowTapeMask(yellow_tape_mask);

    // Finds the lines passing through those masks
    vector<cv::Vec4i> lines;
    vector<Color> line_colors;
    
    auto lines_aux = street_lines::getStreetLines(blue_tape_mask);
    lines.insert(line.end(), lines_aux.begin(), lines_aux.end());
    line_colors.insert(lines_aux.size(), Color::blue);
    
    lines_aux = street_lines::getStreetLines(green_tape_mask);
    lines.insert(line.end(), lines_aux.begin(), lines_aux.end());
    line_colors.insert(lines_aux.size(), Color::green);
    
    lines_aux = street_lines::getStreetLines(yellow_tape_mask);
    lines.insert(line.end(), lines_aux.begin(), lines_aux.end());
    line_colors.insert(lines_aux.size(), Color::yellow);
    
    // Undo the projection distortion
    vector<cv::Vec2f> undistort_lines(lines.size());
    vector<cv::Vec4f> undistort_segs(lines.size());
    for (int i = lines.begin(); i != lines.end(); i++)
    {
        std::tie(undistort_lines[i], undistort_segs[i]) = undistortLine(lines[i]);
    }

    // Find all the possible street sections
    vector<StreetSection> possible_sections;
    for (auto i = undistort_segs.begin(); i != undistort_segs.end(); i++)
    {
        vector<StreetSection> connected_segs;
        vector<StreetSection> opposite_segs;
        vector<StreetSection> imaginable_sections;
        const cv::Vec4f xy_seg = segmentRTToXY(seg.edge_points);
        // Separate the relevant segments
        for (auto j = i+1; i != undistort_segs.end(); j++)
        {
            float dist = distXYSegments(xy_seg, segmentRTToXY(undistort_segs[j]));
            // If the segments are right next to each other
            if (dist < lane_width/5)
            {
                connected_segs.emplace_back(line_colors[j], undistort_line[i], undistort_seg[j]);
            }
            // If the segments are parallel, a lane's distance apart, but not collinear
            else if (linesAreParallel(undistort_lines[i], undistort_lines[j], max_theta_diff)
                     && (lane_width*0.7 < dist && dist < lane_width*1.3)
                     && (abs(undistort_lines[i][0] - undistort_lines[j][0]) > lane_width/5))
            {
                opposite_segs.emplace_back(line_colors[j], undistort_line[i], undistort_seg[j]);
            }
        }
        // If there are no segments opposite to the 'i' segment, try to insert a section parallel to it, closer to the vehicle
        if (opposite_segs.empty())
        {
            const cv::Vec2f translation (lane_width/2 * cos(seg.line[1]+M_PI), lane_width/2 * sin(seg_line[1]+M_PI));
            const cv::Vec4f translated_xy (xy_seg[0]+translation[0], xy_seg[1]+translation[1],
                                           xy_seg[2]+translation[0], xy_seg[3]+translation[1]);
            const cv::Vec4f translated = segmentXYToRt(translated_xy);
            const cv::Vec2f mid_pt_transl = segmentHalfPoint(translated);
            const cv::Vec4f transl_half1 (translated[0], translated[1], mid_pt_transl[0], mid_pt_transl[1]);
            const cv::Vec4f transl_half2 (mid_pt_transl[0], mid_pt_transl[1], translated[2], translated[3]);
            imaginable_sections.emplace_back(Color::blue,
                                            xySegmentToLine(segmentRTToXY(transl_half1)),
                                            transl_half1);
            imaginable_sections.emplace_back(Color::blue,
                                             xySegmentToLine(segmentRTToXY(transl_half2)),
                                             transl_half2);
        }
        // If there are opposite segments, try to create sections between them
        else
        {
            for (auto seg: opposite_segs)
            {
                float dist11 = distXYPoints(cv::Point(xy_seg[0], xy_seg[1]), cv::Point(seg.end_points[0], seg.end_point[1]));
                float dist12 = distXYPoints(cv::Point(xy_seg[0], xy_seg[1]), cv::Point(seg.end_points[2], seg.end_point[3]));
                if (dist11 < dist12)
                {
                    const cv::Vec2f pt1 = segmentHalfPoint(cv::Vec4f(undistort_segs[i][0], undistort_segs[i][1],
                                                                     seg.seg[0], seg.seg[1]));
                    const cv::Vec2f pt2 = segmentHalfPoint(cv::Vec4f(undistort_segs[i][2], undistort_segs[i][3],
                                                                     seg.seg[2], seg.seg[3]));
                }
                else
                {
                    const cv::Vec2f pt1 = segmentHalfPoint(cv::Vec4f(undistort_segs[i][0], undistort_segs[i][1],
                                                                     seg.seg[2], seg.seg[2]));
                    const cv::Vec2f pt2 = segmentHalfPoint(cv::Vec4f(undistort_segs[i][2], undistort_segs[i][3],
                                                                     seg.seg[0], seg.seg[1]));
                }
                const cv::Vec2f mid_pt = segmentHalfPoint(cv::Vec4f(pt1[0], pt1[1], pt2[0], pt2[1]));
                imaginable_sections.emplace_back(Color::blue,
                                                 cv::Vec2f((undistort_lines[i][0]+seg.seg[0])/2,
                                                           (undistort_lines[i][1]+seg.seg[1])/2),
                                                 cv::Vec4f(pt1[0], pt1[1], mid_pt[0], mid_pt[1]));
                imaginable_sections.emplace_back(Color::blue,
                                                 cv::Vec2f((undistort_lines[i][0]+seg.seg[0])/2,
                                                           (undistort_lines[i][1]+seg.seg[1])/2),
                                                 cv::Vec4f(mid_pt[0], mid_pt[1], pt2[0], pt2[1]));
            }
        }
        // Check if the proposed sections are not crossing blue or yellow
        for (auto section: imaginable_sections)
        {
            bool section_possible = true;
            for (auto seg: connected_segs)
            {
                if ((seg.color != Color::green)
                    && (distRTSegments(section.seg, seg.seg) < lane_width/5))
                {
                    section_possible = false;
                    break;
                }
            }
            if (section_possible)
                possible_sections.push_back(section);
        }   
        // If the 'i' segment is green, add a perpendicular section
        if (lines_color[i] == Color::green)
        {
            const cv::Vec2f translation (lane_width/2 * cos(seg.line[1]), lane_width/2 * sin(seg.line[1]));
            const cv::Vec2f mid_pt = segmentHalfPoint(undistort_segs[i]);
            const cv::Vec2f pt1 (mid_pt[0]*cos(mid_pt[1]) + translation[0],
                                 mid_pt[0]*sin(mid_pt[1]) + translation[1]);
            const cv::Vec2f pt2 (mid_pt[0]*cos(mid_pt[1]) - translation[0],
                                 mid_pt[0]*sin(mid_pt[1]) - translation[1]);
            const cv::Vec4f transl_half1 (pt1[0], pt1[1], mid_pt[0], mid_pt[1]);
            const cv::Vec4f transl_half2 (mid_pt[0], mid_pt[1], pt2[0], pt2[1]);
            possible_sections.emplace_back(Color::green,
                                           xySegmentToLine(segmentRTToXY(transl_half1)),
                                           transl_half1);
            possible_sections.emplace_back(Color::green,
                                           xySegmentToLine(segmentRTToXY(transl_half2)),
                                           transl_half2);
        }
        
        // If the segment is yellow and it is perpendicular to the vehicle, add a section crossing it
        else if (lines_color[i] == Color::yellow
                 && linesAreParallel(undistort_lines[1], cv::Vec2f(0, M_PI/2), 3*max_theta_diff))
        {
            const cv::Vec2f translation (lane_width/2 * cos(seg.line[1]), lane_width/2 * sin(seg.line[1]));
            const cv::Vec2f mid_pt = segmentHalfPoint(undistort_segs[i]);
            const cv::Vec2f pt1 (mid_pt[0]*cos(mid_pt[1]) + translation[0],
                                 mid_pt[0]*sin(mid_pt[1]) + translation[1]);
            const cv::Vec2f pt2 (mid_pt[0]*cos(mid_pt[1]) - translation[0],
                                 mid_pt[0]*sin(mid_pt[1]) - translation[1]);
            const cv::Vec4f transl_half1 (pt1[0], pt1[1], mid_pt[0], mid_pt[1]);
            const cv::Vec4f transl_half2 (mid_pt[0], mid_pt[1], pt2[0], pt2[1]);
            possible_sections.emplace_back(Color::yellow,
                                           xySegmentToLine(segmentRTToXY(transl_half1)),
                                           transl_half1);
            possible_sections.emplace_back(Color::yellow,
                                           xySegmentToLine(segmentRTToXY(transl_half2)),
                                           transl_half2));
        }
    }

    // Transform the groups of maybe overlapping collinear sections into a group of non-overlapping sections
    vector<StreetSection> short_sections;
    vector<cv::Vec2f> possible_lines(possible_sections.size());
    for (auto i = possible_sections.begin(); i != possible_sections.end(); i++)
        possible_lines[i] = possible_sections[i].line;
    const auto angle_groups = groupCollinearLines(possible_lines);
    for (auto group: angle_groups)
    {
        // Todo: Implement. (Find the cut points, the edges and take the average of very close points)
        for ()
    }

}

void Vision::getColorMask(cv::Mat& dst, int const h_min, int const h_max, int const l_min, int const l_max, int const s_min, int const s_max)
{
    cv::inRange(img, cv::Scalar(h_min, l_min, s_min), cv::Scalar(h_max, l_max, s_max), dst);
}

void Vision::getRedTapeMask(cv::Mat& dst)
{
    this->getColorMask(dst, 160, 180, 100, 240, 40, 140);
}

void Vision::getBlueTapeMask(cv::Mat& dst)
{
    this->getColorMask(dst, 100, 120, 70, 200, 25, 170);
}

void Vision::getGreenTapeMask(cv::Mat& dst)
{
    this->getColorMask(dst, 60, 90, 70, 150, 30, 120);
}

void Vision::getYellowTapeMask(cv::Mat& dst)
{
    this->getColorMask(dst, 20, 40, 130, 190, 30, 170);
}

void Vision::getWhiteTapeMask(cv::Mat& dst)
{
    this->getColorMask(dst, 20, 110, 120, 255, 0, 70);
}

void Vision::getGroundMask(cv::Mat& dst)
{
    this->getColorMask(dst, 0, 180, 50, 120, 0, 30);
}

bool Vision::isTrafficLightRed()
{
    cv::Mat red_mask;
    getRedTapeMask(red_mask);
    vector<vector<cv::Point>> contours;
    vector<cv::Vec4i> hierarchy;
    cv::findContours(red_mask, contours, hierarchy, CV_RETR_CCOMP, CV_CHAIN_APPROX_SIMPLE);
    if(hierarchy.size()>0)
    {
        for(int j=0; j>=0; j=hierarchy[j][0])
        {
            double area = cv::contourArea(contours[j]);
            if(area < 20)
            {
                return true;
            }
        }
    }
    return false;
    
}

pair<vector<int>, vector<vector<cv::Point2f>>> Vision::findARMarkers()
{
    vector<int> ids;
    vector<vector<cv::Point2f> corners;
    cv::aruco::detectMarkers(this->img, this->aruco_dict, corners, ids);
    return pair(ids, corners);
}
