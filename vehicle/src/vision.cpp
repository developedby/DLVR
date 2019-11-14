#include "vision.hpp"
#include <cassert>
#include <limits>
#include <algorithm>
#include <tuple>
#include <vector>
#include <iostream>
#include <opencv2/core.hpp>
#include <opencv2/imgproc.hpp>
#include <opencv2/aruco.hpp>
#include <opencv2/videoio.hpp>
#include "reduce_lines.hpp"
#include "constants.hpp"
#include "street_lines.hpp"
#include "street_section.hpp"
#include "geometry.hpp"

using std::vector;
using std::pair;
using std::cout;
using std::endl;
using cv::Vec2f;
using cv::Vec4f;
using cv::Vec4i;
using cv::Mat;
using cv::Point2f;
using constants::lane_width;
using constants::max_theta_diff;
using constants::img_y_horizon;
using street_lines::StreetSection;
using street_lines::Color;
using street_lines::distXYPoints;
using street_lines::distXYSegments;
using street_lines::xySegmentHalfPoint;
using street_lines::xySegmentToLine;

Vision::Vision() : cam(), aruco_dict(cv::aruco::getPredefinedDictionary(cv::aruco::DICT_4X4_250)), img()
{
    this->cam.set(cv::CAP_PROP_FORMAT, CV_8UC3);
    //this->cam.setRotation(3);
    this->cam.set(cv::CAP_PROP_FRAME_WIDTH, constants::img_width);
    this->cam.set(cv::CAP_PROP_FRAME_HEIGHT, constants::img_height);
    this->cam.setAWB(10);
    auto success = this->cam.open();
    assert(success);
    for (int i = 0; i < 5; i++)
    {
        this->cam.grab();
    }
    this->top_cam.open(0);
    //this->top_cam.set(cv::CAP_PROP_FRAME_WIDTH, constants::img_width);
    //this->top_cam.set(cv::CAP_PROP_FRAME_HEIGHT, constants::img_height);
    if(!top_cam.isOpened())
    {
        std::cout <<  "nao funciona" <<std::endl;
    }
    
}

void Vision::getCamImg()
{
    this->cam.grab();
    this->cam.retrieve(this->img);
    cv::blur(this->img, this->img, cv::Size(5, 5));
    cv::cvtColor(this->img, this->img, cv::COLOR_BGR2HLS);
}

void Vision::getTopCamImg()
{
    this->top_cam.read(this->top_img);
}

// Looks at the image and finds the streets
// Returns a graph of the StreetSections identified
vector<StreetSection> Vision::findStreets()
{
    // TODO: Cut the mask closer to the vehicle (~1 meter maybe), so that we don't waste time with far away things
    
    // Finds the binary mask for each marker type
    Mat blue_tape_mask;
    this->getBlueTapeMask(blue_tape_mask);
    //cv::imwrite("teste_linhas_blue.jpg", blue_tape_mask);
    Mat green_tape_mask;
    this->getGreenTapeMask(green_tape_mask);
    //cv::imwrite("teste_linhas_green.jpg", green_tape_mask);
    Mat yellow_tape_mask;
    this->getYellowTapeMask(yellow_tape_mask);
    //cv::imwrite("teste_linhas_yellow.jpg", yellow_tape_mask);

    //cout << "Calculou as mascaras de cor." << endl;

    // Finds the lines passing through those masks
    vector<Vec4i> lines;
    vector<Color> line_colors;
    
    auto lines_aux = street_lines::getStreetLines(green_tape_mask);
    ////cout << "Achou linhas verdes." << endl;
    lines.insert(lines.end(), lines_aux.begin(), lines_aux.end());
    line_colors.insert(line_colors.end(), lines_aux.size(), Color::green);
    
    lines_aux = street_lines::getStreetLines(yellow_tape_mask);
    ////cout << "Achou linhas amarelas." << endl;
    lines.insert(lines.end(), lines_aux.begin(), lines_aux.end());
    line_colors.insert(line_colors.end(), lines_aux.size(), Color::yellow);
    
    lines_aux = street_lines::getStreetLines(blue_tape_mask);
    //cout << "Achou linhas azuis." << endl;
    lines.insert(lines.end(), lines_aux.begin(), lines_aux.end());
    //////cout << "insert lines." << endl;
    line_colors.insert(line_colors.end(), lines_aux.size(), Color::blue);
    ////cout << "insert line_colors." << endl;
    
    Mat img_lines = street_lines::drawSegments(lines, this->img);
    cv::imwrite("teste_linhas_linhas.jpg", img_lines);
    
    //cout << "Calculou as linhas. Encontradas: " << lines.size() << endl;
    for (unsigned int i = 0; i < lines.size(); i++)
    {
        //cout << lines[i] << ' ' << int(line_colors[i]) << endl;
    }
    
    // Undo the projection distortion
    vector<Vec2f> undistort_lines(lines.size());
    vector<Vec4f> undistort_segs(lines.size());
    for (unsigned int i = 0; i < lines.size(); i++)
    {
        std::tie(undistort_lines[i], undistort_segs[i]) = street_lines::undistortLine(lines[i]);
    }

    //cout << "Reverteu a distorcao de perspectiva" << endl;
    for (unsigned int i = 0; i < undistort_lines.size(); i++)
    {
        //cout << undistort_lines[i] << ' ' << undistort_segs[i] << endl;
    }

    // Find all the possible street sections
    vector<StreetSection> possible_sections;
    for (unsigned int i = 0; i < undistort_segs.size(); i++)
    {
        vector<StreetSection> connected_segs;
        vector<StreetSection> opposite_segs;
        vector<StreetSection> imaginable_sections;
        // Separate the relevant segments
        for (unsigned int j = 0; j < undistort_segs.size(); j++)
        {
            if (i == j)
                continue;
            const float dist = distXYSegments(undistort_segs[i], undistort_segs[j]);
            // If the segments are right next to each other
            if (dist < lane_width/4)
            {
                connected_segs.emplace_back(line_colors[j], undistort_lines[i], undistort_segs[j]);
            }
            // If the segments are parallel, a lane's distance apart, but not collinear
            else if (j > i
                     && street_lines::linesAreParallel(undistort_lines[i], undistort_lines[j], max_theta_diff)
                     && (lane_width*0.7 < dist && dist < lane_width*1.3)
                     && (abs(undistort_lines[i][0] - undistort_lines[j][0]) > lane_width/4))
            {
                opposite_segs.emplace_back(line_colors[j], undistort_lines[i], undistort_segs[j]);
            }
        }
        
        // If there are no segments opposite to the 'i' segment, try to insert a section parallel to it, closer to the vehicle
        if (opposite_segs.empty())
        {
            const Vec2f translation(lane_width/2 * cos(undistort_lines[i][1]+M_PI),
                                    lane_width/2 * sin(undistort_lines[i][1]+M_PI));
            const Vec4f translated(undistort_segs[i][0]+translation[0], undistort_segs[i][1]+translation[1],
                                   undistort_segs[i][2]+translation[0], undistort_segs[i][3]+translation[1]);
            const Vec2f mid_pt_transl = xySegmentHalfPoint(translated);
            const Vec4f transl_half1(translated[0], translated[1], mid_pt_transl[0], mid_pt_transl[1]);
            const Vec4f transl_half2(mid_pt_transl[0], mid_pt_transl[1], translated[2], translated[3]);
            imaginable_sections.emplace_back(Color::blue, xySegmentToLine(transl_half1), transl_half1);
            imaginable_sections.emplace_back(Color::blue, xySegmentToLine(transl_half2), transl_half2);

        }
        // If there are opposite segments, try to create sections between them
        else
        {
            for (auto seg: opposite_segs)
            {
                float dist11 = distXYPoints(Point2f(undistort_segs[i][0], undistort_segs[i][1]),
                                            Point2f(seg.end_points[0], seg.end_points[1]));
                float dist12 = distXYPoints(Point2f(undistort_segs[i][0], undistort_segs[i][1]),
                                            Point2f(seg.end_points[2], seg.end_points[3]));
                Vec2f pt1;
                Vec2f pt2;
                if (dist11 < dist12)
                {
                    pt1 = xySegmentHalfPoint(Vec4f(undistort_segs[i][0], undistort_segs[i][1],
                                                   seg.end_points[0], seg.end_points[1]));
                    pt2 = xySegmentHalfPoint(Vec4f(undistort_segs[i][2], undistort_segs[i][3],
                                                   seg.end_points[2], seg.end_points[3]));
                }
                else
                {
                    pt1 = xySegmentHalfPoint(Vec4f(undistort_segs[i][0], undistort_segs[i][1],
                                                   seg.end_points[2], seg.end_points[2]));
                    pt2 = xySegmentHalfPoint(Vec4f(undistort_segs[i][2], undistort_segs[i][3],
                                                   seg.end_points[0], seg.end_points[1]));
                }
                const Vec2f mid_pt = xySegmentHalfPoint(Vec4f(pt1[0], pt1[1], pt2[0], pt2[1]));
                const Vec4f new_seg1(pt1[0], pt1[1], mid_pt[0], mid_pt[1]);
                const Vec4f new_seg2(mid_pt[0], mid_pt[1], pt2[0], pt2[1]);
                imaginable_sections.emplace_back(Color::blue, xySegmentToLine(new_seg2), new_seg2);
                imaginable_sections.emplace_back(Color::blue, xySegmentToLine(new_seg1), new_seg1);
            }
        }
        // Check if the proposed sections are not crossing blue or yellow
        for (auto section: imaginable_sections)
        {
            bool section_possible = true;
            for (auto seg: connected_segs)
            {
                if (seg.type != Color::green)
                {
                    const float dist = distXYSegments(section.end_points, seg.end_points);
                    //cout << "Encontrou um segmento que pode dar problema:" << endl;
                    //cout << "  secao: " << section.end_points << ' ' << int(section.type) << endl;
                    //cout << "  seg: " << seg.end_points << ' ' << int(seg.type) << endl;
                    //cout << "  dist: " << dist << endl << endl;
                    if (dist < lane_width/3)
                    {
                        section_possible = false;
                        break;
                    }
                }
            }
            if (section_possible)
                possible_sections.push_back(section);
        }  

        // If the 'i' segment is green, add a perpendicular section
        if (line_colors[i] == Color::green)
        {
            const Vec2f translation(lane_width/2 * cos(undistort_lines[i][1]),
                                    lane_width/2 * sin(undistort_lines[i][1]));
            const Vec2f mid_pt = xySegmentHalfPoint(undistort_segs[i]);
            const Vec2f pt1 (mid_pt[0] + translation[0],
                             mid_pt[1] + translation[1]);
            const Vec2f pt2 (mid_pt[0] - translation[0],
                             mid_pt[1] - translation[1]);
            const Vec4f new_seg1(pt1[0], pt1[1], mid_pt[0], mid_pt[1]);
            const Vec4f new_seg2(mid_pt[0], mid_pt[1], pt2[0], pt2[1]);
            possible_sections.emplace_back(Color::green, xySegmentToLine(new_seg1), new_seg1);
            possible_sections.emplace_back(Color::green, xySegmentToLine(new_seg2), new_seg2);
        }
        // If the segment is yellow and it is perpendicular to the vehicle, add a section crossing it
        else if (line_colors[i] == Color::yellow
                 && street_lines::linesAreParallel(undistort_lines[i], Vec2f(0, M_PI/2), 3*max_theta_diff))
        {
            const Vec2f translation (lane_width/2 * cos(undistort_lines[i][1]),
                                     lane_width/2 * sin(undistort_lines[i][1]));
            const Vec2f mid_pt = xySegmentHalfPoint(undistort_segs[i]);
            const Vec2f pt1 (mid_pt[0] + translation[0],
                             mid_pt[1] + translation[1]);
            const Vec2f pt2 (mid_pt[0] - translation[0],
                             mid_pt[1] - translation[1]);
            const Vec4f new_seg1(pt1[0], pt1[1], mid_pt[0], mid_pt[1]);
            const Vec4f new_seg2(mid_pt[0], mid_pt[1], pt2[0], pt2[1]);
            possible_sections.emplace_back(Color::yellow, xySegmentToLine(new_seg1), new_seg1);
            possible_sections.emplace_back(Color::yellow, xySegmentToLine(new_seg2), new_seg2);
        }
    }

    //cout << "Calculou todas as mini seções. Encontradas: " << possible_sections.size() << endl;
    for (unsigned int i = 0; i < possible_sections.size(); i++)
    {
        //cout << possible_sections[i].line << ' ' << possible_sections[i].end_points << ' ' << int(possible_sections[i].type) << endl;
    }

    // Transforms overlapping sections into a single long section
    vector<StreetSection> long_sections;
    vector<Vec2f> possible_lines(possible_sections.size());
    for (unsigned int i = 0; i < possible_sections.size(); i++)
        possible_lines[i] = possible_sections[i].line;
    // For each group of sections that have the same angle
    const auto angle_groups = street_lines::groupCollinearLines(possible_lines, max_theta_diff, lane_width/3);
    ////cout << "Agrupou as linhas colineares" << endl;
    for (auto group: angle_groups)
    {
        // Choose the axis used to order the points on the line
        int used_axis;
        const float group_angle = possible_sections[group[0]].line[1];
        if ((group_angle < M_PI/4)
            || (((M_PI - M_PI/4) < group_angle) && (group_angle < (M_PI + M_PI/4)))
            || (group_angle > (2*M_PI - M_PI/4)))
        {
            used_axis = 1;
        }
        else
            used_axis = 0;
        //cout << "Escolheu eixo pra juntar linhas " << used_axis <<endl;
        
        // Copies the section that belong to this group and order them by the chosen axis
        vector<StreetSection> collinear_sections;
        for (auto i: group)
        {
            collinear_sections.push_back(possible_sections[i]);
        }
        orderCollinearSections(collinear_sections, used_axis);
        for(auto section : collinear_sections)
        {
            //cout << section.end_points << endl;
        }        
        //cout << "Copiei e ordenei colinear sections" <<endl;
        
        // Finds and joins the overlaping sections, creating different sections when there is a gap
        vector<Vec4f> result_segs;
        int secs_in_result = 0;
        Vec4f seg_in_construction;
        for (auto section: collinear_sections)
        {
            if (secs_in_result == 0)
            {
                seg_in_construction = section.end_points;
                secs_in_result++;
            }
            else
            {
                if (distXYSegments(seg_in_construction, section.end_points) > lane_width/3)
                {
                    //cout << "coloquei em result o " << seg_in_construction <<endl;
                    result_segs.push_back(seg_in_construction);
                    seg_in_construction = section.end_points;
                    secs_in_result = 1;
                }
                else
                {
                    if (seg_in_construction[used_axis+2] < section.end_points[used_axis+2]) 
                    {
                        seg_in_construction[2] = section.end_points[2];
                        seg_in_construction[3] = section.end_points[3];
                    }
                    secs_in_result++;
                }
            }
        }
        if (secs_in_result != 0)
        {
            //cout << "coloquei por último em result o " << seg_in_construction <<endl;
            result_segs.push_back(seg_in_construction);
        }
        ////cout << "Montou o segmento longo" << endl;
        for (auto result_seg: result_segs)
            long_sections.emplace_back(Color::none, xySegmentToLine(result_seg), result_seg);
    }

    //cout << "Calculou todas as seções longas. Encontradas: " << long_sections.size() << endl;
    for (unsigned int i = 0; i < long_sections.size(); i++)
    {
        //cout << long_sections[i].line << ' ' << long_sections[i].end_points << ' ' << int(long_sections[i].type) << endl;
    }

    // Break the sections where they intersect
    vector<StreetSection> final_sections;
    for (unsigned int i = 0; i < long_sections.size(); i++)
    {
        // Find the cut points
        vector<Vec2f> cut_pts;
        cut_pts.emplace_back(long_sections[i].end_points[0], long_sections[i].end_points[1]);
        cut_pts.emplace_back(long_sections[i].end_points[2], long_sections[i].end_points[3]);
        for (unsigned int j = 0; j < long_sections.size(); j++)
        {
            if (i == j)
                continue;
            // If the long sections are connected
            if (distXYSegments(long_sections[i].end_points, long_sections[j].end_points) < lane_width/4)
            {
                // Break the i section at the projected intersection point
                cut_pts.push_back(street_lines::linesIntersection(long_sections[i].line, long_sections[j].line));
            }
        }
        ////cout << "cut_pts i=" << i << ":" << endl;
        //for (auto pt: cut_pts)
        //    //cout << '\t' << pt << endl;

        // Order the cutpoints
        street_lines::orderCollinearPoints(cut_pts, long_sections[i].line[1]);
        ////cout << "cut_pts_ordered i=" << i << ":" << endl;
        //for (auto pt: cut_pts)
        //    //cout << '\t' << pt << endl;
        
        // Remove points that are too close
        vector<int> pts_to_remove;
        for (unsigned int j = 1; j < cut_pts.size(); j++)
        {
            const auto dist = distXYPoints(Point2f(cut_pts[j-1][0], cut_pts[j-1][1]),
                                           Point2f(cut_pts[j][0], cut_pts[j][1]));
            ////cout << "dist i=" << i <<" j=" << j << " - " << dist << endl;
            if ((dist < lane_width/4) && (pts_to_remove.size() < (cut_pts.size()-2)))
            {
                if (j < cut_pts.size()/2)
                    pts_to_remove.push_back(j);
                else
                    pts_to_remove.push_back(j-1);
            }
        }
        ////cout << "pts_to_remove i=" << i << ":" << endl;
        //for (auto pt: pts_to_remove)
         //   //cout << '\t' << pt << endl;
        
        vector<Vec2f> cut_pts_filtered;
        for (unsigned int j = 0; j < cut_pts.size(); j++)
        {
            if (std::find(pts_to_remove.begin(), pts_to_remove.end(), j) == pts_to_remove.end())
            {
                cut_pts_filtered.push_back(cut_pts[j]);
            }
        }
        ////cout << "cut_pts_filtered i=" << i << ":" << endl;
        //for (auto pt: cut_pts_filtered)
        //    //cout << '\t' << pt << endl;

        // Create sections from the cutpoints
        for (unsigned int j = 1; j < cut_pts_filtered.size(); j++)
        {
            Vec4f seg (cut_pts_filtered[j-1][0], cut_pts_filtered[j-1][1],
                       cut_pts_filtered[j][0], cut_pts_filtered[j][1]);
            final_sections.emplace_back(Color::none, xySegmentToLine(seg), seg); 
        }
    }
    
    //cout << "Calculou as seções finais. Encontradas: " << final_sections.size() << endl;
    for (unsigned int i = 0; i < final_sections.size(); i++)
    {
        //cout << final_sections[i].line << ' ' << final_sections[i].end_points << ' ' << int(final_sections[i].type) << endl;
    }
    
    // Link the sections
    for (unsigned int i = 0; i < final_sections.size(); i++)
    {
        for (unsigned int j = i+1; j < final_sections.size(); j++)
        {
            if (distXYPoints(Point2f(final_sections[i].end_points[0], final_sections[i].end_points[1]),
                             Point2f(final_sections[j].end_points[0], final_sections[j].end_points[1])) < lane_width/4)
            {
                final_sections[i].connects_end_point1.push_back(&(final_sections[j]));
                final_sections[j].connects_end_point1.push_back(&(final_sections[i]));
            }
            else if (distXYPoints(Point2f(final_sections[i].end_points[0], final_sections[i].end_points[1]),
                                  Point2f(final_sections[j].end_points[2], final_sections[j].end_points[3])) < lane_width/4)
            {
                final_sections[i].connects_end_point1.push_back(&(final_sections[j]));
                final_sections[j].connects_end_point2.push_back(&(final_sections[i]));
            }
            else if (distXYPoints(Point2f(final_sections[i].end_points[2], final_sections[i].end_points[3]),
                                  Point2f(final_sections[j].end_points[0], final_sections[j].end_points[1])) < lane_width/4)
            {
                final_sections[i].connects_end_point2.push_back(&(final_sections[j]));
                final_sections[j].connects_end_point1.push_back(&(final_sections[i]));
            }
            else if (distXYPoints(Point2f(final_sections[i].end_points[2], final_sections[i].end_points[3]),
                                  Point2f(final_sections[j].end_points[2], final_sections[j].end_points[3])) < lane_width/4)
            {
                final_sections[i].connects_end_point2.push_back(&(final_sections[j]));
                final_sections[j].connects_end_point2.push_back(&(final_sections[i]));
            }
        }
    }
    //cout << "Juntou as seções."<< endl;
    return final_sections;
}

void Vision::getColorMask(Mat& dst, const cv::Scalar min, const cv::Scalar max)
{
    cv::inRange(this->img, min, max, dst);
    const Mat krn_open = cv::getStructuringElement(cv::MORPH_RECT, cv::Size(5, 5));
    cv::morphologyEx(dst, dst, cv::MORPH_OPEN, krn_open);
    const Mat krn_close = cv::getStructuringElement(cv::MORPH_RECT, cv::Size(5, 5));
    cv::morphologyEx(dst, dst, cv::MORPH_CLOSE, krn_close);
}

void Vision::getTapeMask(Mat& dst, const cv::Scalar min, const cv::Scalar max)
{
    Mat raw_mask;
    this->getColorMask(raw_mask, min, max);
    vector<vector<cv::Point>> cnts;
    vector<Vec4i> hierarchy;
    cv::findContours(raw_mask, cnts, hierarchy, cv::RETR_TREE, cv::CHAIN_APPROX_SIMPLE);
    vector<vector<cv::Point>> filtered_cnts;
    for (auto contour: cnts)
    {
        const auto area = cv::contourArea(contour);
        ////cout << "contorno area=" << area << endl;
        if (area > 2000)
        {
            filtered_cnts.push_back(contour);
        }
    }
    dst = cv::Mat::zeros(raw_mask.size(), CV_8U);
    cv::drawContours(dst, filtered_cnts, -1, 255, cv::FILLED);
}

void Vision::getRedTapeMask(Mat& dst)
{
    this->getTapeMask(dst, cv::Scalar(160, 100, 40), cv::Scalar(180, 240, 140));
}

void Vision::getBlueTapeMask(Mat& dst)
{
    this->getTapeMask(dst, cv::Scalar(85, 200, 40), cv::Scalar(105, 250, 255));
}

void Vision::getGreenTapeMask(Mat& dst)
{
    this->getTapeMask(dst, cv::Scalar(40, 130, 40), cv::Scalar(60, 250, 255));
}

void Vision::getYellowTapeMask(Mat& dst)
{
    this->getTapeMask(dst, cv::Scalar(20, 130, 30), cv::Scalar(40, 190, 170));
}

void Vision::getWhiteTapeMask(Mat& dst)
{
    this->getTapeMask(dst, cv::Scalar(20, 120, 0), cv::Scalar(110, 255, 70));
}

void Vision::getGroundMask(Mat& dst)
{
    this->getColorMask(dst, cv::Scalar(0, 50, 0), cv::Scalar(180, 120, 30));
}

bool Vision::isTrafficLightRed()
{
    Mat red_mask;
    getRedTapeMask(red_mask);
    vector<vector<cv::Point>> contours;
    vector<Vec4i> hierarchy;
    cv::findContours(red_mask, contours, hierarchy, cv::RETR_CCOMP, cv::CHAIN_APPROX_SIMPLE);
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
    vector<vector<cv::Point2f>> corners;
    cv::aruco::detectMarkers(this->top_img, this->aruco_dict, corners, ids);
    return pair(ids, corners);
}
