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
#include "street_section.hpp"
#include "geometry.hpp"

using std::vector;
using std::pair;
using cv::Vec2f;
using cv::Vec4f;
using cv::Vec4i;
using cv::Mat;
using cv::Point;
using constants::lane_width;
using constants::max_theta_diff;
using constants::img_y_horizon;
using street_lines::StreetSection;
using street_lines::Color;
using street_lines::segmentRTToXY;
using street_lines::segmentXYToRT;
using street_lines::distXYPoints;
using street_lines::distRTSegments;
using street_lines::undistortLine;
using street_lines::segmentHalfPoint;
using street_lines::groupCollinearLines;

Vision::Vision() : cam(), img(), aruco_dict(cv::aruco::getPredefinedDictionary(cv::aruco::DICT_4X4_250))
{
    this->cam.set(cv::CAP_PROP_FORMAT, CV_8UC3);
    this->cam.setRotation(3);
    this->cam.set(cv::CAP_PROP_FRAME_WIDTH, constants::img_width);
    this->cam.set(cv::CAP_PROP_FRAME_HEIGHT, constants::img_height);
    auto success = this->cam.open();
    assert(success);
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
    Mat blue_tape_mask;
    this->getBlueTapeMask(blue_tape_mask);
    Mat green_tape_mask;
    this->getGreenTapeMask(green_tape_mask);
    Mat yellow_tape_mask;
    this->getYellowTapeMask(yellow_tape_mask);

    // Finds the lines passing through those masks
    vector<Vec4i> lines;
    vector<Color> line_colors;
    
    auto lines_aux = street_lines::getStreetLines(blue_tape_mask);
    lines.insert(lines.end(), lines_aux.begin(), lines_aux.end());
    line_colors.insert(line_colors.end(), lines_aux.size(), Color::blue);
    
    lines_aux = street_lines::getStreetLines(green_tape_mask);
    lines.insert(lines.end(), lines_aux.begin(), lines_aux.end());
    line_colors.insert(line_colors.end(), lines_aux.size(), Color::green);
    
    lines_aux = street_lines::getStreetLines(yellow_tape_mask);
    lines.insert(lines.end(), lines_aux.begin(), lines_aux.end());
    line_colors.insert(line_colors.end(), lines_aux.size(), Color::yellow);
    
    // Undo the projection distortion
    vector<Vec2f> undistort_lines(lines.size());
    vector<Vec4f> undistort_segs(lines.size());
    for (unsigned int i = 0; i < lines.size(); i++)
    {
        std::tie(undistort_lines[i], undistort_segs[i]) = undistortLine(lines[i]);
    }

    // Find all the possible street sections
    vector<StreetSection> possible_sections;
    for (unsigned int i = 0; i < undistort_segs.size(); i++)
    {
        vector<StreetSection> connected_segs;
        vector<StreetSection> opposite_segs;
        vector<StreetSection> imaginable_sections;
        const Vec4f xy_seg = segmentRTToXY(undistort_segs.edge_points);
        // Separate the relevant segments
        for (unsigned int j = i+1; j < undistort_segs.size(); j++)
        {
            float dist = distXYSegments(xy_seg, segmentRTToXY(undistort_segs[j]));
            // If the segments are right next to each other
            if (dist < lane_width/5)
            {
                connected_segs.emplace_back(line_colors[j], undistort_lines[i], undistort_segs[j]);
            }
            // If the segments are parallel, a lane's distance apart, but not collinear
            else if (linesAreParallel(undistort_lines[i], undistort_lines[j], max_theta_diff)
                     && (lane_width*0.7 < dist && dist < lane_width*1.3)
                     && (abs(undistort_lines[i][0] - undistort_lines[j][0]) > lane_width/5))
            {
                opposite_segs.emplace_back(line_colors[j], undistort_lines[i], undistort_segs[j]);
            }
        }
        // If there are no segments opposite to the 'i' segment, try to insert a section parallel to it, closer to the vehicle
        if (opposite_segs.empty())
        {
            const Vec2f translation (lane_width/2 * cos(seg.line[1]+M_PI), lane_width/2 * sin(seg_line[1]+M_PI));
            const Vec4f translated_xy (xy_seg[0]+translation[0], xy_seg[1]+translation[1],
                                           xy_seg[2]+translation[0], xy_seg[3]+translation[1]);
            const Vec4f translated = segmentXYToRt(translated_xy);
            const Vec2f mid_pt_transl = segmentHalfPoint(translated);
            const Vec4f transl_half1 (translated[0], translated[1], mid_pt_transl[0], mid_pt_transl[1]);
            const Vec4f transl_half2 (mid_pt_transl[0], mid_pt_transl[1], translated[2], translated[3]);
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
                float dist11 = distXYPoints(Point(xy_seg[0], xy_seg[1]), Point(seg.end_points[0], seg.end_point[1]));
                float dist12 = distXYPoints(Point(xy_seg[0], xy_seg[1]), Point(seg.end_points[2], seg.end_point[3]));
                if (dist11 < dist12)
                {
                    const Vec2f pt1 = segmentHalfPoint(Vec4f(undistort_segs[i][0], undistort_segs[i][1],
                                                                     seg.seg[0], seg.seg[1]));
                    const Vec2f pt2 = segmentHalfPoint(Vec4f(undistort_segs[i][2], undistort_segs[i][3],
                                                                     seg.seg[2], seg.seg[3]));
                }
                else
                {
                    const Vec2f pt1 = segmentHalfPoint(Vec4f(undistort_segs[i][0], undistort_segs[i][1],
                                                             seg.seg[2], seg.seg[2]));
                    const Vec2f pt2 = segmentHalfPoint(Vec4f(undistort_segs[i][2], undistort_segs[i][3],
                                                             seg.seg[0], seg.seg[1]));
                }
                const Vec2f mid_pt = segmentHalfPoint(Vec4f(pt1[0], pt1[1], pt2[0], pt2[1]));
                imaginable_sections.emplace_back(Color::blue,
                                                 Vec2f((undistort_lines[i][0]+seg.seg[0])/2,
                                                       (undistort_lines[i][1]+seg.seg[1])/2),
                                                 Vec4f(pt1[0], pt1[1], mid_pt[0], mid_pt[1]));
                imaginable_sections.emplace_back(Color::blue,
                                                 Vec2f((undistort_lines[i][0]+seg.seg[0])/2,
                                                       (undistort_lines[i][1]+seg.seg[1])/2),
                                                 Vec4f(mid_pt[0], mid_pt[1], pt2[0], pt2[1]));
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
            const Vec2f translation (lane_width/2 * cos(seg.line[1]), lane_width/2 * sin(seg.line[1]));
            const Vec2f mid_pt = segmentHalfPoint(undistort_segs[i]);
            const Vec2f pt1 (mid_pt[0]*cos(mid_pt[1]) + translation[0],
                                 mid_pt[0]*sin(mid_pt[1]) + translation[1]);
            const Vec2f pt2 (mid_pt[0]*cos(mid_pt[1]) - translation[0],
                                 mid_pt[0]*sin(mid_pt[1]) - translation[1]);
            const Vec4f transl_half1 (pt1[0], pt1[1], mid_pt[0], mid_pt[1]);
            const Vec4f transl_half2 (mid_pt[0], mid_pt[1], pt2[0], pt2[1]);
            possible_sections.emplace_back(Color::green,
                                           xySegmentToLine(segmentRTToXY(transl_half1)),
                                           transl_half1);
            possible_sections.emplace_back(Color::green,
                                           xySegmentToLine(segmentRTToXY(transl_half2)),
                                           transl_half2);
        }
        // If the segment is yellow and it is perpendicular to the vehicle, add a section crossing it
        else if (lines_color[i] == Color::yellow
                 && linesAreParallel(undistort_lines[1], Vec2f(0, M_PI/2), 3*max_theta_diff))
        {
            const Vec2f translation (lane_width/2 * cos(seg.line[1]), lane_width/2 * sin(seg.line[1]));
            const Vec2f mid_pt = segmentHalfPoint(undistort_segs[i]);
            const Vec2f pt1 (mid_pt[0]*cos(mid_pt[1]) + translation[0],
                                 mid_pt[0]*sin(mid_pt[1]) + translation[1]);
            const Vec2f pt2 (mid_pt[0]*cos(mid_pt[1]) - translation[0],
                                 mid_pt[0]*sin(mid_pt[1]) - translation[1]);
            const Vec4f transl_half1 (pt1[0], pt1[1], mid_pt[0], mid_pt[1]);
            const Vec4f transl_half2 (mid_pt[0], mid_pt[1], pt2[0], pt2[1]);
            possible_sections.emplace_back(Color::yellow,
                                           xySegmentToLine(segmentRTToXY(transl_half1)),
                                           transl_half1);
            possible_sections.emplace_back(Color::yellow,
                                           xySegmentToLine(segmentRTToXY(transl_half2)),
                                           transl_half2);
        }
    }

    // Transform the groups of maybe overlapping collinear sections into a single section
    vector<StreetSection> long_sections;
    vector<Vec2f> possible_lines(possible_sections.size());
    for (unsigned int i = 0; i < possible_sections.size(); i++)
        possible_lines[i] = possible_sections[i].line;
    const auto angle_groups = groupCollinearLines(possible_lines);
    for (auto group: angle_groups)
    {
        // Choose the axis used to order the points on the line
        int used_axis;
        const float group_angle = possible_sections[group[0]].line[1];
        if ((group_angle < M_PI/4) || ((M_PI - M_PI/4) < group_angle < (M_PI - M_PI/4)) || (group_angle > (2*M_PI - M_PI/4)))
            used_axis = 1;
        else
            used_axis = 0;
        Vec4f result_seg = segmentRTToXY(possible_sections[group[0]].seg);
        if (result_seg[used_axis] > result_seg[used_axis+2])
        {
            result_seg = Vec4f(result_seg[2], result_seg[3], result_seg[0], result_seg[1]);
        }
        // Find the min and max points
        for (auto i: group)
        {
            const Vec4f xy_seg = segmentRTToXY(possible_sections[group[i]].seg);
            if (xy_seg[used_axis] < result_seg[used_axis])
            {
                result_seg[0] = xy_seg[0];
                result_seg[1] = xy_seg[1];
            }
            else if (xy_seg[used_axis+2] < result_seg[used_axis])
            {
                result_seg[0] = xy_seg[2];
                result_seg[1] = xy_seg[3];
            }
            if (xy_seg[used_axis] > result_seg[used_axis+2])
            {
                result_seg[2] = xy_seg[0];
                result_seg[3] = xy_seg[1];
            }
            else if (xy_seg[used_axis+2] > result_seg[used_axis+2])
            {
                result_seg[2] = xy_seg[2];
                result_seg[3] = xy_seg[3];
            }
        }
        long_sections.emplace_back(Color::none, xySegmentToLine(result_seg), result_seg);
    }

    // Break the sections where they intersect
    vector<StreetSection> final_sections;
    for (auto i = long_sections.begin(); i != long_sections.end(); i++)
    {
        // Find the cut points
        vector<Vec2f> cut_pts;
        cut_pts.emplace_back(long_sections[i].seg[0], long_sections[i].seg[1]);
        cut_pts.emplace_back(long_sections[i].seg[2], long_sections[i].seg[3]);
        for (auto j = long_sections.begin(); j != long_sections.end(); j++)
        {
            if (i == j)
                continue;
            // If the long sections are connected
            if (distRTSegments(long_sections[i].seg, long_sections[j].seg) < lane_width/4)
            {
                // Break the i section at the projected intersection point
                cut_pts.push_back(linesIntersection(long_sections[i].line, long_sections[j].line));
            }
        }
        // Order the cutpoints
        orderCollinearPoints(cut_pts, long_sections[i].line[1]);
        // Remove points that are too close
        vector<int> pts_to_remove;
        for (auto i = cut_pts.begin()+1; i != cut_pts.end(); i++)
        {
            if (distXYPoints(cut_point[i-1], cut_pts[1]) < lane_width/4)
            {
                pts_to_remove.push_back(i-1);
            }
        }
        vector<Vec2f> cut_pts_filtered;
        for (auto i = cut_pts.begin(); i != cut_pts.end(); i++)
        {
            if (std::find(pts_to_remove.begin(), pts_to_remove.end(), i) == pts_to_remove.end())
            {
                cut_pts_filtered.push_back(cut_pts[i]);
            }
        }
        // Create sections from the cutpoints
        for (auto i = cut_pts_filtered.begin()+1; i != cut_pts_filtered.end(); i++)
        {
            Vec4f seg (cut_pts_filtered[i-1][0], cut_pts_filtered[i-1][1],
                           cut_pts_filtered[i][0], cut_pts_filtered[i][1]);
            final_sections.emplace_back(Color::none, xySegmentToLine(seg), seg); 
        }
    }
    
    // Link the sections
    for (auto i = final_sections.begin(); i != final_sections.end(); i++)
    {
        for (auto j = i+1; j != final_sections.end(); j++)
        {
            if (distXYPoints(Point(final_sections[i].seg[0], final_sections[i].seg[1]),
                             Point(final_sections[j].seg[0], final_sections[j].seg[1])) < lane_width/4)
            {
                final_sections[i].connects_end_point1.push_back(&(final_sections[j]));
                final_sections[j].connects_end_point1.push_back(&(final_sections[i]));
            }
            else if (distXYPoints(Point(final_sections[i].seg[0], final_sections[i].seg[1]),
                                  Point(final_sections[j].seg[2], final_sections[j].seg[3])) < lane_width/4)
            {
                final_sections[i].connects_end_point1.push_back(&(final_sections[j]));
                final_sections[j].connects_end_point2.push_back(&(final_sections[i]));
            }
            else if (distXYPoints(Point(final_sections[i].seg[2], final_sections[i].seg[3]),
                                Point(final_sections[j].seg[0], final_sections[j].seg[1])) < lane_width/4)
            {
                final_sections[i].connects_end_point2.push_back(&(final_sections[j]));
                final_sections[j].connects_end_point1.push_back(&(final_sections[i]));
            }
            else if (distXYPoints(Point(final_sections[i].seg[2], final_sections[i].seg[3]),
                                  Point(final_sections[j].seg[2], final_sections[j].seg[3])) < lane_width/4)
            {
                final_sections[i].connects_end_point2.push_back(&(final_sections[j]));
                final_sections[j].connects_end_point2.push_back(&(final_sections[i]));
            }
        }
    }
    
    return final_sections;
}

void Vision::getColorMask(Mat& dst, int const h_min, int const h_max, int const l_min, int const l_max, int const s_min, int const s_max)
{
    cv::inRange(img, cv::Scalar(h_min, l_min, s_min), cv::Scalar(h_max, l_max, s_max), dst);
    Mat mask = Mat::zeros(img.size(), CV_8U);
    // TODO: Cortar a imagem em ~1m de distancia
    mask(cv::Rect(0, img_y_horizon, img.width, img.height));
    cv::bitwise_and(img, mask, img) = 255;
}

void Vision::getRedTapeMask(Mat& dst)
{
    this->getColorMask(dst, 160, 180, 100, 240, 40, 140);
}

void Vision::getBlueTapeMask(Mat& dst)
{
    this->getColorMask(dst, 100, 120, 70, 200, 25, 170);
}

void Vision::getGreenTapeMask(Mat& dst)
{
    this->getColorMask(dst, 60, 90, 70, 150, 30, 120);
}

void Vision::getYellowTapeMask(Mat& dst)
{
    this->getColorMask(dst, 20, 40, 130, 190, 30, 170);
}

void Vision::getWhiteTapeMask(Mat& dst)
{
    this->getColorMask(dst, 20, 110, 120, 255, 0, 70);
}

void Vision::getGroundMask(Mat& dst)
{
    this->getColorMask(dst, 0, 180, 50, 120, 0, 30);
}

bool Vision::isTrafficLightRed()
{
    Mat red_mask;
    getRedTapeMask(red_mask);
    vector<vector<Point>> contours;
    vector<Vec4i> hierarchy;
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
    vector<vector<Point2f>> corners;
    cv::aruco::detectMarkers(this->img, this->aruco_dict, corners, ids);
    return pair(ids, corners);
}
