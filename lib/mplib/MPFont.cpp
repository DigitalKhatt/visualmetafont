#include "MPFont.h"

#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <format>
#include <iostream>
#include <stdexcept>
#include <unistd.h>

void MPFont::shipoutBackend(MP mp, void* edge) {
  static_cast<MPFont*>(mp->userdata)->shipout(edge);
}

char* MPFont::findFile(MP, const char* name, const char* mode, int type) {
  if (mode[0] != 'r' || !access(name, R_OK) || type) return mp_strdup(name);
  return nullptr;
}

MPFont::~MPFont() { clear(); }

void MPFont::clear() {
  if (mp_) {
    for (auto& [code, info] : edges_) {
      mp_gr_toss_objects_extended(info.currentPicture);
      for (auto& [name, picture] : info.controlledPictures)
        if (picture) mp_gr_toss_objects_extended(picture);
    }
    mp_finish(mp_);
    mp_ = nullptr;
  }
  edges_.clear();
  glyphSources_.clear();
  controlledPictureNames_.clear();
  axes_.clear();
}

bool MPFont::initialize(std::string source, std::filesystem::path projectFile) {
  clear();
  projectFile_ = std::filesystem::absolute(std::move(projectFile));
  projectDirectory_ = projectFile_.parent_path();
  fontName_ = projectFile_.stem().string();

  MP_options* options = mp_options();
  options->noninteractive = 1;
  options->command_line = nullptr;
  options->ini_version = true;
  options->math_mode = mp_math_double_mode;
  options->job_name = const_cast<char*>("VisualMetaFont");
  options->shipout_backend = shipoutBackend;
  options->userdata = this;
  options->find_file = findFile;
  mp_ = mp_initialize(options);
  if (!mp_) throw std::runtime_error("Could not initialize MetaPost");

  const auto previous = std::filesystem::current_path();
  std::filesystem::current_path(projectDirectory_);
  try {
    execute(source);
    std::filesystem::current_path(previous);
  } catch (...) {
    std::filesystem::current_path(previous);
    throw;
  }

  if (mp_->job_name) mp_xfree(mp_->job_name);
  mp_->job_name = strdup(fontName_.c_str());
  readAxes();
  return true;
}

std::string MPFont::execute(std::string_view command) {
  if (!mp_) throw std::runtime_error("MetaPost is not initialized");
  mp_->history = mp_spotless;
  const int status = mp_execute(mp_, const_cast<char*>(command.data()), command.size());
  const mp_run_data* results = mp_rundata(mp_);
  std::string output = results && results->term_out.data ? results->term_out.data : "";
  if (status == mp_error_message_issued || status == mp_fatal_error_stop)
    throw std::runtime_error(output);
  return output;
}

double MPFont::numericVariable(std::string_view name) const {
  double value = 0;
  std::string key{name};
  getMPNumVariable(mp_, key.data(), &value);
  return value;
}

double MPFont::internalNumericVariable(std::string_view name) const {
  std::string key{name};
  return mp_get_numeric_internal(mp_, key.data());
}

bool MPFont::boolVariable(std::string_view name) const {
  int value = 0;
  std::string key{name};
  getMPBoolVariable(mp_, key.data(), &value);
  return value == 1;
}

bool MPFont::pairVariable(std::string_view name, double& x, double& y) const {
  std::string key{name};
  return getMPPairVariable(mp_, key.data(), &x, &y);
}

std::string MPFont::stringVariable(std::string_view name) const {
  char* value = nullptr;
  std::string key{name};
  return getMPStringVariable(mp_, key.c_str(), &value) && value ? value : "";
}

std::vector<mp_edge_object*> MPFont::edges() const {
  std::vector<mp_edge_object*> result;
  result.reserve(edges_.size());
  for (const auto& [code, info] : edges_) result.push_back(info.currentPicture);
  return result;
}

mp_edge_object* MPFont::edge(int code) const {
  const auto it = edges_.find(code);
  return it == edges_.end() ? nullptr : it->second.currentPicture;
}

MPFontGlyphInfo MPFont::glyphInfo(int code) const {
  const auto it = edges_.find(code);
  return it == edges_.end() ? MPFontGlyphInfo{} : it->second;
}

void MPFont::shipout(void* value) {
  auto* edge = convert_to_edge(mp_, value);
  if (!edge) return;
  MPFontGlyphInfo info{edge, {}};
  for (const auto& name : controlledPictureNames_) {
    mp_edge_object* picture = nullptr;
    if (getMPPictureVariable(mp_, name.c_str(), &picture))
      info.controlledPictures.emplace(name, picture);
  }
  auto [it, inserted] = edges_.try_emplace(edge->charcode, std::move(info));
  if (!inserted) {
    mp_gr_toss_objects_extended(it->second.currentPicture);
    for (auto& [name, picture] : it->second.controlledPictures)
      if (picture) mp_gr_toss_objects_extended(picture);
    it->second = MPFontGlyphInfo{edge, {}};
    for (const auto& name : controlledPictureNames_) {
      mp_edge_object* picture = nullptr;
      if (getMPPictureVariable(mp_, name.c_str(), &picture))
        it->second.controlledPictures.emplace(name, picture);
    }
  }
}

void MPFont::setControlledPictureNames(std::vector<std::string> names) {
  controlledPictureNames_ = std::move(names);
}

void MPFont::clearGlyphSources() { glyphSources_.clear(); }

void MPFont::registerGlyphSource(std::string name, std::string source,
                                 std::string beginMacro, int unicode) {
  glyphSources_.insert_or_assign(std::move(name),
      GlyphSource{std::move(source), std::move(beginMacro), unicode});
}

bool MPFont::hasGlyph(std::string_view name) const {
  return glyphSources_.contains(std::string{name});
}

void MPFont::generateAlternate(std::string_view name, double left, double right,
                               double third, double fourth, double fifth,
                               double scaleX, std::string_view source,
                               int alternateCode) {
  char buffer[256];
  std::snprintf(buffer, sizeof(buffer),
      "save params;params0:=%.9g;params1:=%.9g;params3:=%.9g;params4:=%.9g;params5:=%.9g;params100:=%.9g;",
      left, right, third, fourth, fifth, scaleX);
  std::string prefix{buffer};
  if (!source.empty()) {
    execute(prefix + std::string{source});
    return;
  }
  if (left != 0 || right != 0) {
    execute(prefix + "generateAlternate(" + std::string{name} + "$,params);");
    return;
  }
  const auto it = glyphSources_.find(std::string{name});
  if (it == glyphSources_.end()) throw std::runtime_error("Unknown glyph source");
  std::string glyphSource = it->second.source;
  if (scaleX == 0) {
    const std::string begin = it->second.beginMacro + "(" + std::string{name} + "," +
                              std::to_string(it->second.unicode);
    const std::string replacement = it->second.beginMacro + "(alternatechar," +
                                    std::to_string(alternateCode);
    if (const auto pos = glyphSource.find(begin); pos != std::string::npos)
      glyphSource.replace(pos, begin.size(), replacement);
  }
  execute(prefix + glyphSource);
}

mp_graphic_object* MPFont::copyBody(const mp_graphic_object* body) const {
  mp_graphic_object* result = nullptr;
  mp_graphic_object* tail = nullptr;
  auto copyPath = [this](mp_gr_knot knot) {
    if (!knot) return static_cast<mp_gr_knot>(nullptr);
    mp_gr_knot first = nullptr, last = nullptr;
    auto* point = knot;
    do {
      auto* copy = static_cast<mp_gr_knot>(mp_xmalloc(mp_, 1, sizeof(mp_gr_knot_data)));
      *copy = *point;
      copy->next = nullptr;
      if (!first) first = copy; else last->next = copy;
      last = copy;
      point = point->next;
    } while (point && point != knot);
    if (point == knot) last->next = first;
    return first;
  };
  for (; body; body = body->next) {
    mp_graphic_object* next = nullptr;
    if (body->type == mp_fill_code) {
      const auto* src = reinterpret_cast<const mp_fill_object*>(body);
      auto* dst = reinterpret_cast<mp_fill_object*>(mp_new_graphic_object(mp_, mp_fill_code));
      dst->path_p = copyPath(src->path_p); dst->next = nullptr;
      dst->pre_script = src->pre_script ? strdup(src->pre_script) : nullptr;
      dst->post_script = src->post_script ? strdup(src->post_script) : nullptr;
      dst->pen_p = nullptr; dst->htap_p = nullptr;
      dst->color_model = src->color_model; dst->color = src->color;
      next = reinterpret_cast<mp_graphic_object*>(dst);
    } else if (body->type == mp_stroked_code) {
      const auto* src = reinterpret_cast<const mp_stroked_object*>(body);
      auto* dst = reinterpret_cast<mp_stroked_object*>(mp_new_graphic_object(mp_, mp_stroked_code));
      dst->path_p = copyPath(src->path_p); dst->next = nullptr;
      dst->pre_script = src->pre_script ? strdup(src->pre_script) : nullptr;
      dst->post_script = src->post_script ? strdup(src->post_script) : nullptr;
      dst->pen_p = nullptr; dst->color_model = src->color_model; dst->color = src->color;
      next = reinterpret_cast<mp_graphic_object*>(dst);
    }
    if (!next) continue;
    if (!result) result = next; else tail->next = next;
    tail = next;
  }
  return result;
}

std::string MPFont::familyName() const { return stringVariable("nametable familyName"); }
std::string MPFont::copyright() const { return stringVariable("nametable copyright"); }
std::string MPFont::log() const {
  const auto* data = mp_ ? mp_rundata(mp_) : nullptr;
  return data && data->term_out.data ? data->term_out.data : "";
}

static std::uint32_t tagFromString(std::string_view tag) {
  std::uint32_t value = 0;
  for (int i = 0; i < 4; ++i) value = (value << 8) | (i < tag.size() ? tag[i] : ' ');
  return value;
}

void MPFont::readAxes() {
  axes_.clear();
  const int count = static_cast<int>(internalNumericVariable("number_of_axes"));
  for (int i = 0; i < count; ++i) {
    MPAxis axis;
    const auto base = std::format("axes {} ", i);
    axis.name = stringVariable(base + "name");
    axis.axisTag = tagFromString(stringVariable(base + "tag"));
    axis.equivExpr = stringVariable(base + "equivExpr");
    axis.minValue = numericVariable(base + "minValue");
    axis.defaultValue = numericVariable(base + "defaultValue");
    axis.maxValue = numericVariable(base + "maxValue");
    axes_.push_back(std::move(axis));
  }
}
