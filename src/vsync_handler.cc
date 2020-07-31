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

#include "vsync_handler.h"
#include "logger.h"

VsyncHandler::VsyncHandler()
{
  tdm_error ret;
  client_ = tdm_client_create(&ret);
  if (ret != TDM_ERROR_NONE)
  {
    LogE("tdm_client_create has failed.");
    return;
  }

  output_ = tdm_client_get_output(client_, const_cast<char *>("default"), &ret);
  if (ret != TDM_ERROR_NONE)
  {
    LogE("tdm_client_get_output has failed.");
    return;
  }

  vblank_ = tdm_client_output_create_vblank(output_, &ret);
  if (ret != TDM_ERROR_NONE)
  {
    LogE("tdm_client_output_create_vblank has failed.");
    return;
  }

  std::thread thread(BeginVblankEventLoop, this);
  thread.join();
}

void VsyncHandler::BeginVblankEventLoop(void *data)
{
  VsyncHandler *handler = reinterpret_cast<VsyncHandler *>(data);

  if (!ecore_init())
  {
    LogE("Could not initialize ecore.");
    return;
  }

  handler->vblank_ecore_pipe_ = ecore_pipe_add(VblankEventLoopCallback, handler);

  ecore_main_loop_begin();
  ecore_shutdown();
}

void VsyncHandler::VblankEventLoopCallback(void *data, void *buffer, unsigned int nbyte)
{
  VsyncHandler *handler = reinterpret_cast<VsyncHandler *>(data);

  int *event_type = reinterpret_cast<int *>(buffer);
  if ((*event_type) == VBLANK_LOOP_REQUEST)
  {
    handler->AsyncWaitForVsyncCallback();
  }
  else if ((*event_type) == VBLANK_LOOP_DEL_PIPE)
  {
    handler->DeleteVblankEventPipe();
  }
}

void VsyncHandler::AsyncWaitForVsyncCallback()
{
  tdm_error ret;
  ret = tdm_client_vblank_wait(vblank_, 1, TdmClientVblankCallback, this);
  if (ret != TDM_ERROR_NONE)
  {
    LogW("tdm_client_vblank_wait has returned an error.");
    return;
  }

  tdm_client_handle_events(client_);
}

void VsyncHandler::DeleteVblankEventPipe()
{
  if (vblank_ecore_pipe_)
  {
    ecore_pipe_del(vblank_ecore_pipe_);
    vblank_ecore_pipe_ = NULL;
  }

  ecore_main_loop_quit();
}

void VsyncHandler::TdmClientVblankCallback(tdm_client_vblank *vblank,
                                           tdm_error error,
                                           unsigned int sequence,
                                           unsigned int tv_sec,
                                           unsigned int tv_usec,
                                           void *user_data)
{
  VsyncHandler *handler = reinterpret_cast<VsyncHandler *>(user_data);

  uint64_t frame_start_time_nanos = tv_sec * 1e9 + tv_usec * 1e3;
  uint64_t frame_target_time_nanos = 16.6 * 1e6 + frame_start_time_nanos;

  FlutterEngineOnVsync(handler->engine_, handler->baton_, frame_start_time_nanos, frame_target_time_nanos);
}

void VsyncHandler::AsyncWaitForVsync(intptr_t baton)
{
  baton_ = baton;

  if (!engine_)
  {
    LogW("The Flutter engine has not been initialized.");
    return;
  }

  if (vblank_ecore_pipe_)
  {
    int event_type = VBLANK_LOOP_REQUEST;
    ecore_pipe_write(vblank_ecore_pipe_, &event_type, sizeof(event_type));
  }

  return;
}

void VsyncHandler::AsyncWaitForRunEngineSuccess(FlutterEngine &engine)
{
  engine_ = engine;

  if (baton_ == 0)
  {
    return;
  }

  AsyncWaitForVsync(baton_);
}

VsyncHandler::~VsyncHandler()
{
  if (vblank_ecore_pipe_)
  {
    int event_type = VBLANK_LOOP_DEL_PIPE;
    ecore_pipe_write(vblank_ecore_pipe_, &event_type, sizeof(event_type));
  }

  if (vblank_)
  {
    tdm_client_vblank_destroy(vblank_);
  }

  if (client_)
  {
    tdm_client_destroy(client_);
  }
}
