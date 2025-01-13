#ifndef BASE64_H
#define BASE64_H

#include <Arduino.h>

class Base64
{
public:
  // Calculate the length of the encoded string
  static size_t encodedLength(size_t inputLength)
  {
    return ((inputLength + 2) / 3) * 4;
  }

  // Calculate the length of the decoded data
  static size_t decodedLength(const char *input)
  {
    size_t len = strlen(input);
    size_t padding = 0;
    if (len >= 2 && input[len - 1] == '=')
      padding++;
    if (len >= 2 && input[len - 2] == '=')
      padding++;
    return (len / 4) * 3 - padding;
  }

  // Encode data to Base64
  static void encode(char *output, const char *input, size_t inputLength)
  {
    const char table[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";

    size_t i = 0, j = 0;
    for (; i < inputLength - 2; i += 3)
    {
      output[j++] = table[(input[i] >> 2) & 0x3F];
      output[j++] = table[((input[i] & 0x3) << 4) | ((input[i + 1] >> 4) & 0xF)];
      output[j++] = table[((input[i + 1] & 0xF) << 2) | ((input[i + 2] >> 6) & 0x3)];
      output[j++] = table[input[i + 2] & 0x3F];
    }

    if (i < inputLength)
    {
      output[j++] = table[(input[i] >> 2) & 0x3F];
      if (i + 1 < inputLength)
      {
        output[j++] = table[((input[i] & 0x3) << 4) | ((input[i + 1] >> 4) & 0xF)];
        output[j++] = table[(input[i + 1] & 0xF) << 2];
      }
      else
      {
        output[j++] = table[(input[i] & 0x3) << 4];
        output[j++] = '=';
      }
      output[j++] = '=';
    }

    output[j] = '\0'; // Null-terminate the string
  }

  // Decode Base64 data
  static bool decode(char *output, const char *input, size_t inputLength)
  {
    const int table[256] = {
        -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, // 0-15
        -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, // 16-31
        -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, 62, -1, -1, -1, 63, // 32-47
        52, 53, 54, 55, 56, 57, 58, 59, 60, 61, -1, -1, -1, -1, -1, -1, // 48-63
        -1, 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14,           // 64-79
        15, 16, 17, 18, 19, 20, 21, 22, 23, 24, 25, -1, -1, -1, -1, -1, // 80-95
        -1, 26, 27, 28, 29, 30, 31, 32, 33, 34, 35, 36, 37, 38, 39, 40, // 96-111
        41, 42, 43, 44, 45, 46, 47, 48, 49, 50, 51, -1, -1, -1, -1, -1, // 112-127
        -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, // 128-143
        -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, // 144-159
        -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, // 160-175
        -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, // 176-191
        -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, // 192-207
        -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, // 208-223
        -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, // 224-239
        -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1  // 240-255
    };

    size_t i = 0, j = 0;
    uint8_t buffer[4];

    while (i < inputLength)
    {
      size_t count = 0;
      for (size_t k = 0; k < 4; k++)
      {
        if (i < inputLength && input[i] != '=' && table[(unsigned char)input[i]] != -1)
        {
          buffer[k] = table[(unsigned char)input[i]];
          count++;
        }
        else
        {
          buffer[k] = 0;
        }
        i++;
      }

      if (count > 1)
        output[j++] = (buffer[0] << 2) | (buffer[1] >> 4);
      if (count > 2)
        output[j++] = (buffer[1] << 4) | (buffer[2] >> 2);
      if (count > 3)
        output[j++] = (buffer[2] << 6) | buffer[3];
    }

    output[j] = '\0'; // Null-terminate the string
    return true;
  }
};

#endif // BASE64_H
