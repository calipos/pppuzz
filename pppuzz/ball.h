#ifndef __BALL_H__
#define __BALL_H__
#include <numeric>
#include "opencv2/opencv.hpp"
struct Ball
{
	enum class BallType
	{
		Nromal=1,
		DoubleAward=2,
	};
	std::uint32_t gridPos;
	cv::Point2f position;
	cv::Point2f dir;
	cv::Point2f velocity;
	float angular_velocity;
	std::uint8_t radius;
	std::uint8_t color_r;
	std::uint8_t color_g;
	std::uint8_t color_b;
	BallType ballType;
	Ball();
	Ball(const int& xMax, const int& yMax);
	static Ball generAnewFallBall(const int& posx, const int& posy,const float& velocity);
	static void draw(cv::Mat& img, const std::vector<Ball>balls);
};

#endif // !__BALL_H__
