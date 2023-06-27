#pragma once

#define GET_CONTEXT(data, context) \
    std::shared_ptr<vkData> data = std::static_pointer_cast<vkData>(context->ApiContextData); \
