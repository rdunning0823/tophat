/*
 * Copyright (C) 2010-2012 Max Kellermann <max@duempel.org>
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

#ifndef XCSOAR_JAVA_URL_HPP
#define XCSOAR_JAVA_URL_HPP

#include "Class.hpp"

#include <jni.h>
#include <assert.h>

namespace Java {
  /**
   * Wrapper for a java.net.URL object.
   */
  class URL {
    static TrivialClass cls;
    static jmethodID ctor, openConnection_method;

  public:
    static void Initialise(JNIEnv *env);
    static void Deinitialise(JNIEnv *env);

    static jobject Create(JNIEnv *env, jstring url) {
      assert(env != NULL);
      assert(url != NULL);
      assert(ctor != NULL);

      return env->NewObject(cls, ctor, url);
    }

    static jobject openConnection(JNIEnv *env, jobject url) {
      assert(env != NULL);
      assert(url != NULL);
      assert(openConnection_method != NULL);

      return env->CallObjectMethod(url, openConnection_method);
    }
  };

  /**
   * Wrapper for a java.net.URLConnection object.
   */
  class URLConnection {
    static jmethodID setConnectTimeout_method;
    static jmethodID setReadTimeout_method;
    static jmethodID getContentLength_method;
    static jmethodID getInputStream_method;

  public:
    static void Initialise(JNIEnv *env);

    static void setConnectTimeout(JNIEnv *env, jobject connection,
                                  jint timeout) {
      assert(env != NULL);
      assert(connection != NULL);
      assert(setConnectTimeout_method != NULL);

      env->CallVoidMethod(connection, setConnectTimeout_method, timeout);
    }

    static void setReadTimeout(JNIEnv *env, jobject connection, jint timeout) {
      assert(env != NULL);
      assert(connection != NULL);
      assert(setReadTimeout_method != NULL);

      env->CallVoidMethod(connection, setReadTimeout_method, timeout);
    }

    static int getContentLength(JNIEnv *env, jobject connection) {
      assert(env != NULL);
      assert(connection != NULL);
      assert(getContentLength_method != NULL);

      return env->CallIntMethod(connection, getContentLength_method);
    }

    static jobject getInputStream(JNIEnv *env, jobject connection) {
      assert(env != NULL);
      assert(connection != NULL);
      assert(getInputStream_method != NULL);

      return env->CallObjectMethod(connection, getInputStream_method);
    }
  };
}

#endif
