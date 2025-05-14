#include "utility.h"
#include <vector>

bool is_Printable(uint8_t c)
{
  return (c >= 0x20 && c <= 0x7E);
}

void print_data(uint8_t data)
{
  if (is_Printable(data))
  {
    Serial.print((char)data);
  }
  else
  {
    Serial.print("[");
    if (data < 0x10)
      Serial.print("0");
    Serial.print(data, HEX);
    Serial.print("]");
  }
}

void print_data(const void *data, size_t len)
{
  const uint8_t *bytes = (const uint8_t *)data;
  for (size_t i = 0; i < len; i++)
  {
    print_data(bytes[i]);
  }
  Serial.println();
}

void print_hex(uint8_t data)
{
  if (data < 0x10)
  {
    Serial.print("0");
  }
  Serial.print(data, HEX);
}

void print_hex(const void *data, size_t len)
{
  uint8_t *_data = (uint8_t *)data;
  for (size_t i = 0; i < len; i++)
  {
    print_hex(_data[i]);
    Serial.print(" ");
  }
  Serial.println();
}

void swap_high_low_bytes(uint16_t *a)
{
  uint16_t temp = *a;
  *a = temp >> 8 | temp << 8;
}

void print_reverse_endian(uint16_t a)
{
  // Serial.println("---------");
  // Serial.println(a);
  swap_high_low_bytes(&a);
  Serial.println(a);
  // Serial.println("---------");
}

static const size_t MAX_VECTOR_BYTES = 256; // Define the maximum byte size
static std::vector<uint8_t> data_queue;

RTC_DATA_ATTR uint8_t vector_buffer[MAX_VECTOR_BYTES];
RTC_DATA_ATTR size_t vector_buffer_size = 0; // Store actual data size

void vector_to_buffer()
{
  if (!data_queue.empty())
  {
    vector_buffer_size = data_queue.size();
    std::copy(data_queue.begin(), data_queue.end(), vector_buffer);

    // Clear remaining buffer space to prevent old data issues
    std::fill(vector_buffer + vector_buffer_size, vector_buffer + MAX_VECTOR_BYTES, 0);
  }
  else
  {
    vector_buffer_size = 0; // Ensure no stale data remains
  }
}

void buffer_to_vector()
{
  if (data_queue.empty() && vector_buffer_size > 0)
  {
    data_queue.insert(data_queue.end(), vector_buffer, vector_buffer + vector_buffer_size);
  }
}

void put_data_to_vector(uint8_t *data, size_t len)
{
  if (data && len > 0)
  {
    // Ensure the total bytes do not exceed MAX_VECTOR_BYTES
    if (data_queue.size() + len > MAX_VECTOR_BYTES)
    {
      size_t excess = (data_queue.size() + len) - MAX_VECTOR_BYTES;
      data_queue.erase(data_queue.begin(), data_queue.begin() + excess); // Remove oldest bytes
    }

    data_queue.insert(data_queue.end(), data, data + len);
  }
}

size_t get_data_from_vector(uint8_t *data, size_t len)
{
  size_t data_size = std::min(len, data_queue.size());
  if (data_size > 0)
  {
    std::copy(data_queue.begin(), data_queue.begin() + data_size, data);
    data_queue.erase(data_queue.begin(), data_queue.begin() + data_size);
  }
  return data_size;
}

bool is_vector_empty()
{
  return data_queue.empty();
}

size_t get_vector_size()
{
  return data_queue.size(); // Returns the total byte size
}

void print_top_vector_data()
{
  if (!data_queue.empty())
  {
    Serial.print("Top data in vector: ");
    print_hex(data_queue.data(), data_queue.size());
  }
}