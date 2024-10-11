#include "bsp.h" // 如果你需要该头文件用于板子的配置
#ifndef FILE_SYSTEM_H
#define FILE_SYSTEM_H
#include "siliqs_heltec_esp32.h"
#include <Arduino.h>
#include <LittleFS.h> // Include the LittleFS library

class FileSystem
{
public:
  // Constructor and Destructor
  FileSystem();
  ~FileSystem();

  // Initialize the file system
  bool begin(bool formatOnFail = true);

  // Write data to a file
  bool writeFile(const char *path, const String &data);

  // Read data from a file
  String readFile(const char *path);

  // Delete a file
  bool deleteFile(const char *path); // Add this line for the delete function

  // List all files
  void listFiles();

  // Format the file system
  bool formatFS();
};
extern FileSystem fileSystem;
#endif // FILE_SYSTEM_H