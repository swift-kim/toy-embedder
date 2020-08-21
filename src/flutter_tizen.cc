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

#include "flutter_tizen.h"
#include "flutter_application.h"
#include "tizen_display.h"
#include "logger.h"

struct FlutterApplicationState
{
  std::unique_ptr<flutter::TizenDisplay> display;
  std::unique_ptr<flutter::FlutterApplication> application;
};

FLUTTER_EXPORT FlutterApplicationRef RunFlutterApplication(
    const FlutterDisplaySize &size,
    const FlutterEngineProperties &engine_properties)
{
  auto state = new FlutterApplicationState();

  state->display = std::make_unique<flutter::TizenDisplay>(size.width, size.height);
  if (!state->display->IsValid())
  {
    LogE("Could not initialize the display.");
    return nullptr;
  }

  state->application = std::make_unique<flutter::FlutterApplication>(
      engine_properties.assets_path,
      engine_properties.icu_data_path,
      engine_properties.aot_library_path,
      *state->display);

  if (!state->application->IsValid())
  {
    LogE("Could not initialize the Flutter application.");
    return nullptr;
  }

  if (!state->application->SetWindowSize(state->display->GetWidth(), state->display->GetHeight()))
  {
    LogE("Could not update the Flutter application size.");
    return nullptr;
  }

  return state;
}

FLUTTER_EXPORT bool StopFlutterApplication(FlutterApplicationRef application)
{
  if (!application)
    return false;

  if (application->display)
  {
    application->display.reset();
  }
  if (application->application)
  {
    application->application.reset();
  }
  delete application;

  return true;
}
