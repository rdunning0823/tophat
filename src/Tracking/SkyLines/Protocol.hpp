/*
 * XCSoar Glide Computer - http://www.xcsoar.org/
 * Copyright (C) 2000-2012 The XCSoar Project
 * A detailed list of copyright holders can be found in the file "AUTHORS".
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *
 * - Redistributions of source code must retain the above copyright
 * notice, this list of conditions and the following disclaimer.
 *
 * - Redistributions in binary form must reproduce the above copyright
 * notice, this list of conditions and the following disclaimer in the
 * documentation and/or other materials provided with the
 * distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 * ``AS IS'' AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS
 * FOR A PARTICULAR PURPOSE ARE DISCLAIMED.  IN NO EVENT SHALL THE
 * FOUNDATION OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT,
 * INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
 * SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT,
 * STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED
 * OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#ifndef XCSOAR_TRACKING_SKYLINES_PROTOCOL_HPP
#define XCSOAR_TRACKING_SKYLINES_PROTOCOL_HPP

#include <stdint.h>

/*
 * This file defines the SkyLines live tracking protocol.  It is a
 * one-way datagram protocol: the client (i.e. the on-board navigation
 * device) sends datagrams to the server without expecting a response.
 * It is light-weight.
 *
 * Mobile network availability is said to be bad during a flight,
 * which directed the design of this protocol.  The submission
 * frequency is high enough that useful data will be seen on the
 * server even if packet loss is high.  The protocol does not require
 * a handshake, because that would raise the requirements for network
 * reliability.
 *
 */

/*
 * The struct definitions below imply a specific memory layout.  They
 * have been designed in a way that all compilers we know will not
 * implicitly insert padding, because all attributes are aligned
 * properly already.
 *
 * All integers are big-endian.
 *
 */

namespace SkyLinesTracking {
  static const uint32_t MAGIC = 0x5df4b67b;

  enum Type {
    PING = 1,
    ACK = 2,
    FIX = 3,
  };

  /**
   * The datagram payload header.
   */
  struct Header {
    /**
     * Must be MAGIC.
     */
    uint32_t magic;

    /**
     * The CRC of this packet including the header, assuming this
     * attribute is 0.
     *
     * The CRC algorithm is CRC16-CCITT with initial value 0x0000
     * (XModem) instead of CCITT's default 0xffff.
     */
    uint16_t crc;

    /**
     * An "enum Type" value.
     */
    uint16_t type;

    /**
     * The authorization key.
     */
    uint64_t key;
  };

#ifdef __cplusplus
  static_assert(sizeof(Header) == 16, "Wrong struct size");
#endif

  /**
   * Check the network connection and verify the key (#PING).  The
   * server responds with #ACK.
   */
  struct PingPacket {
    Header header;

    /**
     * An arbitrary number chosen by the client, usually a sequence
     * number.
     */
    uint16_t id;

    /**
     * Reserved for future use.  Set to zero.
     */
    uint16_t reserved;

    /**
     * Reserved for future use.  Set to zero.
     */
    uint32_t reserved2;
  };

#ifdef __cplusplus
  static_assert(sizeof(PingPacket) == 24, "Wrong struct size");
#endif

  /**
   * A generic acknowledge packet sent by the server in response to
   * certain request packets.
   */
  struct ACKPacket {
    /**
     * The key was not valid.  Usually, requests with bad keys are
     * silently discarded, but the server may use this flag to respond
     * to a bad key in a PING packet.
     */
    static const uint32_t FLAG_BAD_KEY = 0x1;

    Header header;

    /**
     * Copy of the request's id value.
     */
    uint16_t id;

    /**
     * Reserved for future use.  Set to zero.
     */
    uint16_t reserved;

    uint32_t flags;
  };

#ifdef __cplusplus
  static_assert(sizeof(ACKPacket) == 24, "Wrong struct size");
#endif

  struct GeoPoint {
    /**
     * Angle in micro degrees.  Positive means north or east.
     */
    int32_t latitude, longitude;
  };

#ifdef __cplusplus
  static_assert(sizeof(GeoPoint) == 8, "Wrong struct size");
#endif

  /**
   * A GPS fix being uploaded to the server.
   */
  struct FixPacket {
    static const uint32_t FLAG_LOCATION = 0x1;
    static const uint32_t FLAG_TRACK = 0x2;
    static const uint32_t FLAG_GROUND_SPEED = 0x4;
    static const uint32_t FLAG_AIRSPEED = 0x8;
    static const uint32_t FLAG_ALTITUDE = 0x10;
    static const uint32_t FLAG_VARIO = 0x20;
    static const uint32_t FLAG_ENL = 0x40;

    Header header;

    uint32_t flags;

    /**
     * Millisecond of day (UTC).  May be bigger than 24*60*60 if the
     * flight has wrapped midnight.
     */
    uint32_t time;

    GeoPoint location;

    /**
     * Reserved for future use.  Set to zero.
     */
    uint32_t reserved;

    /**
     * Ground track in degrees (0..359).
     */
    uint16_t track;

    /**
     * Ground speed in m/16s.
     */
    uint16_t ground_speed;

    /**
     * Indicated air speed in m/16s.
     */
    uint16_t airspeed;

    /**
     * Altitude in m above MSL.
     */
    int16_t altitude;

    /**
     * Vertical speed in m/256s.
     */
    int16_t vario;

    /**
     * Engine noise level value from the logger, valid range is
     * 0..999.
     */
    uint16_t engine_noise_level;
  };

#ifdef __cplusplus
  static_assert(sizeof(FixPacket) == 48, "Wrong struct size");
#endif
};

#endif
