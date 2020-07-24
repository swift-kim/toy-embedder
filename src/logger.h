// Copyright 2018 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#pragma once

#include <iostream>
#include <dlog.h>

// dlog is a preferred logging mechanism over stdout on Tizen.
#undef LOG_TAG
#undef LOG_

#define LOG_TAG "FLUTTER_EMBEDDER"
#define LOG_(prio, fmt, arg...) \
  __dlog_print(LOG_ID_MAIN, prio, LOG_TAG, "%s: %s(%d) > " fmt, __FILE__, __func__, __LINE__, ##arg);

#define LogD(fmt, args...) LOG_(DLOG_DEBUG, fmt, ##args)
#define LogI(fmt, args...) LOG_(DLOG_INFO, fmt, ##args)
#define LogW(fmt, args...) LOG_(DLOG_WARN, fmt, ##args)
#define LogE(fmt, args...) LOG_(DLOG_ERROR, fmt, ##args)
