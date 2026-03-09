
#include <vector>
#include "floater.h"
#include "frame.h"
Floater::Floater()
{
	this->postionX = IMG_WIDTH / 2;
	floaterWidth = IMG_WIDTH / 11;
	floaterHeight = floaterWidth/5;
	{
		cv::Mat mask = cv::Mat::zeros(floaterHeight + 2, floaterWidth * 2 + 3, CV_8UC1);
		cv::ellipse(mask, cv::Point(mask.cols / 2, mask.rows), cv::Point(floaterWidth, floaterHeight), 0, 0, 360, cv::Scalar(255), -1);
		//cv::Mat mask2 = cv::Mat::zeros(floaterHeight * 2+3, floaterWidth * 2 + 3, CV_8UC1);
		//cv::ellipse(mask2, cv::Point(mask.cols / 2, floaterHeight), cv::Point(floaterWidth, floaterHeight), 0, 0, 360, cv::Scalar(255), -1);
		int lastRowIdx = mask.rows - 1;

		float ellipseA = mask.cols *0.5;
		float ellipseB = mask.rows; 

		for (int c = 0; c < mask.cols; c++)
		{
			if (mask.ptr<uchar>(lastRowIdx)[c]>0)
			{
				for (int r = 0; r < mask.rows; r++)
				{
					if (mask.ptr<uchar>(r)[c]==0)
					{
						continue;
					}
					else
					{
						floaterNormal[c - mask.cols / 2] = cv::Point2f(ellipseB * ellipseB * (c - mask.cols), ellipseA * ellipseA * mask.rows - r);
						floaterNormal[c - mask.cols / 2] /= cv::norm(floaterNormal[c - mask.cols / 2]);
						collapsThre[c- mask.cols / 2] = mask.rows - r;
						break;
					}
				}
			}
		}

		cv::Mat testNorm = cv::Mat::zeros(mask.rows*5, mask.cols);


		//reflectDir = I−2(In⋅Normal)Normal
		return;
	}
}
void Floater::draw(cv::Mat& mat, const Floater& f)
{
	cv::ellipse(mat, cv::Point(f.postionX, IMG_HEIGHT), cv::Point(f.floaterWidth, f.floaterHeight), 0, 0, 360, cv::Scalar(255, 255, 255), -1);
	//cv::ellipse(mat, cv::Point(f.postionX, IMG_HEIGHT), cv::Point(f.floaterWidth, 9), 0, 0, 360, cv::Scalar(0, 0, 255), -1);
	//cv::ellipse(mat, cv::Point(f.postionX, IMG_HEIGHT), cv::Point(f.floaterWidth, 7), 0, 0, 360, cv::Scalar(255, 0, 0), -1);
	//cv::ellipse(mat, cv::Point(f.postionX, IMG_HEIGHT), cv::Point(f.floaterWidth, 5), 0, 0, 360, cv::Scalar(255, 255, 255), -1);
	//cv::ellipse(mat, cv::Point(f.postionX, IMG_HEIGHT), cv::Point(f.floaterWidth, 3), 0, 0, 360, cv::Scalar(255, 0, 255), -1);
	return;
}