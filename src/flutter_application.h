// Copyright 2018 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#pragma once

#include <flutter_embedder.h>
#include <functional>
#include <vector>
#define EFL_BETA_API_SUPPORT
#include <Ecore_Wl2.h>
#include <Ecore_Input.h>

#include "vsync_waiter.h"

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
                       const std::vector<const char*> &args,
                       RenderDelegate &render_delegate);
    virtual ~FlutterApplication();
    bool IsValid() const;
    bool SetWindowSize(size_t width, size_t height);

  private:
    bool valid_;
    RenderDelegate &render_delegate_;
    FlutterEngine engine_ = nullptr;

    std::unique_ptr<VsyncWaiter> vsync_waiter_;

    std::vector<Ecore_Event_Handler *> pointer_event_handlers_;
    bool pointer_state_ = false;

    void SendFlutterPointerEvent(FlutterPointerPhase phase, double x, double y, size_t timestamp);
    static Eina_Bool OnPointerEvent(void *data, int type, void *event);

    // Disallow copy and assign operations.
    FlutterApplication(const FlutterApplication &) = delete;
    void operator=(const FlutterApplication &) = delete;
  };

} // namespace flutter
