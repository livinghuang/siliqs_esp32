#pragma once
#include "siliqs_esp32.h"
#include "sq_ota.h"
#include "src.h"
void create_packet_header(struct DataPacket &packet);
void create_packet_normal(struct DataPacket &packet, struct payload_data piece);
bool validatePacket(const DataPacket &packet);