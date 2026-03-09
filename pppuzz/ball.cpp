#include <random>
#include <numeric>
#include "opencv2/opencv.hpp"
#include "log.h"
#include "frame.h"
#include "random.h"
#include "ball.h"
#include "color.h"

Ball::Ball() {}
Ball::Ball(const int& xMax, const int& yMax)
{
	auto& rng = ThreadSafeRandom::getInstance();
	this->radius = BALL_RADIUS;
	this->position.x = rng.randDouble(0, xMax);
	this->position.y = rng.randDouble(0, yMax);
	float velocitySign = rng.randDouble(-1, 1) > 0 ? 1 : -1;
	this->velocity.x = velocitySign * rng.randDouble(MIN_VELOCITY, MAX_VELOCITY);
	velocitySign = rng.randDouble(-1, 1) > 0 ? 1 : -1;
	this->velocity.y = velocitySign * rng.randDouble(MIN_VELOCITY, MAX_VELOCITY);
	this->angular_velocity = rng.randDouble(-MAX_ANGULAR_VELOCITY, MAX_ANGULAR_VELOCITY);
	float dirRad = rng.randDouble(0, CV_PI);
	this->dir.x = cos(dirRad);
	this->dir.y = sin(dirRad);
	if (Color::colorMap.empty())
	{
		this->color_b = 255;
		this->color_g = 255;
		this->color_b = 255;
	}
	else
	{
		int r = rng.randInt(0, Color::colorMap.rows) % Color::colorMap.rows;
		int c = rng.randInt(0, Color::colorMap.cols) % Color::colorMap.cols;
		this->color_r = Color::colorMap.at<cv::Vec3b>(r, c)[2];
		this->color_g = Color::colorMap.at<cv::Vec3b>(r, c)[1];
		this->color_b = Color::colorMap.at<cv::Vec3b>(r, c)[0];
	}
}
Ball Ball::generAnewFallBall(const int& posx, const int& posy,const float& velocity)
{
	Ball ret;
	auto& rng = ThreadSafeRandom::getInstance();
	ret.radius = BALL_RADIUS;
	ret.position.x = posx;
	ret.position.y = posy;
	float velocitySign = 1;
	ret.velocity.x = 0;
	ret.velocity.y = velocity;
	ret.angular_velocity = 0;
	float dirRad = rng.randDouble(0, CV_PI);
	ret.dir.x = 0;
	ret.dir.y = 1;
	if (Color::colorMap.empty())
	{
		ret.color_b = 255;
		ret.color_g = 255;
		ret.color_b = 255;
	}
	else
	{
		int r = rng.randInt(0, Color::colorMap.rows) % Color::colorMap.rows;
		int c = rng.randInt(0, Color::colorMap.cols) % Color::colorMap.cols;
		ret.color_r = Color::colorMap.at<cv::Vec3b>(r, c)[2];
		ret.color_g = Color::colorMap.at<cv::Vec3b>(r, c)[1];
		ret.color_b = Color::colorMap.at<cv::Vec3b>(r, c)[0];
	}
	return ret;
}
cv::Mat Ball::draw(const cv::Mat& img, const std::vector<Ball>balls)
{
	if (img.empty())
	{
		LOG_ERR_OUT << "img.empty";
		return  cv::Mat();
	}
	cv::Mat copy;
	img.copyTo(copy);
	for (const auto& d : balls)
	{
		cv::circle(copy, d.position, d.radius, cv::Scalar(d.color_b, d.color_g, d.color_r), -1);
	}
	return copy;
}