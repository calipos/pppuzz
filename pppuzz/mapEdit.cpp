#include <vector>
#include <list>
#include <sstream>
#include <iostream>
#include <fstream>
#include <filesystem>
#include "opencv2/opencv.hpp"
#include "mapEdit.h"
#include "frame.h"
#include "log.h"
MapElement MapElement::createEntry(const float& x, const float& y)
{
	MapElement ret;
	ret.elementType = ElementType::Entry;
	ret.shape = Shape::Rectangle;
	ret.health = std::numeric_limits<std::uint32_t>::max();
	ret.metaData.resize(2);
	ret.metaData[0] = x;
	ret.metaData[1] = y;
	return ret;
}
MapElement MapElement::createRectBorder(const float& leftUpX, const float& leftUpY, const float& width, const float& height)
{
	MapElement ret;
	ret.elementType = ElementType::Border;
	ret.shape = Shape::Rectangle;
	ret.health = std::numeric_limits<std::uint32_t>::max();
	ret.metaData.resize(4);
	ret.metaData[0] = leftUpX;
	ret.metaData[1] = leftUpY;
	ret.metaData[2] = width;
	ret.metaData[3] = height;
	return ret;
}
MapElement MapElement::createRectBrick(const float& leftUpX, const float& leftUpY, const float& width, const float& height, const int& health)
{
	MapElement ret;
	ret.elementType = ElementType::Brick;
	ret.shape = Shape::Rectangle;
	ret.health = health;
	ret.metaData.resize(4);
	ret.metaData[0] = leftUpX;
	ret.metaData[1] = leftUpY;
	ret.metaData[2] = width;
	ret.metaData[3] = height;
	return ret;
}
cv::Scalar MapElement::borderColor = cv::Scalar(191, 199, 202);
cv::Scalar MapElement::brickColor = cv::Scalar(20, 40, 150); 
cv::Scalar MapElement::brickBorderColor = cv::Scalar(39, 127, 255);
void MapElement::draw(cv::Mat& frame,const std::vector<MapElement>& elems)
{
	for (const auto&d: elems)
	{
		if (d.elementType== MapElement::ElementType::Border)
		{
			cv::rectangle(frame,cv::Rect(d.metaData[0], d.metaData[1], d.metaData[2], d.metaData[3]), MapElement::borderColor,-1);
		}
		if (d.elementType == MapElement::ElementType::Brick)
		{
			cv::rectangle(frame, cv::Rect(d.metaData[0], d.metaData[1], d.metaData[2], d.metaData[3]), MapElement::brickColor, -1);
		}
		if (d.elementType == MapElement::ElementType::Brick)
		{
			cv::rectangle(frame, cv::Rect(d.metaData[0], d.metaData[1], d.metaData[2], d.metaData[3]), MapElement::brickBorderColor, 1);
		}
	}
	return;
}
std::list<MapElement> extrctBorderElements(const cv::Mat&borderMap,const float& drawScale)
{
	std::list<MapElement> borderElems;
	const int height = borderMap.rows;
	const int width = borderMap.cols;
	cv::Mat rowsMap = cv::Mat::zeros(borderMap.size(), CV_32SC1);
	cv::Mat colsMap = cv::Mat::zeros(borderMap.size(), CV_32SC1);
	cv::Mat borderMapCopy;
	borderMap.copyTo(borderMapCopy);
	{
		for (size_t r = 0; r < height; r++)
		{
			int stratIdx = 0;
			while (true)
			{
				if (stratIdx>= width)
				{
					break;
				}
				if (borderMapCopy.ptr<uchar>(r)[stratIdx]==0)
				{
					stratIdx += 1;
					continue;
				}
				int endIdx = -1;
				for (size_t c = stratIdx+1; c < width; c++)
				{
					if (borderMapCopy.ptr<uchar>(r)[c] == 0)
					{
						endIdx = c;
						break;
					}
				}
				if (endIdx<0)
				{
					endIdx = width;
				}
				for (size_t c = stratIdx; c < endIdx; c++)
				{
					rowsMap.ptr<int>(r)[c] = endIdx - c;
				}
				stratIdx = endIdx + 1;
			}
		}
	}
	{
		for (size_t c = 0; c < width; c++)
		{
			int stratIdx = 0;
			while (true)
			{
				if (stratIdx >= height)
				{
					break;
				}
				if (borderMapCopy.ptr<uchar>(stratIdx)[c] == 0)
				{
					stratIdx += 1;
					continue;
				}
				int endIdx = -1;
				for (size_t r = stratIdx + 1; r < height; r++)
				{
					if (borderMapCopy.ptr<uchar>(r)[c] == 0)
					{
						endIdx = r;
						break;
					}
				}
				if (endIdx < 0)
				{
					endIdx = height;
				}
				for (size_t r = stratIdx; r < endIdx; r++)
				{
					colsMap.ptr<int>(r)[c] = endIdx - r;
				}
				stratIdx = endIdx + 1;
			}
		}
	}
	while (true)
	{  
		cv::Point colsMapMaxLoc, rowsMapMaxLoc;
		cv::minMaxLoc(rowsMap, 0, 0, 0, &rowsMapMaxLoc);
		cv::minMaxLoc(colsMap, 0, 0, 0, &colsMapMaxLoc);
		const int& rowsMapMaxVal = rowsMap.ptr<int>(rowsMapMaxLoc.y)[rowsMapMaxLoc.x];
		const int& colsMapMaxVal = colsMap.ptr<int>(colsMapMaxLoc.y)[colsMapMaxLoc.x];
		if (rowsMapMaxVal == 0 && colsMapMaxVal == 0)
		{
			return borderElems;
		}
		if (rowsMapMaxVal> colsMapMaxVal)
		{
			borderElems.emplace_back(MapElement::createRectBorder(rowsMapMaxLoc.x * drawScale, rowsMapMaxLoc.y * drawScale, rowsMapMaxVal * drawScale, drawScale));
			for (size_t i = rowsMapMaxLoc.x; i < rowsMapMaxLoc.x + rowsMapMaxVal; i++)
			{
				borderMapCopy.ptr<uchar>(rowsMapMaxLoc.y)[i] = 0;
			}
		}
		else
		{
			borderElems.emplace_back(MapElement::createRectBorder(colsMapMaxLoc.x * drawScale, colsMapMaxLoc.y * drawScale, drawScale, colsMapMaxVal * drawScale));
			for (size_t i = colsMapMaxLoc.y; i < colsMapMaxLoc.y + colsMapMaxVal; i++)
			{
				borderMapCopy.ptr<uchar>(i)[colsMapMaxLoc.x] = 0;
			}
		}

		rowsMap = cv::Mat::zeros(borderMap.size(), CV_32SC1);
		colsMap = cv::Mat::zeros(borderMap.size(), CV_32SC1);
		{
			for (size_t r = 0; r < height; r++)
			{
				int stratIdx = 0;
				while (true)
				{
					if (stratIdx >= width)
					{
						break;
					}
					if (borderMapCopy.ptr<uchar>(r)[stratIdx] == 0)
					{
						stratIdx += 1;
						continue;
					}
					int endIdx = -1;
					for (size_t c = stratIdx + 1; c < width; c++)
					{
						if (borderMapCopy.ptr<uchar>(r)[c] == 0)
						{
							endIdx = c;
							break;
						}
					}
					if (endIdx < 0)
					{
						endIdx = width;
					}
					for (size_t c = stratIdx; c < endIdx; c++)
					{
						rowsMap.ptr<int>(r)[c] = endIdx - c;
					}
					stratIdx = endIdx + 1;
				}
			}
		}
		{
			for (size_t c = 0; c < width; c++)
			{
				int stratIdx = 0;
				while (true)
				{
					if (stratIdx >= height)
					{
						break;
					}
					if (borderMapCopy.ptr<uchar>(stratIdx)[c] == 0)
					{
						stratIdx += 1;
						continue;
					}
					int endIdx = -1;
					for (size_t r = stratIdx + 1; r < height; r++)
					{
						if (borderMapCopy.ptr<uchar>(r)[c] == 0)
						{
							endIdx = r;
							break;
						}
					}
					if (endIdx < 0)
					{
						endIdx = height;
					}
					for (size_t r = stratIdx; r < endIdx; r++)
					{
						colsMap.ptr<int>(r)[c] = endIdx - r;
					}
					stratIdx = endIdx + 1;
				}
			}
		}
	}
	return borderElems;
}
static std::vector<std::string> splitString(const std::string& src, const std::string& symbols, bool repeat)
{
	std::vector<std::string> result;
	int startIdx = 0;
	for (int i = 0; i < src.length(); i++)
	{
		bool isMatch = false;
		for (int j = 0; j < symbols.length(); j++)
		{
			if (src[i] == symbols[j])
			{
				isMatch = true;
				break;
			}
			if (!repeat)
			{
				break;
			}
		}
		if (isMatch)
		{
			std::string sub = src.substr(startIdx, i - startIdx);
			startIdx = i + 1;
			if (sub.length() > 0)
			{
				result.push_back(sub);
			}
		}
		if (i + 1 == src.length())
		{
			std::string sub = src.substr(startIdx, src.length() - startIdx);
			startIdx = i + 1;
			if (sub.length() > 0)
			{
				result.push_back(sub);
			}
		}
	}
	return result;
}
std::vector<MapElement> transfStrToMap(const std::filesystem::path& path, const int& squareSize)
{
	if (!std::filesystem::exists(path))
	{
		LOG_ERR_OUT << "path not exists: " << path;
		return std::vector<MapElement>();
	}
	std::fstream fin(path,std::ios::in);
	std::list<std::vector<std::uint32_t>> mapMetaData;
	int lineHasElementCnt = -1;
	std::string aline;
	int lineNum = 1;
	while (std::getline(fin,aline))
	{
		std::vector<std::string> segs = splitString(aline,",",true);
		if (lineHasElementCnt<0)
		{
			lineHasElementCnt = segs.size();
		}
		else if(lineHasElementCnt != segs.size())
		{
			LOG_ERR_OUT << "lineNum = " << lineNum<<"; lineHasElementCnt != segs.size()";
			return std::vector<MapElement>();
		}
		std::vector<std::uint32_t>lineMetaData(lineHasElementCnt);
		for (int i = 0; i < lineHasElementCnt; i++)
		{
			std::stringstream ss;
			ss << segs[i];
			int mapTempInt = 0;
			ss >> mapTempInt;
			if (mapTempInt==-1)
			{
				lineMetaData[i] = std::numeric_limits<std::int32_t>::max();
			}
			else if (mapTempInt==-2)//entry
			{
				lineMetaData[i] = std::numeric_limits<std::int32_t>::max()-1;
			}
			else
			{
				lineMetaData[i] = mapTempInt;
			}
		}
		mapMetaData.emplace_back(lineMetaData);
		lineNum += 1;
	}
	std::vector<MapElement> elems;
	elems.reserve(lineHasElementCnt* lineHasElementCnt);
	float drawScale = 1.f * squareSize / lineHasElementCnt;
	cv::Mat borderMap = cv::Mat::zeros(mapMetaData.size(), lineHasElementCnt, CV_8UC1);
	lineNum = 0;
	for (const auto&d: mapMetaData)
	{
		for (int i = 0; i < lineHasElementCnt; i++)
		{
			if (d[i]== std::numeric_limits<std::int32_t>::max())
			{
				borderMap.ptr<uchar>(lineNum)[i] = 1;
			}
			else if (d[i] == 0)
			{
				continue;
			}
			else if (d[i] == std::numeric_limits<std::int32_t>::max()-1)
			{
				elems.emplace_back(MapElement::createEntry(i * drawScale, lineNum * drawScale));
				
			}
			else
			{
				elems.emplace_back(MapElement::createRectBrick(i * drawScale, lineNum * drawScale, drawScale, drawScale, d[i]));
			}
		}
		lineNum+=1;
	}	 
	std::list<MapElement> borderElems = extrctBorderElements(borderMap, drawScale);
	for (auto&d: borderElems)
	{
		elems.emplace_back(d);
	}   
	return elems;
}
cv::Mat generBorderPic(const int& squareSize)
{
	cv::Mat pic = cv::Mat::zeros(squareSize, squareSize, CV_8UC3);
	//cv::ellipse(pic, cv::Point(200, 200), cv::Point(5, 2), 45, 0, 360, cv::Scalar(255, 255, 255), -1);
	pic.setTo(cv::Scalar(191, 199, 202));
	for (size_t r = 0; r < squareSize; r++)
	{
		for (size_t c = 0; c < squareSize; c++)
		{

		}
	}
	return pic;
}
cv::Mat generBrickPic(const int& squareSize)
{
	return cv::Mat();
}