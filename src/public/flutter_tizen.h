/*
 * Copyright (c) 2020 Samsung Electronics Co., Ltd All Rights Reserved
 *
 *    Licensed under the Apache License, Version 2.0 (the "License");
 *    you may not use this file except in compliance with the License.
 *    You may obtain a copy of the License at
 *
 *        http://www.apache.org/licenses/LICENSE-2.0
 *
 *    Unless required by applicable law or agreed to in writing, software
 *    distributed under the License is distributed on an "AS IS" BASIS,
 *    WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *    See the License for the specific language governing permissions and
 *    limitations under the License.
 */

#ifndef FLUTTER_SHELL_PLATFORM_TIZEN_PUBLIC_FLUTTER_H_
#define FLUTTER_SHELL_PLATFORM_TIZEN_PUBLIC_FLUTTER_H_

#include <stddef.h>
#include <stdint.h>

#include "flutter_export.h"

#if defined(__cplusplus)
extern "C"
{
#endif

  // Properties representing a generic rectangular size.
  typedef struct
  {
    int32_t width;
    int32_t height;
  } FlutterDesktopSize;

  typedef struct FlutterApplicationState *FlutterApplicationRef;

  // Properties for configuring a Flutter engine instance.
  typedef struct
  {
    // The path to the flutter_assets folder for the application to be run.
    // This can either be an absolute path or a path relative to the directory
    // containing the executable.
    const char *assets_path;
    // The path to the icudtl.dat file for the version of Flutter you are using.
    // This can either be an absolute path or a path relative to the directory
    // containing the executable.
    const char *icu_data_path;
  } FlutterDesktopEngineProperties;

  FLUTTER_EXPORT FlutterApplicationRef RunFlutterApplication(
      const FlutterDesktopSize size,
      const FlutterDesktopEngineProperties engine_properties,
      // The switches to pass to the Flutter engine.
      //
      // See: https://github.com/flutter/engine/blob/master/shell/common/switches.h
      // for details. Not all arguments will apply.
      const char **switches,
      // The number of elements in |switches|.
      size_t switches_count);

  // FLUTTER_EXPORT bool StopFlutterApplication(FlutterApplicationRef application);

#if defined(__cplusplus)
} // extern "C"
#endif

#endif // FLUTTER_SHELL_PLATFORM_TIZEN_PUBLIC_FLUTTER_H_
