#ifndef __BALL_H__
#define __BALL_H__
#include <numeric>
#include "opencv2/opencv.hpp"
struct Ball
{
	std::uint32_t gridPos;
	cv::Point2f position;
	cv::Point2f dir;
	cv::Point2f velocity;
	float angular_velocity;
	std::uint8_t radius;
	std::uint8_t color_r;
	std::uint8_t color_g;
	std::uint8_t color_b;
	Ball();
	Ball(const int& xMax, const int& yMax);
	static Ball generAnewFallBall(const int& posx, const int& posy,const float& velocity);
	static cv::Mat draw(const cv::Mat& img, const std::vector<Ball>balls);
};

#endif // !__BALL_H__
