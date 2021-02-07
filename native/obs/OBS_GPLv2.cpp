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
  // Based on obs-studio/UI/window-basic-main-screenshot.cpp
  std::string make_source_thumbnail(OBSSource source) {
    LOG_FUNCTION();
    obs_enter_graphics();
    SCOPE_EXIT([]() { obs_leave_graphics(); });

    const auto width = obs_source_get_base_width(source);
    const auto height = obs_source_get_base_height(source);

    auto texrender = gs_texrender_create(GS_RGBA, GS_ZS_NONE);
    SCOPE_EXIT([&]() { gs_texrender_destroy(texrender); });
    auto stagesurface = gs_stagesurface_create(width, height, GS_RGBA);
    SCOPE_EXIT([&]() { gs_stagesurface_destroy(stagesurface); });

    if (!gs_texrender_begin(texrender, width, height)) {
      Logger::debug("Failed to begin texrender");
      return std::string();
    }
    {
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
    {
      gs_stage_texture(stagesurface, gs_texrender_get_texture(texrender));

      uint8_t* video_data = nullptr;
      uint32_t video_linesize = 0;
      if (!gs_stagesurface_map(stagesurface, &video_data, &video_linesize)) {
        Logger::debug("Failed to map stagesurface");
        return std::string();
      }
      SCOPE_EXIT([&]() { gs_stagesurface_unmap(stagesurface); });
      QImage image(width, height, QImage::Format::Format_RGBX8888);
      int linesize = image.bytesPerLine();
      for (int y = 0; y < (int)height; y++) {
        memcpy(
          image.scanLine(y),
          video_data + (y * video_linesize),
          linesize
        );
      }

      Logger::debug("Returning png data");
      QByteArray buf;
      QBuffer buf_device(&buf);
      image.save(&buf_device, "PNG");
      return buf.toBase64().toStdString();
    }
    Logger::debug("Reached end?");
    return std::string();
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
    co_return make_source_thumbnail(source);
  }

  co_return std::string();
}
