#ifndef _FLOATER_H__
#define _FLOATER_H__
#include <unordered_map>
#include <vector>
#include "opencv2/opencv.hpp"
#include "ball.h"
#include "mapEdit.h"
struct Floater
{
	Floater();
	void setEntry(const std::vector<MapElement>& papElement);
	int postionX;
	int velocityMax;
	int velocity;
	int floaterWidth;
	int floaterHeight;	
	std::vector<cv::Point>entry;
	std::unordered_map<int, int>collapsThre;
	std::unordered_map<int, cv::Point2f>floaterNormal;
	void figureScore(const std::vector<Ball>&balls);
	static void draw(cv::Mat&mat,const Floater&f);
};

#endif // !_FLOATER_H__
