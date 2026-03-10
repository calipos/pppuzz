#include <functional>
#include <vector>
#include "log.h"
#include "frame.h"
#include "floater.h"
constexpr std::uint32_t spread16(const std::uint16_t& x) {
	uint32_t x32 = x;
	x32 = (x32 | (x32 << 8)) & 0x00FF00FF;
	x32 = (x32 | (x32 << 4)) & 0x0F0F0F0F;
	x32 = (x32 | (x32 << 2)) & 0x33333333;
	x32 = (x32 | (x32 << 1)) & 0x55555555;
	return x32;
}
constexpr std::uint16_t compact16(const std::uint32_t& x) {
	std::uint32_t x32 = x & 0x55555555;
	x32 = (x32 | (x32 >> 1)) & 0x33333333;
	x32 = (x32 | (x32 >> 2)) & 0x0F0F0F0F;
	x32 = (x32 | (x32 >> 4)) & 0x00FF00FF;
	x32 = (x32 | (x32 >> 8)) & 0x0000FFFF;
	return static_cast<uint16_t>(x32);
}
constexpr std::uint32_t morton_encode_16_32(const std::uint32_t& x, const std::uint32_t& y) {
	return spread16(x) | (spread16(y) << 1);
}
void morton_decode_16_32(const std::uint32_t& code, uint16_t& x, uint16_t& y) {
	x = compact16(code);
	y = compact16(code >> 1);
}
auto generMortonCode(const std::uint16_t& height, const std::uint16_t& width) {
	std::vector<std::vector<std::uint32_t>> mat(height, std::vector<std::uint32_t>(width));
	for (std::uint16_t r = 0; r < height; ++r) {
		for (std::uint16_t c = 0; c < width; ++c) {
			mat[r][c] = morton_encode_16_32(c, r);;
		}
	}
	return mat;
}
Frame::Frame() {
	frame = cv::Mat::zeros(IMG_HEIGHT, IMG_WIDTH, CV_8UC3);

	generNeibghourGrids(GRID_HEIGHT, GRID_WIDTH, neibghourGrids, neibghourGridSize);
	MortonCode = generMortonCode(IMG_HEIGHT, IMG_WIDTH);
} 

void Frame::generNeibghourGrids(const std::uint32_t& gridHeight, const std::uint32_t& gridWidth, std::vector<std::uint32_t>& neibghourGrids, std::vector<std::uint8_t>& neibghourGridSize)
{
	neibghourGrids.resize(gridHeight * gridWidth * 8);
	neibghourGridSize.resize(gridHeight * gridWidth);
	std::uint32_t gridHeight_1 = gridHeight - 1;
	std::uint32_t gridWidth_1 = gridWidth - 1;
	for (int r = 0; r < gridHeight; r++)
	{
		for (int c = 0; c < gridWidth; c++)
		{
			std::uint32_t thisGridIdx = r * gridWidth + c;
			std::uint8_t& thisNeighborCnt = neibghourGridSize[thisGridIdx];
			std::uint32_t* neightborIdx = &neibghourGrids[thisGridIdx * 8];
			thisNeighborCnt = 0;
			if (r > 0)
			{
				neightborIdx[thisNeighborCnt] = thisGridIdx - gridWidth;
				thisNeighborCnt += 1;
				if (c > 0)
				{
					neightborIdx[thisNeighborCnt] = thisGridIdx - gridWidth - 1;
					thisNeighborCnt += 1;
				}
				if (c < gridWidth_1)
				{
					neightborIdx[thisNeighborCnt] = thisGridIdx - gridWidth + 1;
					thisNeighborCnt += 1;
				}
			}
			if (c > 0)
			{
				neightborIdx[thisNeighborCnt] = thisGridIdx - 1;
				thisNeighborCnt += 1;
			}
			if (c < gridWidth_1)
			{
				neightborIdx[thisNeighborCnt] = thisGridIdx + 1;
				thisNeighborCnt += 1;
			}
			if (r < gridHeight_1)
			{
				neightborIdx[thisNeighborCnt] = thisGridIdx + gridWidth;
				thisNeighborCnt += 1;
				if (c > 0)
				{
					neightborIdx[thisNeighborCnt] = thisGridIdx + gridWidth - 1;
					thisNeighborCnt += 1;
				}
				if (c < gridWidth_1)
				{
					neightborIdx[thisNeighborCnt] = thisGridIdx + gridWidth + 1;
					thisNeighborCnt += 1;
				}
			}
		}
	}
	return;
}

void Frame::upDate(std::vector<Ball>& balls, std::vector<MapElement>& mapElement, Floater& controlFloater, const float& period)
{
	controlFloater.postionX += controlFloater.velocity * period;
	std::vector<std::pair<std::uint32_t, std::uint32_t>>posHash(balls.size());
	for (int i = 0; i < balls.size(); i++)
	{
		balls[i].position.x += balls[i].velocity.x * period;
		balls[i].position.y += balls[i].velocity.y * period;
		//border reflect here    ------------------------------------------
		{
			if (balls[i].position.y < 0)
			{
				balls[i].position.y = 0;
				balls[i].velocity.y *= -1;
			}
			if (balls[i].position.x < 0)
			{
				balls[i].position.x = 0;
				balls[i].velocity.x *= -1;
			}
			if (balls[i].position.x >= IMG_WIDTH)
			{
				balls[i].position.x = IMG_WIDTH - 0.1;
				balls[i].velocity.x *= -1;
			}
		}
		//border reflect here    ==========================================
		int hashPosX = std::floor(balls[i].position.x);
		int hashPosY = std::floor(balls[i].position.y);
		if (hashPosX < 0 || hashPosX >= IMG_WIDTH || hashPosY < 0 || hashPosY >= IMG_HEIGHT)
		{
			posHash[i].first = INVALID_HASH;
			posHash[i].second = i;
			balls[i].gridPos = INVALID_HASH;
		}
		else
		{
			posHash[i].first = MortonCode[hashPosY][hashPosX];
			posHash[i].second = i;
			int gridPosX = std::floor(balls[i].position.x / GRID_UNIT_SIZE);
			int gridPosY = std::floor(balls[i].position.y / GRID_UNIT_SIZE);
			balls[i].gridPos = gridPosX + gridPosY * GRID_WIDTH;
		}
	}
	std::sort(posHash.begin(), posHash.end(), [](const auto& a, const auto& b) {return a.first < b.first; });
	std::vector<Ball> balls_sorted;
	balls_sorted.reserve(balls.size());
	for (int i = 0; i < posHash.size(); i++)
	{
		if (posHash[i].first == INVALID_HASH)
		{
			break;
		}
		balls_sorted.emplace_back(balls[posHash[i].second]);
	}
	balls = balls_sorted;
	std::vector<std::uint32_t>grid_idx_start(GRID_CNT, INVALID_HASH);
	std::vector<std::uint32_t>grid_idx_end(GRID_CNT, 0);
	for (std::uint32_t i = 0; i < balls.size(); i++)
	{
		const std::int32_t& gridPos = balls[i].gridPos;
		if (gridPos != INVALID_HASH)
		{
			if (grid_idx_start[gridPos] == INVALID_HASH)
			{
				grid_idx_start[gridPos] = i;
				grid_idx_end[gridPos] = i + 1;
			}
			else
			{
				grid_idx_end[gridPos] = i + 1;
			}
		}
	}
	for (std::uint32_t i = 0; i < balls.size(); i++)
	{
		const std::int32_t& gridPos = balls[i].gridPos;
		const std::uint8_t& neightCnt = neibghourGridSize[gridPos];
		const std::uint32_t const* neighbors = (const std::uint32_t const*)&neibghourGrids[gridPos * 8];
		for (std::uint8_t neighbor_i = 0; neighbor_i < neightCnt; neighbor_i++)
		{
			std::uint32_t startIdx = grid_idx_start[neighbors[neighbor_i]];
			if (startIdx == INVALID_HASH)
			{
				continue;
			}
			std::uint32_t endIdx = grid_idx_end[neighbors[neighbor_i]];
			for (std::uint32_t j = startIdx; j < endIdx && startIdx != INVALID_HASH; j++)
			{
				if (i == j)
				{
					continue;
				}
				cv::Point2f relPos = balls[i].position - balls[j].position;
				float dist = cv::norm(relPos);
				if (dist < GRID_UNIT_SIZE)
				{
					balls[i].velocity += relPos;
					//if (balls[i].velocity.x > MAX_VELOCITY)balls[i].velocity.x = MAX_VELOCITY;
					//if (balls[i].velocity.y > MAX_VELOCITY)balls[i].velocity.y = MAX_VELOCITY;
					//if (balls[i].velocity.x <-MAX_VELOCITY)balls[i].velocity.x = -MAX_VELOCITY;
					//if (balls[i].velocity.y <-MAX_VELOCITY)balls[i].velocity.y = -MAX_VELOCITY;
				}
			}
		}
		//int gridPosX = gridPosGRID_UNIT_SIZE);
		//int gridPosY = std::floor(balls[i].position.y / GRID_UNIT_SIZE);
	}
	return;
}


int Frame::run(const int& frameCnt,const bool& saveToMp4)
{ 


	cv::VideoWriter* mp4Writer = nullptr;
	if (saveToMp4)
	{
		int fourcc = cv::VideoWriter::fourcc('m', 'p', '4', 'v');
		std::string output_video = "output.mp4";           // 输出视频文件名
		double fps = 90;
		mp4Writer = new cv::VideoWriter(output_video, fourcc, fps, cv::Size(IMG_WIDTH, IMG_HEIGHT));
		if (!mp4Writer->isOpened()) {
			LOG_ERR_OUT << "错误：无法创建视频文件！请检查编码器支持或路径权限。" << std::endl;
			return -1;
		}
	}
	 

	Floater controlFloater;
	std::vector<Ball>balls;
	balls.reserve(2048);
	std::vector<MapElement>map1 = transfStrToMap("1.map", IMG_WIDTH);
	controlFloater.setEntry(map1);

	for (size_t i = 0; i < frameCnt; i++)
	{
		if (balls.size()==0)
		{
			balls.emplace_back(Ball::generAnewFallBall(new_fall_ball_pos_x, new_fall_ball_pos_y, new_fall_ball_velocity));			
		}
		controlFloater.figureScore(balls);
		Frame::upDate(balls, map1, controlFloater, 0.8);
		cv::Mat currframe;
		frame.copyTo(currframe);
		MapElement::draw(currframe, map1);
		Ball::draw(currframe, balls);
		Floater::draw(frame, controlFloater);
		if (saveToMp4)
		{
			mp4Writer->write(currframe);
		}
		else
		{
			cv::imshow("1", currframe);
			cv::waitKey(15);
		}
		LOG_OUT << i << "/" << frameCnt;

	}
	mp4Writer->release();
	return 0;
}