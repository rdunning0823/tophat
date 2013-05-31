/*
Copyright_License {

  XCSoar Glide Computer - http://www.xcsoar.org/
  Copyright (C) 2000-2012 The XCSoar Project
  A detailed list of copyright holders can be found in the file "AUTHORS".

  This program is free software; you can redistribute it and/or
  modify it under the terms of the GNU General Public License
  as published by the Free Software Foundation; either version 2
  of the License, or (at your option) any later version.

  This program is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU General Public License for more details.

  You should have received a copy of the GNU General Public License
  along with this program; if not, write to the Free Software
  Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
}
*/

#ifndef XCSOAR_ANDROID_PORT_BRIDGE_HPP
#define XCSOAR_ANDROID_PORT_BRIDGE_HPP

#include "Java/Object.hpp"

class DataHandler;

class PortBridge : protected Java::Object {
  static jmethodID close_method;
  static jmethodID setListener_method;
  static jmethodID isValid_method;
  static jmethodID drain_method;
  static jmethodID getBaudRate_method, setBaudRate_method;
  static jmethodID write_method;

  static constexpr size_t write_buffer_size = 4096;
  Java::GlobalRef<jbyteArray> write_buffer;

public:
  /**
   * Global initialisation.  Looks up the methods of the AndroidPort
   * Java class.
   */
  static void Initialise(JNIEnv *env);

  PortBridge(JNIEnv *env, jobject obj);

  ~PortBridge() {
    close(Java::GetEnv());
  }

  void close(JNIEnv *env) {
    env->CallVoidMethod(Get(), close_method);
  }

  void setListener(JNIEnv *env, DataHandler *handler);

  bool isValid(JNIEnv *env) {
    return env->CallBooleanMethod(Get(), isValid_method);
  }

  bool drain(JNIEnv *env) {
    return env->CallBooleanMethod(Get(), drain_method);
  }

  int getBaudRate(JNIEnv *env) const {
    return env->CallIntMethod(Get(), getBaudRate_method);
  }

  bool setBaudRate(JNIEnv *env, int baud_rate) {
    return env->CallBooleanMethod(Get(), setBaudRate_method, baud_rate);
  }

  int write(JNIEnv *env, const void *data, size_t length);
};

#endif
