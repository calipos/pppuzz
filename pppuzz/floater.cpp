#include <list>
#include <tuple>
#include <vector>
#include "floater.h"
#include "frame.h"
Floater::Floater()
{
	velocityMax = 5;
	this->postionX = IMG_WIDTH / 2;
	floaterWidth = IMG_WIDTH / 11;
	floaterHeight = floaterWidth/5;
	{
		postionX;
		cv::Mat mask = cv::Mat::zeros(floaterHeight + 2, floaterWidth * 2 + 3, CV_8UC1);
		cv::ellipse(mask, cv::Point(mask.cols / 2, mask.rows), cv::Point(floaterWidth, floaterHeight), 0, 0, 360, cv::Scalar(255), -1);
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
						floaterNormal[c - mask.cols / 2] = cv::Point2f((ellipseB+20) * (ellipseB + 20) * (c - mask.cols / 2), ellipseA * ellipseA * (r-mask.rows));
						floaterNormal[c - mask.cols / 2] /= cv::norm(floaterNormal[c - mask.cols / 2]);
						collapsThre[c- mask.cols / 2] = mask.rows - r;
						break;
					}
				}
			}
		}
		//cv::Mat testNorm = cv::Mat::zeros(mask.rows * 5, mask.cols * 2, CV_8UC3);
		//int testNormFloaterPosX = testNorm.cols / 2;
		//cv::ellipse(testNorm, cv::Point(testNormFloaterPosX, testNorm.rows), cv::Point(floaterWidth, floaterHeight), 0, 0, 360, cv::Scalar(255,255,255), -1);
		//for (int c = 0; c < testNorm.cols; c+=2)
		//{
		//	if (floaterNormal.count(c- testNormFloaterPosX)>0)
		//	{
		//		cv::Point dir = 20 * floaterNormal[c - testNormFloaterPosX];
		//		cv::Point a(c, testNorm.rows - collapsThre[c - testNormFloaterPosX]);
		//		cv::Point b = a + dir;
		//		cv::line(testNorm,a,b,cv::Scalar(0,0,255),1);
		//	}
		//}



		//reflectDir = I−2(In⋅Normal)Normal
		return;
	}
}
void Floater::setEntry(const std::vector<MapElement>& mapElement)
{
	for (const auto&d: mapElement)
	{
		if (d.elementType==MapElement::ElementType::Entry)
		{
			entry.emplace_back(d.metaData[0], d.metaData[1]);
		}
	} 
	return;
}
void Floater::figureScore(const std::vector<Ball>& balls)
{
	if (entry.size()<1)
	{
		velocity = 0;
		return;
	}
	std::list<std::tuple<int,float,float>>floaterPosAndScoreAndTime;
	for (int i = 0; i < balls.size(); i++)
	{ 
		if (balls[i].velocity.y>0)
		{
			float time = (IMG_HEIGHT - balls[i].position.y) / balls[i].velocity.y;
			float localX = time * balls[i].velocity.x + balls[i].position.x;
			int recursiveCnt = 0;
			cv::Point2f incidentRay = balls[i].velocity;
			while (localX<0)
			{
				localX = localX + IMG_WIDTH;
				recursiveCnt += 1;
				if (localX>=0)
				{
					if (recursiveCnt%2==1)
					{
						localX = IMG_WIDTH - 1 - localX;
						incidentRay.x *= -1;
					}
					break;
				}
			}
			recursiveCnt = 0;
			while (localX >= IMG_WIDTH)
			{
				localX = localX - IMG_WIDTH;
				recursiveCnt += 1;
				if (localX < IMG_WIDTH)
				{
					if (recursiveCnt % 2 == 1)
					{
						localX = IMG_WIDTH - 1 - localX;
						incidentRay.x *= -1;
					}
					break;
				}
			}
			incidentRay /= cv::norm(incidentRay);

			 
			for (size_t entryIdx = 0; entryIdx < entry.size(); entryIdx++)
			{
				const cv::Point& targetEntry = entry[entryIdx];
				for (const auto& floaterEdge : collapsThre)
				{
					int edgeHeight = floaterEdge.second;
					cv::Point2f edheNormDir = floaterNormal[floaterEdge.first];
					cv::Point2f hitPoint = cv::Point2f(localX, IMG_HEIGHT) - incidentRay * edgeHeight;
					cv::Point2f reflectRay = cv::Point2f(targetEntry) - hitPoint;
					reflectRay /= cv::norm(reflectRay);
					cv::Point2f reflectRay2 = incidentRay - incidentRay.dot(edheNormDir) * edheNormDir * 2;
					float score = reflectRay2.dot(reflectRay);
					floaterPosAndScoreAndTime.emplace_back(std::make_tuple(hitPoint.x - floaterEdge.first, score, time));
				}
			}			 
		}
	}
	std::sort(floaterPosAndScoreAndTime.begin(), floaterPosAndScoreAndTime.end(), [](const auto& a, const auto& b) {return std::get<1>(a) > std::get<1>(b); });
	for (const auto&d: floaterPosAndScoreAndTime)
	{
		float v = (std::get<0>(d) - postionX) / std::get<2>(d);
		if (abs(v)<velocityMax)
		{
			velocity = v;
			break;
		}
	}
	return;
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