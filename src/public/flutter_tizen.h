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

  typedef struct FlutterApplicationState *FlutterApplicationRef;

  // Properties representing a generic rectangular size.
  typedef struct
  {
    int32_t width;
    int32_t height;
  } FlutterDisplaySize;

  // Properties for configuring a Flutter engine instance.
  typedef struct
  {
    // The path to the flutter_assets folder for the application to be run.
    const char *assets_path;
    // The path to the icudtl.dat file for the version of Flutter you are using.
    const char *icu_data_path;
    // The path to the libapp.so file for the application to be run.
    const char *aot_library_path;
  } FlutterEngineProperties;

  FLUTTER_EXPORT FlutterApplicationRef RunFlutterApplication(
      const FlutterDisplaySize &size,
      const FlutterEngineProperties &properties);

  FLUTTER_EXPORT bool StopFlutterApplication(FlutterApplicationRef application);

#if defined(__cplusplus)
} // extern "C"
#endif

#endif // FLUTTER_SHELL_PLATFORM_TIZEN_PUBLIC_FLUTTER_H_
