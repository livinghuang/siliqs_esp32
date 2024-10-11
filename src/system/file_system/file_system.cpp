#include "file_system.h"
#include "siliqs_heltec_esp32.h"
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

// Initialize LittleFS
bool FileSystem::begin(bool formatOnFail)
{
  if (!LittleFS.begin(formatOnFail))
  {
    console.log(sqINFO, "Failed to load LittleFS");
    return false;
  }
  console.log(sqINFO, "LittleFS loaded successfully");

  // Get and display storage information
  size_t totalBytes = LittleFS.totalBytes(); // Total storage in bytes
  size_t usedBytes = LittleFS.usedBytes();   // Used storage in bytes
  size_t freeBytes = totalBytes - usedBytes; // Free storage in bytes

  console.log(sqINFO, "Storage Information:");
  console.log(sqINFO, "Total Space: " + String(totalBytes) + " bytes");
  console.log(sqINFO, "Used Space: " + String(usedBytes) + " bytes");
  console.log(sqINFO, "Free Space: " + String(freeBytes) + " bytes");

  return true;
}

// Write data to a file
bool FileSystem::writeFile(const char *path, const String &data)
{
  File file = LittleFS.open(path, FILE_WRITE);
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
  File file = LittleFS.open(path, FILE_READ);
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
  if (LittleFS.remove(path))
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

// List all files in LittleFS
void FileSystem::listFiles()
{
  console.log(sqINFO, "Listing all files in LittleFS:");
  File root = LittleFS.open("/");
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
  if (LittleFS.format())
  {
    console.log(sqINFO, "LittleFS formatted successfully");
    return true;
  }
  else
  {
    console.log(sqINFO, "Failed to format LittleFS");
    return false;
  }
}