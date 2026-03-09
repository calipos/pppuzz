#ifndef __MAP_EDIT_H__
#define __MAP_EDIT_H__
#include <numeric>
#include <vector>
#include <filesystem>
struct MapElement
{
	enum class ElementType
	{
		Brick = 1,
		Border = 2,
	};
	enum class Shape
	{
		Rectangle = 1, 
		Circle = 2,
	};
	std::uint32_t health;
	Shape shape;
	ElementType elementType;
	std::vector<float>metaData;
	static MapElement createRectBorder(const float& leftUpX, const float& leftUpY, const float& rightDownX, const float& rightDownY);
	static MapElement createRectBrick(const float& leftUpX, const float& leftUpY, const float& rightDownX, const float& rightDownY, const int& health = 1);
	static void draw(cv::Mat& frame, const std::vector<MapElement>& elems);
	static cv::Scalar borderColor;
	static cv::Scalar brickColor;
	static cv::Scalar brickBorderColor;
};

std::vector<MapElement> transfStrToMap(const std::filesystem::path& path, const int& squareSize);
cv::Mat generBorderPic(const int& squareSize);
cv::Mat generBrickPic(const int& squareSize);
#endif // !__MAP_EDIT_H__
