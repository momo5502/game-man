#pragma once
#include "utils/binary_buffer.hpp"

class serializable
{
public:
	virtual ~serializable() = default;
	virtual void serialize(utils::binary_buffer& buffer) = 0;
};
