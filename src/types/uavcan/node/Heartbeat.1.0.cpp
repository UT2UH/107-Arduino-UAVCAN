/**
 * This software is distributed under the terms of the MIT License.
 * Copyright (c) 2020 LXRobotics.
 * Author: Alexander Entinger <alexander.entinger@lxrobotics.com>
 * Contributors: https://github.com/107-systems/107-Arduino-UAVCAN/graphs/contributors.
 */

/**************************************************************************************
 * INCLUDE
 **************************************************************************************/

#include "Heartbeat.1.0.h"

#include <libcanard/canard_dsdl.h>

/**************************************************************************************
 * STATIC CONSTEXPR DEFINITION
 **************************************************************************************/

constexpr CanardPortID       Heartbeat_1_0::PORT_ID;
constexpr size_t             Heartbeat_1_0::MAX_PAYLOAD_SIZE;
constexpr CanardTransferKind Heartbeat_1_0::TRANSFER_KIND;

/**************************************************************************************
 * CTOR/DTOR
 **************************************************************************************/

Heartbeat_1_0::Heartbeat_1_0(uint32_t const uptime, Health const health, Mode const mode, uint32_t const vssc)
: _uptime{uptime}
, _health{health}
, _mode{mode}
, _vssc{vssc}
{

}

/**************************************************************************************
 * PUBLIC MEMBER FUNCTIONS
 **************************************************************************************/

Heartbeat_1_0 Heartbeat_1_0::create(CanardTransfer const & transfer)
{
  Mode     const mode   = static_cast<Mode>  (canardDSDLGetU8 (reinterpret_cast<uint8_t const *>(transfer.payload), transfer.payload_size, 34,  3));
  uint32_t const uptime =                     canardDSDLGetU32(reinterpret_cast<uint8_t const *>(transfer.payload), transfer.payload_size,  0, 32);
  Health   const health = static_cast<Health>(canardDSDLGetU8 (reinterpret_cast<uint8_t const *>(transfer.payload), transfer.payload_size, 32,  2));
  uint32_t const vssc   =                     canardDSDLGetU32(reinterpret_cast<uint8_t const *>(transfer.payload), transfer.payload_size, 37, 19);

  return Heartbeat_1_0(uptime, health, mode, vssc);
}

size_t Heartbeat_1_0::encode(uint8_t * payload) const
{
  canardDSDLSetUxx(payload, 34, static_cast<uint8_t>(_mode),    3);
  canardDSDLSetUxx(payload,  0,                      _uptime,  32);
  canardDSDLSetUxx(payload, 37,                      _vssc,    19);
  canardDSDLSetUxx(payload, 32, static_cast<uint8_t>(_health),  2);

  return MAX_PAYLOAD_SIZE;
}
