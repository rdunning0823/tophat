/*
 * Copyright (C) 2012 Max Kellermann <max@duempel.org>
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

#ifndef XCSOAR_SOCKET_ADDRESS_HPP
#define XCSOAR_SOCKET_ADDRESS_HPP

#include "Compiler.h"

#ifdef HAVE_POSIX
#include <sys/socket.h>
#else
#include <winsock2.h>
#endif

/**
 * An OO wrapper for a UNIX socket descriptor.
 */
class SocketAddress {
  size_t length;
  struct sockaddr_storage address;

public:
  SocketAddress() = default;

  operator const struct sockaddr *() const {
    return reinterpret_cast<const struct sockaddr *>(&address);
  }

  size_t GetLength() const {
    return length;
  }

  int GetFamily() const {
    return address.ss_family;
  }

  bool IsDefined() const {
    return GetFamily() != AF_UNSPEC;
  }

  void Clear() {
    address.ss_family = AF_UNSPEC;
  }

  /**
   * Set up a wildcard address with the specified port.
   */
  void Port(unsigned port);

  bool Lookup(const char *host, const char *service, int socktype);
};

#endif
