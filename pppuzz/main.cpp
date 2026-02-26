#include <iostream>
#include <fstream>
#include <vector>
#include <random>
#include <list>
#include "json/json.h"
#include "opencv2/opencv.hpp"
#include "log.h"
#include "random.h"

static cv::Mat colorMap = cv::imread("../colorMap/bremm.png");
constexpr static int IMG_HEIGHT = 480;
constexpr static int IMG_WIDTH = 360;
constexpr static int INVALID_HASH = IMG_HEIGHT* IMG_WIDTH;
constexpr static int BALL_RADIUS = 3;
constexpr static int MAX_VELOCITY = 10;
constexpr static int MAX_ANGULAR_VELOCITY = 3;

struct Ball
{
	std::uint32_t posHash;
	cv::Point2f dir;
	cv::Point2f velocity;
	float angular_velocity;
	cv::Point2f position;
	std::uint8_t radius;
	std::uint8_t color_r;
	std::uint8_t color_g;
	std::uint8_t color_b;
	Ball() {}
	Ball(const int& xMax, const int& yMax) 
	{
		auto& rng = ThreadSafeRandom::getInstance();
		this->radius = BALL_RADIUS;
		this->position.x = rng.randDouble(0, xMax);
		this->position.y = rng.randDouble(0, yMax);
		this->velocity.x = rng.randDouble(0, MAX_VELOCITY);
		this->velocity.y = rng.randDouble(0, MAX_VELOCITY);
		this->angular_velocity = rng.randDouble(-MAX_ANGULAR_VELOCITY, MAX_ANGULAR_VELOCITY);
		this->dir.x = rng.randDouble(-1, 1);
		this->dir.y = rng.randDouble(-1, 1);
		this->dir /= cv::norm(this->dir);
		if (colorMap.empty())
		{
			this->color_b = 255;
			this->color_g = 255;
			this->color_b = 255;
		}
		else
		{
			int r = rng.randInt(0, colorMap.rows) % colorMap.rows;
			int c = rng.randInt(0, colorMap.cols) % colorMap.cols;
			this->color_r = colorMap.at<cv::Vec3b>(r, c)[2];
			this->color_g = colorMap.at<cv::Vec3b>(r, c)[1];
			this->color_b = colorMap.at<cv::Vec3b>(r, c)[0];
		}
	}
	static cv::Mat draw(const cv::Mat& img, const std::vector<Ball>balls)
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
	static void upDate(std::vector<Ball>&balls, const float& period)
	{
		std::vector<std::pair<std::uint32_t, std::uint32_t>>posHash(balls.size());
		for (int i = 0; i < balls.size(); i++)
		{ 
			balls[i].position.x += balls[i].velocity.x * period;
			balls[i].position.y += balls[i].velocity.y * period;
			//border reflect here    ------------------------------------------
			int hashPosX = std::floor(balls[i].position.x);
			int hashPosY = std::floor(balls[i].position.y);
			if (hashPosX < 0 || hashPosX >= IMG_WIDTH || hashPosY < 0 || hashPosY >= IMG_HEIGHT)
			{
				posHash[i].first = INVALID_HASH;
				posHash[i].second = i;
				balls[i].posHash = posHash[i].first;
			}
			else 
			{
				posHash[i].first = IMG_WIDTH * hashPosY + hashPosX;
				posHash[i].second = i; 
				balls[i].posHash = posHash[i].first;
			}
		}
		std::sort(posHash.begin(), posHash.end(), [](const auto& a, const auto& b) {return a.first < b.first; });
		std::vector<Ball> balls_sorted(balls.size());
		for (int i = 0; i < posHash.size(); i++)
		{
			balls_sorted[i] = balls[posHash[i].second];
		}
		balls = balls_sorted;
		return;
	}
};
int main()
{
	cv::Mat img = cv::Mat::zeros(IMG_HEIGHT, IMG_WIDTH, CV_8UC3);
	std::vector<Ball>balls;
	for (size_t i = 0; i < 100; i++)
	{
		balls.emplace_back(Ball(IMG_WIDTH, IMG_HEIGHT));
	}
	for (size_t i = 0; i < 200; i++)
	{
		cv::Mat frame = Ball::draw(img, balls);
		Ball::upDate(balls,0.5);
		cv::imshow("1",frame);
		cv::waitKey(15);
	}
	
	return 0;
}