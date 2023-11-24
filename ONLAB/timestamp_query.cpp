

#include "timestamp_query.h"

namespace v {

	TS_query::TS_query(Device& device): device(device) {
		setupQueryPool();

		VkPhysicalDeviceProperties physicalDeviceProperties;
		vkGetPhysicalDeviceProperties(device.getPhysicalDevice(), &physicalDeviceProperties);
	    timeStampPeriod = physicalDeviceProperties.limits.timestampPeriod;

	}

	TS_query::~TS_query() {

		vkDestroyQueryPool(device.getLogicalDevice(), queryPool, nullptr);
	}


	void TS_query::setupQueryPool()
	{
		VkQueryPoolCreateInfo queryPoolInfo = {};
		queryPoolInfo.sType = VK_STRUCTURE_TYPE_QUERY_POOL_CREATE_INFO;
		queryPoolInfo.queryType = VK_QUERY_TYPE_TIMESTAMP;
		queryPoolInfo.queryCount = 2;
		if (vkCreateQueryPool(device.getLogicalDevice(), &queryPoolInfo, NULL, &queryPool) != VK_SUCCESS) {
			throw std::runtime_error("failed to create ");
		}

	}

	void TS_query::resetQueryPool(VkCommandBuffer cmd) {

		// Must be done outside of render pass
		vkCmdResetQueryPool(cmd, queryPool, 0, 2);
	}

	void TS_query::writeTimeStamp(VkCommandBuffer cmd, uint64_t index, VkPipelineStageFlagBits flag ) {
		vkCmdWriteTimestamp(cmd, flag, queryPool, index);

	}


	void TS_query::getQueryResults()
	{

		vkGetQueryPoolResults(device.getLogicalDevice(), queryPool, 0, 2, sizeof(timeStamps), timeStamps, sizeof(std::uint64_t), VK_QUERY_RESULT_64_BIT);
		
		float delta_in_ms = float(timeStamps[1] - timeStamps[0]) * timeStampPeriod / 1000000.0f;
		deltatimes.push_back(delta_in_ms);
	}
	

}