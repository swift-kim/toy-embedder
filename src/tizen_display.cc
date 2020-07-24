// Copyright 2018 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "tizen_display.h"
#include "logger.h"

namespace flutter
{
  TizenDisplay::TizenDisplay(uint32_t display_width, uint32_t display_height)
  {
    display_width_ = display_width;
    display_height_ = display_height;

    // Create and initialize a wayland window.
    {
      if (!ecore_wl2_init())
      {
        LogE("Could not initialize the ecore_wl2 library.");
        return;
      }

      wl2_display_ = ecore_wl2_display_connect(nullptr);
      if (!wl2_display_)
      {
        LogE("Could not find an existing wayland display.");
        return;
      }

      wl2_window_ = ecore_wl2_window_new(wl2_display_, nullptr, 0, 0, display_width_, display_height_);
      if (!wl2_window_)
      {
        LogE("Could not create a wayland window.");
        return;
      }

      ecore_wl2_window_type_set(wl2_window_, ECORE_WL2_WINDOW_TYPE_TOPLEVEL);
      ecore_wl2_window_alpha_set(wl2_window_, EINA_FALSE);
      ecore_wl2_window_show(wl2_window_);
      ecore_wl2_window_geometry_set(wl2_window_, 0, 0, display_width_, display_height_);

      egl_window_ = ecore_wl2_egl_window_create(wl2_window_, display_width_, display_height_);
      if (!egl_window_)
      {
        LogE("Could not create a EGL window.");
        return;
      }
    }

    // Setup the EGL Display.
    {
      display_ = ::eglGetDisplay((EGLNativeDisplayType)ecore_wl2_display_get(wl2_display_));
      if (display_ == EGL_NO_DISPLAY)
      {
        LogE("Could not get the EGL display.");
        return;
      }

      if (::eglInitialize(display_, nullptr, nullptr) != EGL_TRUE)
      {
        LogE("Could not initialize the EGL display.");
        return;
      }
    }

    // Choose an EGL config.
    EGLConfig config = {0};
    {
      EGLint num_config = 0;
      const EGLint attribute_list[] = {
          EGL_RED_SIZE, 8,
          EGL_GREEN_SIZE, 8,
          EGL_BLUE_SIZE, 8,
          EGL_ALPHA_SIZE, 8,
          EGL_SURFACE_TYPE, EGL_WINDOW_BIT,
          EGL_NONE};

      if (::eglChooseConfig(display_, attribute_list, &config, 1, &num_config) != EGL_TRUE)
      {
        LogE("Could not choose an EGL config.");
        return;
      }
    }

    // Create the EGL context.
    {
      const EGLint context_attributes[] = {
          EGL_CONTEXT_CLIENT_VERSION,
          2,
          EGL_NONE};

      context_ = ::eglCreateContext(display_, config, EGL_NO_CONTEXT, context_attributes);
      if (context_ == EGL_NO_CONTEXT)
      {
        LogE("Could not create the EGL context.");
        return;
      }

      resource_context_ = ::eglCreateContext(display_, config, context_, context_attributes);
      if (resource_context_ == EGL_NO_CONTEXT)
      {
        LogE("Could not create the EGL resource context.");
        return;
      }
    }

    // Create the EGL window surface.
    {
      void *native_window = ecore_wl2_egl_window_native_get(egl_window_);

      surface_ = ::eglCreateWindowSurface(display_, config, native_window, nullptr);
      if (surface_ == EGL_NO_SURFACE)
      {
        LogE("Could not create EGL surface.");
        return;
      }
    }

    valid_ = true;
  }

  TizenDisplay::~TizenDisplay()
  {
    if (surface_ != EGL_NO_SURFACE)
    {
      ::eglDestroySurface(display_, surface_);
      surface_ = EGL_NO_SURFACE;
    }

    if (context_ != EGL_NO_CONTEXT)
    {
      ::eglDestroyContext(display_, context_);
      context_ = EGL_NO_CONTEXT;
    }

    if (resource_context_ != EGL_NO_CONTEXT)
    {
      ::eglDestroyContext(display_, resource_context_);
      resource_context_ = EGL_NO_CONTEXT;
    }

    if (display_ != EGL_NO_DISPLAY)
    {
      ::eglTerminate(display_);
      display_ = EGL_NO_DISPLAY;
    }

    if (egl_window_)
    {
      ecore_wl2_egl_window_destroy(egl_window_);
      egl_window_ = nullptr;
    }

    if (wl2_window_)
    {
      ecore_wl2_window_free(wl2_window_);
      wl2_window_ = nullptr;
    }

    if (wl2_display_)
    {
      ecore_wl2_display_destroy(wl2_display_);
      wl2_display_ = nullptr;
    }

    ecore_wl2_shutdown();
  }

  bool TizenDisplay::IsValid() const { return valid_; }

  size_t TizenDisplay::GetWidth() const { return display_width_; }

  size_t TizenDisplay::GetHeight() const { return display_height_; }

  // |FlutterApplication::RenderDelegate|
  bool TizenDisplay::OnApplicationContextMakeCurrent()
  {
    if (!valid_)
    {
      LogE("Cannot make an invalid display current.");
      return false;
    }

    if (::eglMakeCurrent(display_, surface_, surface_, context_) != EGL_TRUE)
    {
      LogE("Could not make the context current.");
      return false;
    }

    return true;
  }

  // |FlutterApplication::RenderDelegate|
  bool TizenDisplay::OnApplicationContextMakeResourceCurrent()
  {
    if (!valid_)
    {
      LogE("Cannot make an invalid resource current.");
      return false;
    }

    if (::eglMakeCurrent(display_, EGL_NO_SURFACE, EGL_NO_SURFACE, resource_context_) != EGL_TRUE)
    {
      LogE("Could not make the resource context current.");
      return false;
    }

    return true;
  }

  // |FlutterApplication::RenderDelegate|
  bool TizenDisplay::OnApplicationContextClearCurrent()
  {
    if (!valid_)
    {
      LogE("Cannot clear an invalid display.");
      return false;
    }

    if (::eglMakeCurrent(display_, EGL_NO_SURFACE, EGL_NO_SURFACE, EGL_NO_CONTEXT) != EGL_TRUE)
    {
      LogE("Could not clear the current context.");
      return false;
    }

    return true;
  }

  // |FlutterApplication::RenderDelegate|
  bool TizenDisplay::OnApplicationPresent()
  {
    if (!valid_)
    {
      LogE("Cannot present an invalid display.");
      return false;
    }

    if (::eglSwapBuffers(display_, surface_) != EGL_TRUE)
    {
      LogE("Could not swap buffers to present the screen.");
      return false;
    }

    return true;
  }

  // |FlutterApplication::RenderDelegate|
  uint32_t TizenDisplay::OnApplicationGetOnscreenFBO()
  {
    // Just FBO0.
    return 0;
  }

  // |FlutterApplication::RenderDelegate|
  void *TizenDisplay::GetProcAddress(const char *name)
  {
    if (name == nullptr)
    {
      return nullptr;
    }

    if (auto address = eglGetProcAddress(name))
    {
      return reinterpret_cast<void *>(address);
    }

    return nullptr;
  }

} // namespace flutter
