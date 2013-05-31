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

#include "Android/BluetoothHelper.hpp"
#include "PortBridge.hpp"
#include "Java/String.hpp"
#include "Java/Class.hpp"

namespace BluetoothHelper {
  static Java::TrivialClass cls;
  static jmethodID list_method, connect_method, createServer_method;
}

bool
BluetoothHelper::Initialise(JNIEnv *env)
{
  assert(!cls.IsDefined());
  assert(env != NULL);

  if (!cls.FindOptional(env, "org/tophat/BluetoothHelper"))
    /* Android < 2.0 doesn't have Bluetooth support */
    return false;

  list_method = env->GetStaticMethodID(cls, "list", "()[Ljava/lang/String;");
  connect_method = env->GetStaticMethodID(cls, "connect",
                                          "(Ljava/lang/String;)Lorg/tophat/AndroidPort;");
  createServer_method = env->GetStaticMethodID(cls, "createServer",
                                               "()Lorg/tophat/AndroidPort;");
  return true;
}

void
BluetoothHelper::Deinitialise(JNIEnv *env)
{
  cls.ClearOptional(env);
}

jobjectArray
BluetoothHelper::list(JNIEnv *env)
{
  if (!cls.IsDefined())
    return NULL;

  /* call BluetoothHelper.connect() */

  return (jobjectArray)env->CallStaticObjectMethod(cls, list_method);
}

PortBridge *
BluetoothHelper::connect(JNIEnv *env, const char *address)
{
  if (cls == NULL)
    return NULL;

  /* call BluetoothHelper.connect() */

  const Java::String address2(env, address);
  jobject obj = env->CallStaticObjectMethod(cls, connect_method,
                                            address2.Get());
  if (obj == NULL)
    return NULL;

  PortBridge *helper = new PortBridge(env, obj);
  env->DeleteLocalRef(obj);

  return helper;
}

PortBridge *
BluetoothHelper::createServer(JNIEnv *env)
{
  if (cls == NULL)
    return NULL;

  jobject obj = env->CallStaticObjectMethod(cls, createServer_method);
  if (obj == NULL)
    return NULL;

  PortBridge *helper = new PortBridge(env, obj);
  env->DeleteLocalRef(obj);

  return helper;
}
