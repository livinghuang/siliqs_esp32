// Copyright 2015-2020 Espressif Systems (Shanghai) PTE LTD
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//     http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

#include "vfs_api.h"

extern "C"
{
#include <sys/unistd.h>
#include <sys/stat.h>
#include <dirent.h>
}
#include "sdkconfig.h"
#include "sqLittleFS.h"

#ifdef CONFIG_LITTLEFS_PAGE_SIZE
extern "C"
{
#include "esp_littlefs.h"
}

using namespace fs;

class sqLittleFSImpl : public VFSImpl
{
public:
  sqLittleFSImpl();
  virtual ~sqLittleFSImpl() {}
};

sqLittleFSImpl::sqLittleFSImpl() {}

SQ_LittleFSFS::SQ_LittleFSFS() : FS(FSImplPtr(new sqLittleFSImpl())), partitionLabel_(NULL) {}

SQ_LittleFSFS::~SQ_LittleFSFS()
{
  if (partitionLabel_)
  {
    free(partitionLabel_);
    partitionLabel_ = NULL;
  }
}

bool SQ_LittleFSFS::begin(bool formatOnFail, const char *basePath, uint8_t maxOpenFiles, const char *partitionLabel)
{
  // Force using "spiffs" partition label for testing
  const char *forcedPartitionLabel = "spiffs";

  // Free any previously allocated memory for the label
  if (partitionLabel_)
  {
    free(partitionLabel_);
    partitionLabel_ = NULL;
  }

  // Always use the forced label
  partitionLabel_ = strdup(forcedPartitionLabel);

  // Check if already mounted
  if (esp_littlefs_mounted(partitionLabel_))
  {
    console.log(sqINFO, "LittleFS Already Mounted!");
    return true;
  }

  // Configure LittleFS
  esp_vfs_littlefs_conf_t conf = {
      .base_path = basePath,
      .partition_label = partitionLabel_,
      .partition = NULL,
      .format_if_mount_failed = false,
      .read_only = false,
      .dont_mount = false,
      .grow_on_mount = true};

  // Attempt to mount LittleFS
  esp_err_t err = esp_vfs_littlefs_register(&conf);
  if (err == ESP_FAIL && formatOnFail)
  {
    if (format())
    {
      err = esp_vfs_littlefs_register(&conf);
    }
  }

  if (err != ESP_OK)
  {
    console.log(sqERROR, "Mounting LittleFS failed! Error: %d", err);
    return false;
  }

  // Retrieve and log partition information
  const esp_partition_t *partition = esp_partition_find_first(ESP_PARTITION_TYPE_DATA, ESP_PARTITION_SUBTYPE_DATA_SPIFFS, partitionLabel_);
  if (partition)
  {
    console.log(sqINFO, "Partition Label: %s", partition->label);
    console.log(sqINFO, "Partition Type: 0x%02X", partition->type);
    console.log(sqINFO, "Partition Subtype: 0x%02X", partition->subtype);
    console.log(sqINFO, "Partition Address: 0x%08X", partition->address);
    console.log(sqINFO, "Partition Size: %d bytes", partition->size);
  }
  else
  {
    console.log(sqWARN, "Could not retrieve partition information for label: %s", partitionLabel_);
  }

  _impl->mountpoint(basePath);
  return true;
}

void SQ_LittleFSFS::end()
{
  if (esp_littlefs_mounted(partitionLabel_))
  {
    esp_err_t err = esp_vfs_littlefs_unregister(partitionLabel_);
    if (err)
    {
      console.log(sqERROR, "Unmounting LittleFS failed! Error: %d", err);
      return;
    }
    _impl->mountpoint(NULL);
  }
}

bool SQ_LittleFSFS::format()
{
  disableCore0WDT();
  esp_err_t err = esp_littlefs_format(partitionLabel_);
  enableCore0WDT();
  if (err)
  {
    console.log(sqERROR, "Formatting LittleFS failed! Error: %d", err);
    return false;
  }
  return true;
}

size_t SQ_LittleFSFS::totalBytes()
{
  size_t total, used;
  if (esp_littlefs_info(partitionLabel_, &total, &used))
  {
    return 0;
  }
  return total;
}

size_t SQ_LittleFSFS::usedBytes()
{
  size_t total, used;
  if (esp_littlefs_info(partitionLabel_, &total, &used))
  {
    return 0;
  }
  return used;
}

SQ_LittleFSFS sqLittleFS;
#endif
