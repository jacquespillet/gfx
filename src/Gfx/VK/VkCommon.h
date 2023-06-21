#pragma once

#define GET_CONTEXT(data, context) \
    vkData *data = (vkData*)context->ApiContextData; \
