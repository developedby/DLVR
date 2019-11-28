#include "streets.hpp"

#include <cmath>
#include <iostream>
#include <limits>
#include <vector>
#include <array>
#include <algorithm>
#include <sstream>
#include <string>

#include <opencv2/core.hpp>
#include <opencv2/imgproc.hpp>
#include <opencv2/highgui.hpp>

#include "geometry.hpp"
#include "constants.hpp"

using geometry::square;
using std::vector;
using std::tuple;
using std::pair;
using cv::Vec4i;
using cv::Vec4f;
using cv::Vec2f;
using cv::Mat;
using cv::Scalar;
using cv::Point2f;

namespace streets
{
    void StreetSection::print() const
    {
        std::cout << this->as_str() << std::endl;
    }
 
    std::string StreetSection::as_str() const
    {
        std::stringstream ss = std::stringstream();
        ss << this->line << ' ' << this->seg << ' ' << int(this->color);
        return ss.str();
    }

    bool StreetSection::operator==(const StreetSection& other) const
    {
        return (this->color == other.color) && (this->seg == other.seg) && (this->line == other.line);
    }

    // Finds the lines that go through the tapes in the image
    tuple<vector<Vec4i>, vector<Color>> findTapeLines(const Mat& img)
    {
        // Finds the binary mask for each marker type
        auto [blue_mask, green_mask, yellow_mask] = getTapeMasks(img);
        if (consts::save_img)
        {
            cv::imwrite("teste_linhas_blue.jpg", blue_mask);
            cv::imwrite("teste_linhas_green.jpg", green_mask);    
            cv::imwrite("teste_linhas_yellow.jpg", yellow_mask);
        }
        //std::cout << "Calculou as mascaras de cor." << std::endl;

        // Finds the lines passing through those masks
        vector<Vec4i> lines;
        vector<Color> line_colors;

        auto lines_aux = getStreetLines(yellow_mask);
        lines.insert(lines.end(), lines_aux.begin(), lines_aux.end());
        line_colors.insert(line_colors.end(), lines_aux.size(), Color::yellow);
        //std::cout << "Achou linhas amarelas." << std::endl;
        
        lines_aux = getStreetLines(green_mask);
        lines.insert(lines.end(), lines_aux.begin(), lines_aux.end());
        line_colors.insert(line_colors.end(), lines_aux.size(), Color::green);
        //std::cout << "Achou linhas verdes." << std::endl;

        lines_aux = getStreetLines(blue_mask);
        lines.insert(lines.end(), lines_aux.begin(), lines_aux.end());
        line_colors.insert(line_colors.end(), lines_aux.size(), Color::blue);
        //std::cout << "Achou linhas azuis." << std::endl;

        if (consts::save_img)
        {
            Mat img_lines = drawSegments(lines, img);
            cv::imwrite("teste_linhas_linhas.jpg", img_lines);
        }        
        return tuple(lines, line_colors);
    }
    
    // Undoes the projection distortion in each image line
    // Converts the undistorted segments to StreetSection
    vector<StreetSection> undoProjectionDistortion(const vector<Vec4i>& img_lines, const vector<Color>& line_colors)
    {
        vector<StreetSection> undistorted_secs;
        for (unsigned int i = 0; i < img_lines.size(); i++)
        {
            const auto undistorted_seg = imgSegToRealSeg(img_lines[i]);
            undistorted_secs.emplace_back(line_colors[i], geometry::segmentToLine(undistorted_seg), undistorted_seg);
        }
        return undistorted_secs;
    }
    
    vector<StreetSection> findPossibleStreetSections(const vector<StreetSection>& tape_secs)
    {
        vector<StreetSection> short_secs;
        for (unsigned int i = 0; i < tape_secs.size(); i++)
        {
            vector<StreetSection> connected_secs;
            vector<StreetSection> imaginable_secs;
            // Separate the relevant segments
            int num_opposite_secs = 0;
            for (unsigned int j = 0; j < tape_secs.size(); j++)
            {
                if (i == j)
                    continue;                
                // If the segments are parallel, a lane's distance apart, but not collinear, they are opposite
                const float dist = geometry::distSegments(tape_secs[i].seg, tape_secs[j].seg);
                const bool are_parallel = geometry::linesAreParallel(tape_secs[i].line, tape_secs[j].line, consts::max_theta_diff);
                const bool are_a_lane_width_apart = (consts::lane_width*0.7 < dist && dist < consts::lane_width*1.3);
                const bool are_collinear = geometry::linesAreCollinear(tape_secs[i].line, tape_secs[j].line, consts::max_theta_diff, consts::lane_width/4);
                /*std::cout << "\tDistancia entre segmentos: " << dist << std::endl;
                std::cout << "\tSao paralelos: " << are_parallel << std::endl;
                std::cout << "\tTem a distancia certa: " << are_a_lane_width_apart << std::endl;
                std::cout << "\tSao colineares: " << are_collinear << std::endl << std::endl;*/
                if (are_parallel && are_a_lane_width_apart && (!are_collinear))
                {
                    num_opposite_secs++;
                    if (j > i)
                        insertMiddleSection(tape_secs[i], tape_secs[j], Color::blue, imaginable_secs);
                }
            }
            // If there are no segments opposite to the 'i' segment, try to insert a section parallel to it, closer to the vehicle
            if (num_opposite_secs == 0)
            {
                insertParallelSection(tape_secs[i], Color::blue, imaginable_secs);
            }
            
            // If the 'i' segment is green, add a perpendicular section
            if (tape_secs[i].color == Color::green)
            {
                insertPerpendicularSection(tape_secs[i], Color::green, imaginable_secs);
                //std::cout << "segmentos verdes " << new_seg1 << ' ' << new_seg2 << std::endl;
            }
            
            // If the segment is yellow and it is perpendicular to the vehicle, add a section crossing it
            else if (tape_secs[i].color == Color::yellow
                     && geometry::linesAreParallel(tape_secs[i].line, Vec2f(0, M_PI/2), consts::max_theta_diff)
                     && geometry::segmentLength(tape_secs[i].seg) > 0.13)
            {
                insertPerpendicularSection(tape_secs[i], Color::yellow, imaginable_secs);
                //std::cout << "colocando cara amarelo em imaginable_secs" << std::endl;
            }
            
            // Check if the proposed sections are not crossing blue or yellow
            //std::cout << "Verificando interseccao das secoes propostas" << std::endl;
            for (const auto& sec: imaginable_secs)
            {
                bool sec_possible = true;
                for (const auto& other_sec: tape_secs)
                {
                    if (sec == other_sec)
                        continue;
                    if (other_sec.color == Color::blue)
                    {
                        const float dist = geometry::distSegments(sec.seg, other_sec.seg);
                        /*std::cout << "\tsecao proposta: " << sec.as_str() << std::endl;
                        std::cout << "\tsecao que pode intersectar: " << other_sec.as_str() << std::endl;
                        std::cout << "\tdist: " << dist << std::endl << std::endl;*/
                        if (dist < (consts::lane_width/4))
                        {
                            //std::cout << "distancia pequena, negado" <<std::endl;
                            sec_possible = false;
                            break;
                        }
                    }
                }
                if (sec_possible)
                    short_secs.push_back(sec);
            }  
        }
        
        // Remove sections that are too short
        // When they are too short they often have a bad angle 
        vector<StreetSection> not_so_short_secs;
        for (const auto& sec: short_secs)
        {
            if (geometry::segmentLength(sec.seg) > 0.04)
                not_so_short_secs.push_back(sec);
        }
        
        return not_so_short_secs;
    }
    
    // Fuses overlapping collinear sections into long sections
    vector<StreetSection> groupIntoLongSections(const vector<StreetSection>& short_secs)
    {
        vector<StreetSection> long_secs;        
        // Group lines that can be potentially fused together
        const auto angle_groups = groupCollinearSections(short_secs, consts::max_theta_diff, consts::lane_width/4);
        /*for(auto group: angle_groups)
        {
            std::cout << "group: ";
            for (auto idx:group)
            {
                std::cout << idx << " ";
            }
            std::cout << std::endl;
        }*/
        //cout << "Agrupou as linhas colineares" << endl;
        // For each group of collinear sections
        for (const auto& group: angle_groups)
        {
            // Choose the axis used to order the points on the line
            int used_axis;
            const float group_angle = short_secs[group[0]].line[1];
            if ((group_angle < M_PI/4)
                || (((M_PI - M_PI/4) < group_angle) && (group_angle < (M_PI + M_PI/4)))
                || (group_angle > (2*M_PI - M_PI/4)))
            {
                used_axis = 1;
            }
            else
                used_axis = 0;
            //cout << "Escolheu eixo pra juntar linhas " << used_axis <<endl;
            
            // Copies the sections that belong to this group and order them by the chosen axis
            vector<StreetSection> collinear_sections;
            for (const auto& i: group)
            {
                collinear_sections.push_back(short_secs[i]);
            }
            orderCollinearSections(collinear_sections, used_axis);
            /*for(const auto& section : collinear_sections)
            {
                cout << section.seg << endl;
            }*/       
            //cout << "Copiei e ordenei colinear sections" <<endl;
            
            // Finds and joins the overlapping sections, creating different sections when there is a gap
            vector<Vec4f> result_segs;
            int secs_in_result = 0;
            Vec4f seg_in_construction;
            for (const auto& section: collinear_sections)
            {
                if (secs_in_result == 0)
                {
                    seg_in_construction = section.seg;
                    secs_in_result++;
                }
                else
                {
                    if (geometry::distSegments(seg_in_construction, section.seg) > consts::lane_width/4)
                    {
                        //cout << "coloquei em result o " << seg_in_construction <<endl;
                        result_segs.push_back(seg_in_construction);
                        seg_in_construction = section.seg;
                        secs_in_result = 1;
                    }
                    else
                    {
                        //cout << "seg in construc: " <<  segmentToLine(seg_in_construction) << " section: " << section.line << endl;
                        if (seg_in_construction[used_axis+2] < section.seg[used_axis+2]) 
                        {
                            seg_in_construction[2] = section.seg[2];
                            seg_in_construction[3] = section.seg[3];
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
            //cout << "Montou o segmento longo" << endl;
            for (const auto& result_seg: result_segs)
                long_secs.emplace_back(Color::none, geometry::segmentToLine(result_seg), result_seg);
        }
        return long_secs;
    }
    
    // Break the long sections where they intersect
    vector<StreetSection> breakIntersectingSections(const vector<StreetSection>& long_secs)
    {
        vector<StreetSection> broken_secs;
        for (unsigned int i = 0; i < long_secs.size(); i++)
        {
            // Find the cut points
            vector<Vec2f> cut_pts;
            cut_pts.emplace_back(long_secs[i].seg[0], long_secs[i].seg[1]);
            cut_pts.emplace_back(long_secs[i].seg[2], long_secs[i].seg[3]);
            for (unsigned int j = 0; j < long_secs.size(); j++)
            {
                if (i == j)
                    continue;
                // If the long sections are connected
                if (geometry::distSegments(long_secs[i].seg, long_secs[j].seg) < consts::lane_width/4)
                {
                    // Break the i section at the projected intersection point
                    auto intersection_pt = geometry::linesIntersection(long_secs[i].line, long_secs[j].line);
                    if (! std::isinf(intersection_pt[0]))
                        cut_pts.push_back(intersection_pt);
                }
            }
            /*cout << "cut_pts i=" << i << ":" << endl;
            for (auto pt: cut_pts)
                cout << '\t' << pt << endl;*/

            // Order the cutpoints
            geometry::orderCollinearPoints(cut_pts, long_secs[i].line[1]);
            /*cout << "cut_pts_ordered i=" << i << ":" << endl;
            for (auto pt: cut_pts)
                cout << '\t' << pt << endl;*/
            
            // Remove points that are too close
            vector<int> pts_to_remove;
            for (unsigned int j = 1; j < cut_pts.size(); j++)
            {
                const auto dist = geometry::distPoints(Point2f(cut_pts[j-1][0], cut_pts[j-1][1]),
                                                       Point2f(cut_pts[j][0], cut_pts[j][1]));
                //cout << "dist i=" << i <<" j=" << j << " - " << dist << endl;
                // Marks the point closer to the middle of the vector as to be removed
                // Don't remove points if there's only 2 left
                if ((dist < consts::lane_width/4) && (pts_to_remove.size() < (cut_pts.size()-2)))
                {
                    if (j < cut_pts.size()/2)
                        pts_to_remove.push_back(j);
                    else
                        pts_to_remove.push_back(j-1);
                }
            }
            /*cout << "pts_to_remove i=" << i << ":" << endl;
            for (auto pt: pts_to_remove)
                cout << '\t' << pt << endl;*/
            // Filters out the points marked as to be removed
            vector<Vec2f> cut_pts_filtered;
            for (unsigned int j = 0; j < cut_pts.size(); j++)
            {
                if (std::find(pts_to_remove.begin(), pts_to_remove.end(), j) == pts_to_remove.end())
                {
                    cut_pts_filtered.push_back(cut_pts[j]);
                }
            }
            /*cout << "cut_pts_filtered i=" << i << ":" << endl;
            for (auto pt: cut_pts_filtered)
                cout << '\t' << pt << endl;*/

            // Create sections from the cutpoints
            for (unsigned int j = 1; j < cut_pts_filtered.size(); j++)
            {
                Vec4f seg(cut_pts_filtered[j-1][0], cut_pts_filtered[j-1][1],
                          cut_pts_filtered[j][0], cut_pts_filtered[j][1]);
                broken_secs.emplace_back(Color::none, geometry::segmentToLine(seg), seg); 
            }
        }
        return broken_secs;
    }
   
    // Links together the sections, building a forest of sections that are connected
    // The linking happens in the connects_end_pt1 and connects_end_pt2 attributes of the StreeSections
    void buildSectionGraph(vector<StreetSection>& secs)
    {
        for (unsigned int i = 0; i < secs.size(); i++)
        {
            for (unsigned int j = i+1; j < secs.size(); j++)
            {
                // If any of the end points are close, link them
                if (geometry::distPoints(Point2f(secs[i].seg[0], secs[i].seg[1]),
                                         Point2f(secs[j].seg[0], secs[j].seg[1])) < consts::lane_width/4)
                {
                    secs[i].connects_end_pt1.push_back(&(secs[j]));
                    secs[j].connects_end_pt1.push_back(&(secs[i]));
                }
                else if (geometry::distPoints(Point2f(secs[i].seg[0], secs[i].seg[1]),
                                              Point2f(secs[j].seg[2], secs[j].seg[3])) < consts::lane_width/4)
                {
                    secs[i].connects_end_pt1.push_back(&(secs[j]));
                    secs[j].connects_end_pt2.push_back(&(secs[i]));
                }
                else if (geometry::distPoints(Point2f(secs[i].seg[2], secs[i].seg[3]),
                                              Point2f(secs[j].seg[0], secs[j].seg[1])) < consts::lane_width/4)
                {
                    secs[i].connects_end_pt2.push_back(&(secs[j]));
                    secs[j].connects_end_pt1.push_back(&(secs[i]));
                }
                else if (geometry::distPoints(Point2f(secs[i].seg[2], secs[i].seg[3]),
                                              Point2f(secs[j].seg[2], secs[j].seg[3])) < consts::lane_width/4)
                {
                    secs[i].connects_end_pt2.push_back(&(secs[j]));
                    secs[j].connects_end_pt2.push_back(&(secs[i]));
                }
            }
        }
    }
    
    // Finds the lines in the mask representing the street lane markers
    vector<Vec4i> getStreetLines(const Mat& lines_mask)
    {
        vector<Vec4i> lines;
        Mat labelImage(lines_mask.size(), CV_32S);
        cv::HoughLinesP(
            lines_mask, lines, consts::hough_precision_px,
            consts::hough_precision_rad, consts::hough_thresh,
            consts::hough_min_len, consts::hough_max_gap
        );
        if(consts::save_img)
            cv::imwrite("teste_linha_ruffles.jpg", drawSegments(lines, Mat::zeros(consts::img_height, consts::img_width, CV_8UC3)));
        //std::cout << "Linhas: " << lines.size() << std::endl;
        int nLabels = cv::connectedComponents(lines_mask, labelImage, 8);
        //std::cout << "numero de labels" << nLabels<<std::endl;
        lines = reduceSegments(lines, labelImage, nLabels);
        return lines;
    }

    vector<Vec4i> reduceSegments (const vector<Vec4i>& segs, const Mat& label_image, const int n_labels)
    {
        //std::cout << "\nreduceSegments com " << segs.size() << " segs" << std::endl;
        // Separate lines by blob
        if (consts::save_img)
            drawLabelImage(label_image, n_labels);
        vector<vector<Vec4i>> segs_by_blob(n_labels);
        for (const auto& seg: segs)
        {
            const int label_1 = label_image.at<int>(seg[1], seg[0]);
            const int label_2 = label_image.at<int>(seg[3], seg[2]);
            if ((label_1 == label_2)
                || (label_1 != 0  && label_2 == 0)
                || (label_2 != 0  && label_1 == 0))
            {
                (segs_by_blob[label_1 ? label_1 : label_2]).push_back(seg);
            }
        }

        vector<Vec4i> out_segs;
        for (const auto& blob: segs_by_blob)
        {
            //std::cout << "esse blob tem tamanho " << blob.size() << std::endl;
            vector<Vec2f> lines(blob.size());
            std::transform(blob.begin(), blob.end(), lines.begin(), geometry::segmentToLine);
            auto groups = groupCollinearLines(lines, consts::max_theta_diff, 200);
            //int a = 0;
            for (const auto& group: groups)
            {         
                //std::cout << "group " << a << " " << group.size()<< std::endl;
                //a++;
                // Ignore very small groups
                if (group.size() < 10)
                    continue;

                float theta = 0;
                vector<cv::Vec2f> seg_pts;
                for (const auto& i: group)
                {
                    seg_pts.emplace_back(blob[i][0], blob[i][1]);
                    seg_pts.emplace_back(blob[i][2], blob[i][3]);
                    const float line_angle = lines[i][1];
                    if (line_angle > M_PI)
                        theta += line_angle - M_PI;
                    else
                        theta += line_angle;
                }
                theta /= group.size();
                //std::cout << "angle " << theta <<std::endl;
                geometry::orderCollinearPoints(seg_pts, theta);
                
                Vec4i result_seg(0, 0, 0, 0);
                const unsigned int n_pts = seg_pts.size();
                float constexpr percentile = 0.3;
                for (int i = 0; i < int(percentile*n_pts); i++)
                {
                    result_seg[0] += seg_pts[i][0];
                    result_seg[1] += seg_pts[i][1];
                    result_seg[2] += seg_pts[n_pts-i-1][0];
                    result_seg[3] += seg_pts[n_pts-i-1][1];
                }
                result_seg[0] /= int(percentile*n_pts);
                result_seg[1] /= int(percentile*n_pts);
                result_seg[2] /= int(percentile*n_pts);
                result_seg[3] /= int(percentile*n_pts);
                out_segs.push_back(result_seg);
            }
        }
        /*std::cout << "reduce lines" << std::endl;
        for (auto seg:out_segs)
            std::cout << seg << std::endl;*/
        return out_segs;
    }

    // Undoes the projection distortion for a single segment
    // Transforms from the image domain to "real distances" domain
    Vec4f imgSegToRealSeg(const Vec4i& seg)
    {
        float constexpr x_center = consts::img_width / 2;
        // Theta max e menor que theta min porque é theta da distancia maxima (mais perto do horizonte)
        float constexpr px_per_rad = (consts::img_height/(consts::img_theta_min - consts::img_theta_max));
        
        const float theta1 = seg[1]/px_per_rad + consts::img_theta_max;
        const float y1_m = consts::cam_height_m / tan(theta1);
        const float phi1 = (seg[0] - x_center) / px_per_rad;
        const float x1_m = tan(phi1) * y1_m;
        
        const float theta2 = seg[3]/px_per_rad + consts::img_theta_max;
        const float y2_m = consts::cam_height_m / tan(theta2);
        const float phi2 = (seg[2] - x_center) / px_per_rad;
        const float x2_m = tan(phi2) * y2_m;
        
        return Vec4f(x1_m, y1_m, x2_m, y2_m);
    }

    // From a group of maybe repeated lines, select only the unique ones, separating them by their distance (rho)
    // Return the indexes of the unique lines in the input vector
    vector<vector<unsigned int>> groupLinesByDistance(const vector<Vec2f>& lines, const float max_rho_diff)
    {
        vector<vector<unsigned int>> groups; // Each element has a group of lines that are repeated
        vector<int> classification(lines.size(), -1); // Which group a line is part of
        int counter = 0;
        for (unsigned int i = 0; i < lines.size(); i++)
        {
            if (classification[i] == -1)
            {
                groups.push_back(vector{i});
                classification[i] = counter;
                for (unsigned int j = i+1; j < lines.size(); j++)
                {
                    if ((classification[j] == -1)
                        && ((lines[i][0]-max_rho_diff) < lines[j][0])
                        && (lines[j][0] < (lines[i][0]+max_rho_diff)))
                    {
                        classification[j] = counter;
                        groups[counter].push_back(j);
                    }
                }
                counter++;
            }
        }
        return groups;
    }

    // Separate the lines in different groups of collinear lines
    vector<vector<unsigned int>> groupCollinearLines(const vector<Vec2f>& lines, const float max_theta_diff, const float max_rho_diff)
    {
        vector<int> classification(lines.size(), -1);
        vector<vector<unsigned int>> groups;
        int counter = 0;
        for (unsigned int i = 0; i < lines.size(); i++)
        {
            if (classification[i] == -1)
            {
                classification[i] = counter;
                groups.push_back(vector{i});
                for (unsigned int j = i+1; j < lines.size(); j++)
                {
                    bool lines_are_collinear = geometry::linesAreCollinear(lines[i], lines[j], max_theta_diff, max_rho_diff);
                    //std::cout << lines[i] << " " <<lines[j] <<std::endl;
                    //std::cout << "colineares: " << lines_are_collinear << std::endl;
                    if ((classification[j] == -1) && lines_are_collinear)
                    {
                        classification[j] = counter;
                        groups[counter].push_back(j);
                    }
                }
                counter++;
            }
        }
        return groups;
    }

    // Separate the lines in different groups of collinear sections
    vector<vector<unsigned int>> groupCollinearSections(const vector<StreetSection>& secs, const float max_theta_diff, const float max_dist)
    {
        vector<int> classification(secs.size(), -1);
        vector<vector<unsigned int>> groups;
        int counter = 0;
        for (unsigned int i = 0; i < secs.size(); i++)
        {
            if (classification[i] == -1)
            {
                classification[i] = counter;
                groups.push_back(vector{i});
                for (unsigned int j = i+1; j < secs.size(); j++)
                {
                    bool secs_are_parallel = geometry::linesAreParallel(secs[i].line, secs[j].line, max_theta_diff);
                    bool secs_are_close = geometry::distSegments(secs[i].seg, secs[j].seg) < max_dist;
                    //std::cout << lines[i] << " " <<lines[j] <<std::endl;
                    //std::cout << "colineares: " << lines_are_collinear << std::endl;
                    if ((classification[j] == -1) && secs_are_parallel && secs_are_close)
                    {
                        classification[j] = counter;
                        groups[counter].push_back(j);
                    }
                }
                counter++;
            }
        }
        return groups;
    }

    // Does a stable in-place sort of collinear street sections
    void orderCollinearSections(std::vector<StreetSection>& sections, const int used_axis)
    {
		for (auto& section: sections)
		{
			if (section.seg[used_axis] > section.seg[used_axis+2])
			{
				std::swap(section.seg[0], section.seg[2]);
				std::swap(section.seg[1], section.seg[3]);
			}
		}
        std::stable_sort(
	        sections.begin(), sections.end(),
            [used_axis](auto sec1, auto sec2){return sec1.seg[used_axis] < sec2.seg[used_axis];}
	    );
    }
    
    tuple<Mat, Mat, Mat> getTapeMasks(const Mat& img)
    {
        return tuple<Mat, Mat, Mat> (
            getBlueTapeMask(img),
            getGreenTapeMask(img),
            getYellowTapeMask(img)
        );
    }

    cv::Mat getColorMask(const cv::Mat& img, const cv::Scalar min, const cv::Scalar max)
    {
        cv::Mat out;
        cv::inRange(img, min, max, out);
        const cv::Mat krn_open = cv::getStructuringElement(cv::MORPH_RECT, cv::Size(5, 5));
        cv::morphologyEx(out, out, cv::MORPH_OPEN, krn_open);
        const cv::Mat krn_close = cv::getStructuringElement(cv::MORPH_RECT, cv::Size(5, 5));
        cv::morphologyEx(out, out, cv::MORPH_CLOSE, krn_close);
        return out;
    }

    Mat getTapeMask(const Mat& img, const cv::Scalar min, const cv::Scalar max)
    {
        Mat raw_mask = getColorMask(img, min, max);
        vector<vector<cv::Point>> cnts;
        vector<Vec4i> hierarchy;
        cv::findContours(raw_mask, cnts, hierarchy, cv::RETR_TREE, cv::CHAIN_APPROX_SIMPLE);
        vector<vector<cv::Point>> filtered_cnts(cnts.size());
        auto it = std::copy_if(cnts.begin(), cnts.end(), filtered_cnts.begin(),
                               [](auto cnt){return cv::contourArea(cnt) > 2000;});
        filtered_cnts.resize(std::distance(filtered_cnts.begin(), it));
        Mat filtered_mask = cv::Mat::zeros(raw_mask.size(), CV_8U);
        cv::drawContours(filtered_mask, filtered_cnts, -1, 255, cv::FILLED);
        return filtered_mask;
    }

    Mat getRedTapeMask(const Mat& img)
    {
        return getTapeMask(img, cv::Scalar(160, 100, 40), cv::Scalar(180, 240, 140));
    }

    Mat getBlueTapeMask(const Mat& img)
    {
        //std::cout << "Pegando mascara azul" << std::endl;
        return getTapeMask(img, cv::Scalar(85, 100, 40), cv::Scalar(105, 250, 255));
    }

    Mat getGreenTapeMask(const Mat& img)
    {
        //std::cout << "Pegando mascara verde" << std::endl;
        return getTapeMask(img, cv::Scalar(40, 100, 60), cv::Scalar(60, 250, 255));
    }

    Mat getYellowTapeMask(const Mat& img)
    {
        //std::cout << "Pegando mascara amarela" << std::endl;
        return getTapeMask(img, cv::Scalar(20, 170, 115), cv::Scalar(45, 240, 255));
    }

    Mat getWhiteTapeMask(const Mat& img)
    {
        return getTapeMask(img, cv::Scalar(20, 120, 0), cv::Scalar(110, 255, 70));
    }

    Mat getGroundMask(const Mat& img)
    {
        return getColorMask(img, cv::Scalar(0, 50, 0), cv::Scalar(180, 120, 30));
    }
    
    // Inserts two halfs of a section parallel to sec, closer to the vehicle, into sec_vec
    void insertParallelSection(const StreetSection& sec, const Color color, vector<StreetSection>& sec_vec)
    {
        const Vec2f translation(consts::lane_width/2 * cos(sec.line[1]+M_PI),
                                consts::lane_width/2 * sin(sec.line[1]+M_PI));
        const Vec4f translated(sec.seg[0]+translation[0], sec.seg[1]+translation[1],
                               sec.seg[2]+translation[0], sec.seg[3]+translation[1]);
        const Vec2f mid_pt_transl = geometry::segmentHalfPoint(translated);
        const Vec4f transl_half1(translated[0], translated[1], mid_pt_transl[0], mid_pt_transl[1]);
        const Vec4f transl_half2(mid_pt_transl[0], mid_pt_transl[1], translated[2], translated[3]);
        sec_vec.emplace_back(color, geometry::segmentToLine(transl_half1), transl_half1);
        sec_vec.emplace_back(color, geometry::segmentToLine(transl_half2), transl_half2);
    }
    
    // Inserts a section perpendicular to sec, sharing the middle point with sec, into sec_vec
    void insertPerpendicularSection(const StreetSection& sec, const Color color, vector<StreetSection>& sec_vec)
    {
        const Vec2f translation(consts::lane_width/2 * cos(sec.line[1]),
                                consts::lane_width/2 * sin(sec.line[1]));
        const Vec2f mid_pt = geometry::segmentHalfPoint(sec.seg);
        const Vec4f new_seg(
            mid_pt[0] + translation[0], mid_pt[1] + translation[1],
            mid_pt[0] - translation[0], mid_pt[1] - translation[1]
        );
        sec_vec.emplace_back(color, geometry::segmentToLine(new_seg), new_seg);
    }
    
    // Inserts two halfs of a section that goes between two parallel sections
    void insertMiddleSection(const StreetSection& sec1, const StreetSection& sec2, const Color color, vector<StreetSection>& sec_vec)
    {
        float dist11 = geometry::distPoints(Point2f(sec1.seg[0], sec1.seg[1]),
                                            Point2f(sec2.seg[0], sec2.seg[1]));
        float dist12 = geometry::distPoints(Point2f(sec1.seg[0], sec1.seg[1]),
                                            Point2f(sec2.seg[2], sec2.seg[3]));
        Vec2f pt1, pt2;
        if (dist11 < dist12)
        {
            pt1 = geometry::segmentHalfPoint(Vec4f(sec1.seg[0], sec1.seg[1],
                                                   sec2.seg[0], sec2.seg[1]));
            pt2 = geometry::segmentHalfPoint(Vec4f(sec1.seg[2], sec1.seg[3],
                                                   sec2.seg[2], sec2.seg[3]));
        }
        else
        {
            pt1 = geometry::segmentHalfPoint(Vec4f(sec1.seg[0], sec1.seg[1],
                                                   sec2.seg[2], sec2.seg[3]));
            pt2 = geometry::segmentHalfPoint(Vec4f(sec1.seg[2], sec1.seg[3],
                                                   sec2.seg[0], sec2.seg[1]));
        }
        const Vec2f mid_pt = geometry::segmentHalfPoint(Vec4f(pt1[0], pt1[1], pt2[0], pt2[1]));
        const Vec4f new_seg1(pt1[0], pt1[1], mid_pt[0], mid_pt[1]);
        const Vec4f new_seg2(mid_pt[0], mid_pt[1], pt2[0], pt2[1]);
        /*std::cout << "Inserindo middle section das seguintes secoes" << std::endl;
        std::cout << '\t' << sec1.as_str() << std::endl;
        std::cout << '\t' << sec2.as_str() << std::endl;
        std::cout << "Middle sections: " << new_seg1 << " " << new_seg2 << std::endl;*/
        sec_vec.emplace_back(color, geometry::segmentToLine(new_seg2), new_seg2);
        sec_vec.emplace_back(color, geometry::segmentToLine(new_seg1), new_seg1);
    }
    
    // Returns a bgr version of the image with the segments drawn on it
    Mat drawSegments(const vector<Vec4i>& segs, const Mat& img, const bool cvt_color)
    {
        Mat img_segs;
        if (cvt_color)
            cv::cvtColor(img, img_segs, cv::COLOR_HLS2BGR);
        else
            img_segs = img.clone();

        for (const auto& seg: segs)
        {
            cv::line(img_segs, Point2f(seg[0], seg[1]), Point2f(seg[2], seg[3]), Scalar(0, 255, 0), 1, cv::LINE_AA);
        }
        return img_segs;
    }
    
    // Draws the line on the image
    void drawLine(const Vec2f& line, const Mat& img)
    {
        const auto len = 2*(img.rows + img.cols);
        const float a = cos(line[1]);
        const float b = sin(line[1]);
        const float x0 = a * line[0];
        const float y0 = b * line[0];
        const cv::Point2f pt1(x0 + len*(-b), y0 + len*a);
        const cv::Point2f pt2(x0 - len*(-b), y0 - len*a);
        cv::line(img, pt1, pt2, 255, 1, cv::LINE_8);
    }
    
    // Draws a labelized image
    void drawLabelImage(const cv::Mat& label_img, int num_labels)
    {
        std::vector<cv::Vec3b> colors(num_labels);
        colors[0] = cv::Vec3b(0, 0, 0);  // background
        for(int label = 1; label < num_labels; label++)
        {
            colors[label] = cv::Vec3b((rand()%256), (rand()%256), (rand()%256));
        }
        cv::Mat dst(label_img.size(), CV_8UC3);
        for(int r = 0; r < dst.rows; ++r)
        {
            for(int c = 0; c < dst.cols; ++c)
            {
                int label = label_img.at<int>(r, c);
                cv::Vec3b& pixel = dst.at<cv::Vec3b>(r, c);
                pixel = colors[label];
            }
        }
        cv::imwrite("blobs.jpg", dst);
    }
}
