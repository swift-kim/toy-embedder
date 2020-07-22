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
      : render_delegate_(render_delegate)
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
    };

    auto result = FlutterEngineRun(FLUTTER_ENGINE_VERSION, &config, &args, this, &engine_);

    if (result != kSuccess)
    {
      LogE("Could not start the Flutter engine.");
      return;
    }

    valid_ = true;
  }

  FlutterApplication::~FlutterApplication()
  {
    if (engine_ == nullptr)
    {
      return;
    }

    auto result = FlutterEngineShutdown(engine_);

    if (result != kSuccess)
    {
      LogE("Could not shutdown the Flutter engine.");
    }
  }

  bool FlutterApplication::IsValid() const { return valid_; }

  bool FlutterApplication::SetWindowSize(size_t width, size_t height)
  {
    FlutterWindowMetricsEvent event = {};
    event.struct_size = sizeof(event);
    event.width = width;
    event.height = height;
    event.pixel_ratio = 1.0;
    return FlutterEngineSendWindowMetricsEvent(engine_, &event) == kSuccess;
  }

  void FlutterApplication::ProcessEvents()
  {
    __FlutterEngineFlushPendingTasksNow();
  }

  bool FlutterApplication::SendPointerEvent(int button, int x, int y)
  {
    if (!valid_)
    {
      LogE("Pointer events on an invalid application.");
      return false;
    }

    // Simple hover event. Nothing to do.
    if (last_button_ == 0 && button == 0)
    {
      return true;
    }

    FlutterPointerPhase phase = kCancel;

    if (last_button_ == 0 && button != 0)
    {
      phase = kDown;
    }
    else if (last_button_ == button)
    {
      phase = kMove;
    }
    else
    {
      phase = kUp;
    }

    last_button_ = button;
    return SendFlutterPointerEvent(phase, x, y);
  }

  bool FlutterApplication::SendFlutterPointerEvent(FlutterPointerPhase phase, double x, double y)
  {
    FlutterPointerEvent event = {};
    event.struct_size = sizeof(event);
    event.phase = phase;
    event.x = x;
    event.y = y;
    event.timestamp =
        std::chrono::duration_cast<std::chrono::microseconds>(
            std::chrono::high_resolution_clock::now().time_since_epoch())
            .count();
    return FlutterEngineSendPointerEvent(engine_, &event, 1) == kSuccess;
  }

  void FlutterApplication::ReadInputEvents()
  {
    // TODO(chinmaygarde): Fill this in for touch screen and not just devices that
    // fake mice.
    ::sleep(INT_MAX);
  }

} // namespace flutter
