/**
 * This software is distributed under the terms of the MIT License.
 * Copyright (c) 2020 LXRobotics.
 * Author: Alexander Entinger <alexander.entinger@lxrobotics.com>
 * Contributors: https://github.com/107-systems/107-Arduino-UAVCAN/graphs/contributors.
 */

/**************************************************************************************
 * INCLUDE
 **************************************************************************************/

#include <vector>

#include <catch.hpp>

#include <test/util/Const.h>
#include <test/util/Types.h>
#include <test/util/micros.h>

#include <ArduinoUAVCAN.h>

/**************************************************************************************
 * PRIVATE GLOBAL VARIABLES
 **************************************************************************************/

static util::CanFrame        can_frame;
static uint32_t              hb_uptime  = 0;
static Heartbeat_1_0::Health hb_health  = Heartbeat_1_0::Health::NOMINAL;
static Heartbeat_1_0::Mode   hb_mode    = Heartbeat_1_0::Mode::OPERATIONAL;
static uint32_t              hb_vssc    = 0;
static CanardNodeID          hb_node_id = 0;

/**************************************************************************************
 * PRIVATE FUNCTION DEFINITION
 **************************************************************************************/

static bool transmitCanFrame(uint32_t const id, uint8_t const * data, uint8_t const len)
{
  util::CanFrame frame{id, std::vector<uint8_t>(data, data + len)};
  can_frame = frame;
  return true;
}

void onHeatbeat_1_0_Received(CanardTransfer const & transfer, ArduinoUAVCAN & /* uavcan */)
{
  Heartbeat_1_0 const hb = Heartbeat_1_0::create(transfer);

  hb_node_id = transfer.remote_node_id;
  hb_uptime  = hb.uptime();
  hb_health  = hb.health();
  hb_mode    = hb.mode();
  hb_vssc    = hb.vssc();
}

/**************************************************************************************
 * TEST CODE
 **************************************************************************************/

TEST_CASE("A '32085.Heartbeat.1.0.uavcan' message is sent", "[heartbeat-01]")
{
  ArduinoUAVCAN uavcan(util::LOCAL_NODE_ID, util::micros, transmitCanFrame);

  Heartbeat_1_0 hb_1(9876, Heartbeat_1_0::Health::NOMINAL, Heartbeat_1_0::Mode::SOFTWARE_UPDATE, 5);
  uavcan.publish(hb_1);
  uavcan.transmitCanFrame();
  /*
   * pyuavcan publish 32085.uavcan.node.Heartbeat.1.0 '{uptime: 9876, health: 0, mode: 3, vendor_specific_status_code: 5}' --tr='CAN(can.media.socketcan.SocketCANMedia("vcan0",8),13)'
   */
  REQUIRE(can_frame.id   == 0x107D550D);
  REQUIRE(can_frame.data == std::vector<uint8_t>{0x94, 0x26, 0x00, 0x00, 0xAC, 0x00, 0x00, 0xE0});

  Heartbeat_1_0 hb_2(9881, Heartbeat_1_0::Health::ADVISORY, Heartbeat_1_0::Mode::MAINTENANCE, 123);
  uavcan.publish(hb_2);
  uavcan.transmitCanFrame();
  /*
   * pyuavcan publish 32085.uavcan.node.Heartbeat.1.0 '{uptime: 9881, health: 1, mode: 2, vendor_specific_status_code: 123}' --tr='CAN(can.media.socketcan.SocketCANMedia("vcan0",8),13)'
   */
  REQUIRE(can_frame.id   == 0x107D550D);
  REQUIRE(can_frame.data == std::vector<uint8_t>{0x99, 0x26, 0x00, 0x00, 0x69, 0x0F, 0x00, 0xE1});
}

TEST_CASE("A '32085.Heartbeat.1.0.uavcan' message is received", "[heartbeat-02]")
{
  ArduinoUAVCAN uavcan(util::LOCAL_NODE_ID, util::micros, nullptr);

  REQUIRE(uavcan.subscribe<Heartbeat_1_0>(onHeatbeat_1_0_Received));

  /* Create:
   *   pyuavcan publish 32085.uavcan.node.Heartbeat.1.0 '{uptime: 1337, health: 2, mode: 7, vendor_specific_status_code: 42}' --tr='CAN(can.media.socketcan.SocketCANMedia("vcan0",8),59)'
   *
   * Capture:
   *   sudo modprobe vcan
   *   sudo ip link add dev vcan0 type vcan
   *   sudo ip link set up vcan0
   *   candump -decaxta vcan0
   */
  uint8_t const data[] = {0x39, 0x05, 0x00, 0x00, 0x5E, 0x05, 0x00, 0xE1};
  uavcan.onCanFrameReceived(0x107D553B, data, sizeof(data));

  REQUIRE(hb_node_id == 59);
  REQUIRE(hb_uptime  == 1337);
  REQUIRE(hb_health  == Heartbeat_1_0::Health::CAUTION);
  REQUIRE(hb_mode    == Heartbeat_1_0::Mode::OFFLINE);
  REQUIRE(hb_vssc    == 42);
}
