#ifndef __FRAME_DEFINE_H__
#define __FRAME_DEFINE_H__
#include <numeric>
#include <vector>
#include <thread>
#include <condition_variable>
#include <mutex>
#include "opencv2/opencv.hpp"
#include "ball.h"
#include "mapEdit.h"
constexpr float IMG_SCALE_FACTOR = 1;
constexpr std::uint32_t IMG_HEIGHT = 1366 * IMG_SCALE_FACTOR;// 480;
constexpr std::uint32_t IMG_WIDTH = 768 * IMG_SCALE_FACTOR;// 360;
constexpr std::uint32_t INVALID_HASH = 0x0fffffff;
constexpr std::uint32_t BALL_RADIUS = 3 * IMG_SCALE_FACTOR;
constexpr std::uint32_t GRID_UNIT_SIZE = 2 * BALL_RADIUS;
constexpr std::uint32_t GRID_HEIGHT = (IMG_HEIGHT % GRID_UNIT_SIZE == 0) ? (IMG_HEIGHT / GRID_UNIT_SIZE) : (IMG_HEIGHT / GRID_UNIT_SIZE + 1);
constexpr std::uint32_t GRID_WIDTH = (IMG_WIDTH % GRID_UNIT_SIZE == 0) ? (IMG_WIDTH / GRID_UNIT_SIZE) : (IMG_WIDTH / GRID_UNIT_SIZE + 1);
constexpr std::uint32_t GRID_CNT = GRID_HEIGHT * GRID_WIDTH;
constexpr std::int32_t MAX_VELOCITY = 1;
constexpr std::int32_t MIN_VELOCITY = 0.5;
constexpr std::int32_t MAX_ANGULAR_VELOCITY = 3;
constexpr std::int32_t new_fall_ball_pos_x = IMG_WIDTH / 2;
constexpr std::int32_t new_fall_ball_pos_y = IMG_WIDTH + (IMG_HEIGHT - IMG_WIDTH) * 0.382;
constexpr float new_fall_ball_velocity = 2;
struct Frame
{
	Frame(); 
	std::vector<std::uint32_t> neibghourGrids;
	std::vector<std::uint8_t> neibghourGridSize;
	void generNeibghourGrids(const std::uint32_t& gridHeight, const std::uint32_t& gridWidth, std::vector<std::uint32_t>& neibghourGrids, std::vector<std::uint8_t>& neibghourGridSize);
	void upDate(std::vector<Ball>& balls, std::vector<MapElement>& mapElement, const float& period);
	cv::Mat frame;
	std::vector<std::vector<std::uint32_t>>  MortonCode;
	int run(const int& frameCnt, const bool& saveToMp4);




#define conditionErrorCode(conditionName) returncode_##conditionName
#define conditionReadyFlag(conditionName) ready_for_##conditionName
#define conditionReturnCheck(conditionName) disable_ready_for_##conditionName
#define BIND_FUNC_NAME(conditionName) _##conditionName##_bindActiveFuncName
#define disableConditionReturnCheck(conditionName) conditionReturnCheck (conditionName) = true;
#define enableConditionReturnCheck(conditionName) conditionReturnCheck (conditionName) = false;
#define RegisterCondition(conditionName,activeThreadFunc) \
	static int conditionErrorCode(conditionName)=0; \
	static bool conditionReadyFlag(conditionName)=false; \
	static bool conditionReturnCheck(conditionName) = false; \
	static std::string  BIND_FUNC_NAME(conditionName)=#activeThreadFunc;
	static std::mutex syncMtx;
	static std::condition_variable syncCv;
#define OthersWaitMe(conditionName) 								 \
{																	 \
	std::string bindCheck = BIND_FUNC_NAME(conditionName);			 \
	if (bindCheck.compare(__func__) != 0) {							 \
		LOG_OUT << "NOT MATCH";										 \
		exit(-1);													 \
	}																 \
	if (conditionReadyFlag(conditionName))							 \
	{																 \
		LOG_OUT << "ready flag must be false first.";				 \
		exit(-1);													 \
	}																 \
	{																 \
		std::lock_guard<std::mutex> lock(syncMtx);					 \
		conditionReadyFlag(conditionName) = true;					 \
		if (conditionReturnCheck(conditionName))					 \
		{															 \
			LOG_OUT<<"disableConditionReturnCheck: "<<#conditionName;\
			conditionErrorCode(conditionName)=0;					 \
		}															 \
		syncCv.notify_all();										 \
		LOG_OUT << "[ "<< __func__ <<" ] ready to notify";			 \
	}																 \
}
#define OthersWaitMeLambda(conditionName) 				             \
{																	 \
	std::string bindCheck = BIND_FUNC_NAME(conditionName);			 \
	if (bindCheck.compare(lambdaName) != 0) {						 \
		LOG_OUT << "NOT MATCH";										 \
		exit(-1);													 \
	}																 \
	if (conditionReadyFlag(conditionName))							 \
	{																 \
		LOG_OUT << "ready flag must be false first.";				 \
		exit(-1);													 \
	}																 \
	{																 \
		std::lock_guard<std::mutex> lock(syncMtx);					 \
		conditionReadyFlag(conditionName) = true;					 \
		if (conditionReturnCheck(conditionName))					 \
		{															 \
			LOG_OUT<<"disableConditionReturnCheck: "<<#conditionName;\
			conditionErrorCode(conditionName)=0;					 \
		}															 \
		syncCv.notify_all();										 \
		LOG_OUT << "[ "<< lambdaName <<" ] notify=>"<<#conditionName;\
	}																 \
}	
#define WaitSomeone(conditionName)																	   \
{																									   \
	std::unique_lock<std::mutex> lock(syncMtx);														   \
	while (!ready_for_##conditionName)																   \
	{																								   \
		LOG_OUT << "[ " << __func__ << " ] wait ...";												   \
		syncCv.wait(lock);																			   \
		if (ready_for_##conditionName)																   \
		{																							   \
			break;																					   \
		}																							   \
	}																								   \
	LOG_OUT << "[ " << __func__ << " ] awake";													       \
	if (conditionErrorCode(conditionName) != 0)														   \
	{																								   \
		LOG_OUT << "[ " << __func__ << " ] get a errorCode notify:"<<conditionErrorCode(conditionName);\
	}																								   \
}
#define WaitSomeoneLambda(conditionName)															   \
{																									   \
	std::unique_lock<std::mutex> lock(syncMtx);														   \
	while (!ready_for_##conditionName)																   \
	{																								   \
		LOG_OUT << "[ " << lambdaName << " ] wait ..."<<#conditionName;								   \
		syncCv.wait(lock);																			   \
		if (ready_for_##conditionName)																   \
		{																							   \
			break;																					   \
		}																							   \
	}																								   \
	LOG_OUT << "[ " << lambdaName << " ] awake from "<<#conditionName;							       \
	if (conditionErrorCode(conditionName) != 0)														   \
	{																								   \
		LOG_OUT << "[ " << lambdaName << " ] get a errorCode notify:"<<conditionErrorCode(conditionName);\
	}																								   \
}

};

#endif // !_FRAME_DEFINE_H__
