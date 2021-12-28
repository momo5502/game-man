#pragma once
#include <string>
#include <vector>
#include <stdexcept>

namespace utils {
class binary_buffer {
public:
  binary_buffer(std::string buffer) : write_(false),
                                      buffer_(std::move(buffer))
  {
  }

  binary_buffer(const std::vector<uint8_t>& buffer)
    : binary_buffer(std::string(reinterpret_cast<const char*>(buffer.data()), buffer.size()))
  {
  }

  binary_buffer() : write_(true)
  {
  }

  template <typename T>
  void handle(T& obj)
  {
    this->handle(&obj, sizeof(obj));
  }

  template <typename T>
  void handle(std::vector<T>& vector)
  {
    uint32_t size = static_cast<uint32_t>(vector.size());
    this->handle(size);

    vector.resize(size);
    this->handle(vector.data(), size);
  }

  const std::string& get_buffer() const
  {
    return this->buffer_;
  }

private:
  bool write_;
  size_t pos{0};
  std::string buffer_{};

  void handle(void* data, const size_t size)
  {
    if (write_) {
      buffer_.append(reinterpret_cast<const char*>(data), size);
    }
    else {
      if (pos + size > this->buffer_.size()) {
        throw std::runtime_error("OOB read");
      }

      memcpy(data, this->buffer_.data() + pos, size);
      pos += size;
    }
  }
};
}
