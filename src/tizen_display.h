// Copyright 2018 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#pragma once

#include <EGL/egl.h>
#define EFL_BETA_API_SUPPORT
#include <Ecore_Wl2.h>

#include "flutter_application.h"

namespace flutter
{
  class TizenDisplay : public FlutterApplication::RenderDelegate
  {
  public:
    TizenDisplay(uint32_t display_width, uint32_t display_height);
    virtual ~TizenDisplay();
    bool IsValid() const;
    size_t GetWidth() const;
    size_t GetHeight() const;

  private:
    int32_t display_width_ = 0;
    int32_t display_height_ = 0;
    EGLDisplay display_ = EGL_NO_DISPLAY;
    EGLContext context_ = EGL_NO_CONTEXT;
    EGLContext resource_context_ = EGL_NO_CONTEXT;
    EGLSurface surface_ = EGL_NO_SURFACE;

    Ecore_Wl2_Egl_Window *egl_window_ = nullptr;
    Ecore_Wl2_Display *wl2_display_ = nullptr;
    Ecore_Wl2_Window *wl2_window_ = nullptr;

    bool valid_ = false;

    // |FlutterApplication::RenderDelegate|
    bool OnApplicationContextMakeCurrent() override;
    // |FlutterApplication::RenderDelegate|
    bool OnApplicationContextMakeResourceCurrent() override;
    // |FlutterApplication::RenderDelegate|
    bool OnApplicationContextClearCurrent() override;
    // |FlutterApplication::RenderDelegate|
    bool OnApplicationPresent() override;
    // |FlutterApplication::RenderDelegate|
    uint32_t OnApplicationGetOnscreenFBO() override;
    // |FlutterApplication::RenderDelegate|
    void *GetProcAddress(const char *) override;

    // Disallow copy and assign operations.
    TizenDisplay(const TizenDisplay &) = delete;
    void operator=(const TizenDisplay &) = delete;
  };

} // namespace flutter
