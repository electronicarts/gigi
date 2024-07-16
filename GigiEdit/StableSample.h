///////////////////////////////////////////////////////////////////////////////
//         Gigi Rapid Graphics Prototyping and Code Generation Suite         //
//        Copyright (c) 2024 Electronic Arts Inc. All rights reserved.       //
///////////////////////////////////////////////////////////////////////////////

#pragma once
#include <algorithm>	// std::min()

class StableSample {
public:
	// @return FLT_MAX if not set yet
	float getStableAverage() const { return stableAverage; }
	// @return FLT_MAX if not set yet
	float getStableMin() const { return stableMin; }
	// @return FLT_MAX if not set yet
	float getStableMax() const { return stableMax; }

	// call every frame
	// @param deltaTime in seconds
	void tick(const float value, const float deltaTime) {
		currentValue = value;
		currentSum += value;
		currentMin = std::min(currentMin, value);
		currentMax = std::max(currentMax, value);

		runningTime += deltaTime;
		++runningCount;

		if (runningCount >= countThreshold || (runningTime >= timeThreshold && runningCount > 0))
		{
			stableAverage = currentSum / runningCount;
			stableMin = currentMin;
			stableMax = currentMax;
			runningCount = 0;
			currentSum = {};
			currentMin = FLT_MAX;
			currentMax = -FLT_MAX;
			runningTime = 0;
		}
	}

	// not stable
	// @return FLT_MAX if not set yet
	float getCurrentValue() const { return currentValue; }

private:
	// most recent non stable value
	float currentValue{ FLT_MAX };

	// user settings, todo: expose
	int countThreshold{ 100 };
	float timeThreshold{ 0.2f };

	// stable values
	float stableAverage{ FLT_MAX };
	float stableMin{ FLT_MAX };
	float stableMax{ FLT_MAX };

	// intermediates over <count> ticks or over time
	float currentSum{ FLT_MAX };
	float currentMin{ FLT_MAX };
	float currentMax{ FLT_MAX };
	int runningCount{};
	// in seconds
	float runningTime{};
};

