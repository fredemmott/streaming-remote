/******************************************************************************
    Copyright (C) 2020 by Hugh Bailey <obs.jim@gmail.com>
    Copyright (C) 2021 by Fred Emmott <fred@fredemmott.com>

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License version 2 as published
    by the Free Software Foundation.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.
******************************************************************************

Note: this file contains code derived from OBS Studio, but unlike OBS Studio,
is licensed only under version 2 of the GNU General Public License,
*not* "or any later version".

******************************************************************************/

#include "OBS.h"

#include "Core/AwaitablePromise.h"

#include <obs.h>
#include <obs.hpp>

#include <QBuffer>
#include <QImage>
#include <QScopeGuard>

#define SCOPE_EXIT_IMPL(id, x) const auto SCOPE_GUARD_ ## id = \
  qScopeGuard(x)
#define SCOPE_EXIT_IMPL_WRAP(id, x) SCOPE_EXIT_IMPL(id, x)
#define SCOPE_EXIT(x) SCOPE_EXIT_IMPL_WRAP(__COUNTER__, x)

namespace {
  typedef AwaitablePromise<float> TickPromise;

  void resolve_promise_on_tick(void* untyped_promise, float seconds) {
    reinterpret_cast<TickPromise*>(untyped_promise)->resolve(seconds);
  }

  asio::awaitable<void> next_tick(asio::io_context& ctx) {
    TickPromise p(ctx);

    obs_add_tick_callback(&resolve_promise_on_tick, &p);
    SCOPE_EXIT([&]() { obs_remove_tick_callback(&resolve_promise_on_tick, &p); });
    co_await p.async_wait();
  }

  // Based on obs-studio/UI/window-basic-main-screenshot.cpp
  asio::awaitable<std::string> make_source_thumbnail(asio::io_context& ctx, OBSSource source) {
    LOG_FUNCTION();
    gs_texrender_t* texrender = nullptr;
    SCOPE_EXIT([&]() { gs_texrender_destroy(texrender); });
    gs_stagesurf_t* stagesurface = nullptr;
    SCOPE_EXIT([&]() { gs_stagesurface_destroy(stagesurface); });

    const auto width = obs_source_get_base_width(source);
    const auto height = obs_source_get_base_height(source);

    co_await next_tick(ctx);
    {
      obs_enter_graphics();
      SCOPE_EXIT([]() { obs_leave_graphics(); });


      texrender = gs_texrender_create(GS_RGBA, GS_ZS_NONE);
      stagesurface = gs_stagesurface_create(width, height, GS_RGBA);
      gs_texrender_reset(texrender);

      if (!gs_texrender_begin(texrender, width, height)) {
        Logger::debug("Failed to begin texrender");
        co_return std::string();
      }
      SCOPE_EXIT([&]() { gs_texrender_end(texrender); });
      vec4 zero;
      vec4_zero(&zero);
      gs_clear(GS_CLEAR_COLOR, &zero, 0.0f, 0);
      gs_ortho(0.0f, (float) width, 0.0f, (float)height, -100.0f, 100.0f);
      gs_blend_state_push();
      SCOPE_EXIT([]() { gs_blend_state_pop(); });
      gs_blend_function(GS_BLEND_ONE, GS_BLEND_ZERO);

      obs_source_inc_showing(source);
      SCOPE_EXIT([=]() { obs_source_dec_showing(source); });
      obs_source_video_render(source);
    }
    co_await next_tick(ctx);
    {
      obs_enter_graphics();
      SCOPE_EXIT([]() { obs_leave_graphics(); });
      gs_stage_texture(stagesurface, gs_texrender_get_texture(texrender));
    }
    co_await next_tick(ctx);
    {
      obs_enter_graphics();
      SCOPE_EXIT([]() { obs_leave_graphics(); });
      uint8_t* video_data = nullptr;
      uint32_t video_linesize = 0;
      if (!gs_stagesurface_map(stagesurface, &video_data, &video_linesize)) {
        Logger::debug("Failed to map stagesurface");
        co_return std::string();
      }
      SCOPE_EXIT([&]() { gs_stagesurface_unmap(stagesurface); });
      QImage image(width, height, QImage::Format::Format_RGBX8888);
      image.fill(0);
      int linesize = image.bytesPerLine();
      for (int y = 0; y < (int)height; y++) {
        memcpy(
          image.scanLine(y),
          video_data + (y * video_linesize),
          linesize
        );
      }

      QByteArray buf;
      QBuffer buf_device(&buf);
      image.save(&buf_device, "PNG");
      co_return buf.toBase64().toStdString();
    }
    co_return std::string();
  }
}

asio::awaitable<std::string> OBS::getSceneThumbnailAsBase64Png(const std::string& id) {
  LOG_FUNCTION();
  obs_frontend_source_list sources {};
  SCOPE_EXIT([&]() { obs_frontend_source_list_free(&sources); });
  obs_frontend_get_scenes(&sources);
  for (size_t i = 0; i < sources.sources.num; i++) {
    const auto source = sources.sources.array[i];
    if (id != obs_source_get_name(source)) {
      continue;
    }
    co_return co_await make_source_thumbnail(getIoContext(), source);
  }

  co_return std::string();
}
