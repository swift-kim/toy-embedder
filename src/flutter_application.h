// Copyright 2018 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#pragma once

#include <flutter_embedder.h>
#include <functional>
#include <vector>

namespace flutter
{
  class FlutterApplication
  {
  public:
    class RenderDelegate
    {
    public:
      virtual bool OnApplicationContextMakeCurrent() = 0;
      virtual bool OnApplicationContextMakeResourceCurrent() = 0;
      virtual bool OnApplicationContextClearCurrent() = 0;
      virtual bool OnApplicationPresent() = 0;
      virtual uint32_t OnApplicationGetOnscreenFBO() = 0;
      virtual void *GetProcAddress(const char *) = 0;
    };

    FlutterApplication(std::string bundle_path,
                       std::string icu_data_path,
                       const std::vector<std::string> &args,
                       RenderDelegate &render_delegate);
    ~FlutterApplication();
    bool IsValid() const;
    void ProcessEvents();
    bool SetWindowSize(size_t width, size_t height);
    bool SendPointerEvent(int button, int x, int y);
    void ReadInputEvents();

  private:
    bool valid_;
    RenderDelegate &render_delegate_;
    FlutterEngine engine_ = nullptr;
    int last_button_ = 0;

    bool SendFlutterPointerEvent(FlutterPointerPhase phase, double x, double y);

    // Disallow copy and assign operations.
    FlutterApplication(const FlutterApplication &) = delete;
    void operator=(const FlutterApplication &) = delete;
  };

} // namespace flutter
