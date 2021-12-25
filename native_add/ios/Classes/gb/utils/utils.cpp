#include "../std_include.hpp"
#include <cstdarg>

#define VA_BUFFER_COUNT		4
#define VA_BUFFER_SIZE		65536

namespace utils
{
	const char* va(const char* fmt, ...)
	{
		static char va_buffer[VA_BUFFER_COUNT][VA_BUFFER_SIZE];
		static int va_next_buffer_index = 0;

		va_list ap;
		va_start(ap, fmt);
		char* dest = va_buffer[va_next_buffer_index];
		vsnprintf(va_buffer[va_next_buffer_index], VA_BUFFER_SIZE, fmt, ap);
		va_next_buffer_index = (va_next_buffer_index + 1) % VA_BUFFER_COUNT;
		va_end(ap);
		return dest;
	}

	bool read_file(const std::string& name, std::vector<uint8_t>& data)
	{
		std::ifstream stream(name, std::ios::binary);
		if (stream.is_open())
		{
			stream.seekg(0, std::ios::end);
			const std::streampos size = stream.tellg();
			stream.seekg(0, std::ios::beg);

			if (size > -1)
			{
				data.clear();
				data.resize(size_t(size));

				stream.read(reinterpret_cast<char*>(data.data()), size);
				return true;
			}
		}

		return false;
	}
}
