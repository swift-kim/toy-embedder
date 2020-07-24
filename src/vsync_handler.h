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

#pragma once

#include <flutter_embedder.h>
#include <thread>
#include <tdm_client.h>
#include <Ecore.h>

class VsyncHandler
{
public:
  VsyncHandler();
  ~VsyncHandler();
  void AsyncWaitForVsync(intptr_t baton);
  void AsyncWaitForRunEngineSuccess(FlutterEngine &engine);

private:
  static const int VBLANK_LOOP_REQUEST = 1;
  static const int VBLANK_LOOP_DEL_PIPE = 2;

  FlutterEngine engine_;
  intptr_t baton_;

  tdm_client *client_;
  tdm_client_output *output_;
  tdm_client_vblank *vblank_;
  Ecore_Pipe *vblank_ecore_pipe_;

  void AsyncWaitForVsyncCallback();
  void DeleteVblankEventPipe();
  static void BeginVblankEventLoop(void *data);
  static void TdmClientVblankCallback(tdm_client_vblank *vblank,
                                      tdm_error error,
                                      unsigned int sequence,
                                      unsigned int tv_sec,
                                      unsigned int tv_usec,
                                      void *user_data);
  static void VblankEventLoopCallback(void *data, void *buffer, unsigned int nbyte);
};
