#ifndef _FLOATER_H__
#define _FLOATER_H__
#include <unordered_map>
#include "opencv2/opencv.hpp"
struct Floater
{
	Floater();
	int postionX;
	int floaterWidth;
	int floaterHeight;

	std::unordered_map<int, int>collapsThre;
	std::unordered_map<int, cv::Point2f>floaterNormal;
	static void draw(cv::Mat&mat,const Floater&f);
};

#endif // !_FLOATER_H__
