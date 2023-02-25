
#ifndef XCSOAR_GLIDER_LINK_DATA_H
#define XCSOAR_GLIDER_LINK_DATA_H

#include "GliderLink/List.hpp"

#include <type_traits>

/**
 * A container for all data received by GliderLink.
 */
struct GliderLinkData {
  GliderLinkTrafficList traffic;

  void Complement(const GliderLinkData &add) {
    traffic.Replace(add.traffic);
  }

  void Expire(fixed clock) {
    traffic.Expire(clock);
  }
};

#endif
