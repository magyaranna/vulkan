#pragma once

#include "vulkan/vulkan.h"
#include "device.h"

namespace v {


	class TS_query {
	private:

		Device& device;

		VkQueryPool queryPool;

		uint64_t timeStamps[2] = { 1, 1 };
		float timeStampPeriod;

		

		void setupQueryPool();

	public:

		TS_query(Device& device);
		~TS_query();

		std::vector<float> deltatimes;

		void resetQueryPool(VkCommandBuffer cmd);
		void writeTimeStamp(VkCommandBuffer cmd, uint64_t index, VkPipelineStageFlagBits flag);

		void getQueryResults();

	};

}

