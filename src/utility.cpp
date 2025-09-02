#include "utility.h"
#include <vector>

/**
 * Check if a byte is a printable ASCII character.
 *
 * @param c The byte to check.
 * @return true if the byte is a printable ASCII character (range 0x20 to 0x7E), false otherwise.
 */

bool is_Printable(uint8_t c)
{
  return (c >= 0x20 && c <= 0x7E);
}

/**
 * Print a single byte of data to the serial console in a human-readable format.
 *
 * If the byte is a printable ASCII character, it is printed directly. Otherwise,
 * it is printed in the format "[XX]" where XX is the byte value in hexadecimal.
 */
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

/**
 * Print a buffer of data to the serial console in a human-readable format.
 *
 * Each byte in the buffer is checked if it's a printable ASCII character using
 * the is_Printable function. If it is, the character is printed directly.
 * Otherwise, it is printed in the format "[XX]" where XX is the byte value
 * in hexadecimal. Each byte is printed in sequence followed by a newline.
 *
 * @param data A pointer to the buffer of data to be printed.
 * @param len The number of bytes in the data buffer.
 */

void print_data(const void *data, size_t len)
{
  const uint8_t *bytes = (const uint8_t *)data;
  for (size_t i = 0; i < len; i++)
  {
    print_data(bytes[i]);
  }
  Serial.println();
}

/**
 * Print a single byte of data as a hexadecimal value.
 *
 * If the byte's value is less than 0x10, a leading zero is printed.
 *
 * @param data The byte to be printed.
 */
void print_hex(uint8_t data)
{
  if (data < 0x10)
  {
    Serial.print("0");
  }
  Serial.print(data, HEX);
}

/**
 * Print a buffer of data in hexadecimal format.
 *
 * Each byte in the buffer is printed as a two-digit hexadecimal number.
 * Bytes are separated by a space. The output is followed by a newline.
 *
 * @param data A pointer to the buffer of data to be printed.
 * @param len The number of bytes in the data buffer.
 */

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

/**
 * Swap the high and low order bytes of a 16-bit word.
 *
 * The input to this function is a pointer to a uint16_t, which is the
 * 16-bit word to be swapped. The function operates in-place, meaning
 * that the input word is modified directly.
 *
 * @param a A pointer to the 16-bit word to be swapped.
 */

uint16_t swap_high_low_bytes(uint16_t a)
{
  return (a >> 8) | (a << 8);
}

void swap_high_low_bytes(uint16_t *a)
{
  *a = swap_high_low_bytes(*a);
}

/**
 * Print a 16-bit word in reverse byte order.
 *
 * The input to this function is a 16-bit word, which is swapped in-place
 * by calling swap_high_low_bytes. The swapped word is then printed to the
 * serial port.
 *
 * @param a The 16-bit word to be printed.
 */
void print_reverse_endian(uint16_t a)
{
  swap_high_low_bytes(&a);
  Serial.println(a);
}

/**
 * @brief Calculate CRC16 for error checking
 * @param data Pointer to the data
 * @param length Length of the data
 * @return Calculated CRC16 value
 */
uint16_t calculateCRC(const uint8_t *data, size_t length)
{
  uint16_t crc = 0xFFFF;
  for (size_t i = 0; i < length; i++)
  {
    crc ^= data[i];
    for (uint8_t j = 0; j < 8; j++)
    {
      if (crc & 1)
      {
        crc = (crc >> 1) ^ 0xA001;
      }
      else
      {
        crc >>= 1;
      }
    }
  }
  return crc;
}

/**
 * @brief Calculate CRC16 for error checking and store result in result_crc
 *
 * This function is a wrapper around calculateCRC that takes a pointer to a
 * uint16_t where the result is stored.
 *
 * @param data Pointer to the data
 * @param length Length of the data
 * @param result_crc Pointer to a uint16_t where the result is stored
 */
void calculateCRC(const uint8_t *data, size_t length, uint16_t *result_crc)
{
  *result_crc = calculateCRC(data, length);
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