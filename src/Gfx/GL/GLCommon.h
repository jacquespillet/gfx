#pragma once

#define GET_CONTEXT(data, context) \
    std::shared_ptr<glData> data = std::static_pointer_cast<glData>(context->ApiContextData); \
