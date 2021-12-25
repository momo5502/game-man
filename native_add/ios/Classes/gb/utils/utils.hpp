#pragma once

#define zero_object(x) memset(&(x), 0, sizeof(x))
#define array_size(x) (sizeof(x) / sizeof(x[0]))

namespace utils
{
	const char* va(const char* fmt, ...);
	bool read_file(const std::string& name, std::vector<uint8_t>& data);

	template <typename T>
	void merge(std::vector<T>* target, T* source, const size_t length)
	{
		if (source)
		{
			for (size_t i = 0; i < length; ++i)
			{
				target->push_back(source[i]);
			}
		}
	}

	template <typename T>
	void merge(std::vector<T>* target, std::vector<T> source)
	{
		for (auto& entry : source)
		{
			target->push_back(entry);
		}
	}
}
