// Copyright 2018 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "flutter_application.h"

#include <sys/types.h>
#include <unistd.h>
#include <chrono>
#include <climits>
#include <sstream>
#include <vector>

#include "utils.h"

namespace flutter
{
  static_assert(FLUTTER_ENGINE_VERSION == 1, "");

  FlutterApplication::FlutterApplication(
      std::string bundle_path,
      std::string icu_data_path,
      const std::vector<std::string> &command_line_args,
      RenderDelegate &render_delegate)
      : render_delegate_(render_delegate),
        vsync_handler_(std::make_unique<VsyncHandler>())
  {
    if (!FileExistsAtPath(bundle_path))
    {
      LogE("Could not find Flutter asset bundle.");
      return;
    }

    if (!FileExistsAtPath(icu_data_path))
    {
      LogE("Could not find ICU data.");
      return;
    }

    FlutterRendererConfig config = {};
    config.type = kOpenGL;
    config.open_gl.struct_size = sizeof(config.open_gl);
    config.open_gl.make_current = [](void *data) -> bool {
      return reinterpret_cast<FlutterApplication *>(data)->render_delegate_.OnApplicationContextMakeCurrent();
    };
    config.open_gl.make_resource_current = [](void *data) -> bool {
      return reinterpret_cast<FlutterApplication *>(data)->render_delegate_.OnApplicationContextMakeResourceCurrent();
    };
    config.open_gl.clear_current = [](void *data) -> bool {
      return reinterpret_cast<FlutterApplication *>(data)->render_delegate_.OnApplicationContextClearCurrent();
    };
    config.open_gl.present = [](void *data) -> bool {
      return reinterpret_cast<FlutterApplication *>(data)->render_delegate_.OnApplicationPresent();
    };
    config.open_gl.fbo_callback = [](void *data) -> uint32_t {
      return reinterpret_cast<FlutterApplication *>(data)->render_delegate_.OnApplicationGetOnscreenFBO();
    };
    config.open_gl.gl_proc_resolver = [](void *data, const char *name) -> void * {
      return reinterpret_cast<FlutterApplication *>(data)->render_delegate_.GetProcAddress(name);
    };

    std::vector<const char *> command_line_args_c;
    for (const auto &arg : command_line_args)
    {
      command_line_args_c.push_back(arg.c_str());
    }

    FlutterProjectArgs args = {
        .struct_size = sizeof(FlutterProjectArgs),
        .assets_path = bundle_path.c_str(),
        .icu_data_path = icu_data_path.c_str(),
        .command_line_argc = static_cast<int>(command_line_args_c.size()),
        .command_line_argv = command_line_args_c.data(),
        .vsync_callback = [](void *data, intptr_t baton) -> void {
          reinterpret_cast<FlutterApplication *>(data)->vsync_handler_->AsyncWaitForVsync(baton);
        },
    };

    auto result = FlutterEngineRun(FLUTTER_ENGINE_VERSION, &config, &args, this, &engine_);
    if (result != kSuccess)
    {
      LogE("Could not start the Flutter engine.");
      return;
    }

    vsync_handler_->AsyncWaitForRunEngineSuccess(engine_);

    pointer_event_handlers_.push_back(ecore_event_handler_add(ECORE_EVENT_MOUSE_BUTTON_DOWN, OnPointerEvent, this));
    pointer_event_handlers_.push_back(ecore_event_handler_add(ECORE_EVENT_MOUSE_BUTTON_UP, OnPointerEvent, this));
    pointer_event_handlers_.push_back(ecore_event_handler_add(ECORE_EVENT_MOUSE_MOVE, OnPointerEvent, this));

    valid_ = true;
  }

  bool FlutterApplication::IsValid() const { return valid_; }

  bool FlutterApplication::SetWindowSize(size_t width, size_t height)
  {
    FlutterWindowMetricsEvent event = {};
    event.struct_size = sizeof(event);
    event.width = width;
    event.height = height;
    event.pixel_ratio = 1.5;
    return FlutterEngineSendWindowMetricsEvent(engine_, &event) == kSuccess;
  }

  void FlutterApplication::SendFlutterPointerEvent(FlutterPointerPhase phase, double x, double y, size_t timestamp)
  {
    FlutterPointerEvent event = {};
    event.struct_size = sizeof(event);
    event.phase = phase;
    event.x = x;
    event.y = y;
    event.timestamp = timestamp;
    FlutterEngineSendPointerEvent(engine_, &event, 1);
  }

  Eina_Bool FlutterApplication::OnPointerEvent(void *data, int type, void *event)
  {
    auto *app = reinterpret_cast<FlutterApplication *>(data);

    if (type == ECORE_EVENT_MOUSE_BUTTON_DOWN)
    {
      app->pointer_state_ = true;

      auto *buttonEvent = reinterpret_cast<Ecore_Event_Mouse_Button *>(event);
      app->SendFlutterPointerEvent(kDown, buttonEvent->x, buttonEvent->y, buttonEvent->timestamp);
    }
    else if (type == ECORE_EVENT_MOUSE_BUTTON_UP)
    {
      app->pointer_state_ = false;

      auto *buttonEvent = reinterpret_cast<Ecore_Event_Mouse_Button *>(event);
      app->SendFlutterPointerEvent(kUp, buttonEvent->x, buttonEvent->y, buttonEvent->timestamp);
    }
    else if (type == ECORE_EVENT_MOUSE_MOVE)
    {
      if (app->pointer_state_)
      {
        auto *moveEvent = reinterpret_cast<Ecore_Event_Mouse_Move *>(event);
        app->SendFlutterPointerEvent(kMove, moveEvent->x, moveEvent->y, moveEvent->timestamp);
      }
    }

    return ECORE_CALLBACK_PASS_ON;
  }

  FlutterApplication::~FlutterApplication()
  {
    for (auto handler : pointer_event_handlers_)
    {
      ecore_event_handler_del(handler);
    }
    pointer_event_handlers_.clear();

    if (engine_)
    {
      auto result = FlutterEngineShutdown(engine_);
      if (result != kSuccess)
      {
        LogE("Could not shutdown the Flutter engine.");
      }
    }
  }

} // namespace flutter
