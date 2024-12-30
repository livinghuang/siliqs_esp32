#include "file_system.h"
#include "siliqs_esp32.h"
// Constructor
FileSystem::FileSystem()
{
  // Optional logging or initialization
}

// Destructor
FileSystem::~FileSystem()
{
  // Cleanup resources if needed
}

// Initialize sqLittleFS
bool FileSystem::begin(bool formatOnFail)
{
  if (!sqLittleFS.begin(formatOnFail))
  {
    console.log(sqINFO, "Failed to load sqLittleFS");
    return false;
  }
  console.log(sqINFO, "sqLittleFS loaded successfully");

  // Get and display storage information
  size_t totalBytes = sqLittleFS.totalBytes(); // Total storage in bytes
  size_t usedBytes = sqLittleFS.usedBytes();   // Used storage in bytes
  size_t freeBytes = totalBytes - usedBytes;   // Free storage in bytes

  console.log(sqINFO, "Storage Information:");
  console.log(sqINFO, "Total Space: " + String(totalBytes) + " bytes");
  console.log(sqINFO, "Used Space: " + String(usedBytes) + " bytes");
  console.log(sqINFO, "Free Space: " + String(freeBytes) + " bytes");

  return true;
}
// Append data to a file
bool FileSystem::appendFile(const char *path, const String &data)
{
  // Check if the file path is valid
  if (path == nullptr || strlen(path) == 0)
  {
    console.log(sqERROR, "Invalid file path");
    return false;
  }

  // Attempt to open the file in append mode
  File file = sqLittleFS.open(path, FILE_APPEND);
  if (!file)
  {
    console.log(sqERROR, "Failed to open file: " + String(path) + ", cannot append data");
    return false;
  }

  // Append data to the file
  if (file.print(data))
  {
    file.flush(); // Ensure data is written to storage
    console.log(sqINFO, "Data successfully appended to file: " + String(path));
    file.close();
    return true;
  }
  else
  {
    console.log(sqERROR, "Failed to append data to file: " + String(path));
    file.close();
    return false;
  }
}

// Write data to a file
bool FileSystem::writeFile(const char *path, const String &data)
{
  // Check if the file path is valid
  if (path == nullptr || strlen(path) == 0)
  {
    console.log(sqERROR, "Invalid file path");
    return false;
  }

  File file = sqLittleFS.open(path, FILE_WRITE);
  if (!file)
  {
    console.log(sqINFO, "Failed to open file " + String(path) + ", cannot write");
    return false;
  }

  if (file.print(data))
  {
    console.log(sqINFO, "File " + String(path) + " written successfully");
    file.close();
    return true;
  }
  else
  {
    console.log(sqINFO, "File " + String(path) + " write failed");
    file.close();
    return false;
  }
}

// Read data from a file
String FileSystem::readFile(const char *path)
{
  // Check if the file path is valid
  if (path == nullptr || strlen(path) == 0)
  {
    console.log(sqERROR, "Invalid file path");
    return "";
  }

  File file = sqLittleFS.open(path, FILE_READ);
  if (!file)
  {
    console.log(sqINFO, "Failed to open file " + String(path) + ", cannot read");
    return "";
  }

  String fileContent;
  while (file.available())
  {
    fileContent += char(file.read());
  }

  console.log(sqINFO, "File " + String(path) + " read successfully");
  file.close();
  return fileContent;
}

// Delete a file
bool FileSystem::deleteFile(const char *path)
{
  // Check if the file path is valid
  if (path == nullptr || strlen(path) == 0)
  {
    console.log(sqERROR, "Invalid file path");
    return false;
  }

  if (sqLittleFS.remove(path))
  {
    console.log(sqINFO, "File " + String(path) + " deleted successfully");
    return true;
  }
  else
  {
    console.log(sqINFO, "Failed to delete file " + String(path));
    return false;
  }
}

// List all files in sqLittleFS
void FileSystem::listFiles()
{
  console.log(sqINFO, "Listing all files in sqLittleFS:");
  File root = sqLittleFS.open("/");
  File file = root.openNextFile();
  while (file)
  {
    console.log(sqINFO, "File: " + String(file.name()) + ", Size: " + String(file.size()) + " bytes");
    file = root.openNextFile();
  }
}

// Format the file system
bool FileSystem::formatFS()
{
  if (sqLittleFS.format())
  {
    console.log(sqINFO, "sqLittleFS formatted successfully");
    return true;
  }
  else
  {
    console.log(sqINFO, "Failed to format sqLittleFS");
    return false;
  }
}