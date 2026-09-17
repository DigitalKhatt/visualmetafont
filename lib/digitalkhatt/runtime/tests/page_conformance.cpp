#include <digitalkhatt/engine.h>

#include <array>
#include <cmath>
#include <cstdint>
#include <fstream>
#include <future>
#include <iostream>
#include <iterator>
#include <limits>
#include <memory>
#include <stdexcept>
#include <string>
#include <vector>

namespace {

void require(bool condition, const char* message) {
  if (!condition) throw std::runtime_error(message);
}

using Engine = std::unique_ptr<dk_engine_t, decltype(&dk_engine_destroy)>;
using Page = std::unique_ptr<dk_page_t, decltype(&dk_page_destroy)>;

const std::array<std::u16string, 15> kPage43Lines{
    u"\u0671\u0644\u0644\u0651\u064e\u0647\u064f \u0648\u064e\u0644\u0650\u064a\u0651\u064f \u0671\u0644\u0651\u064e\u0630\u0650\u064a\u0646\u064e \u0621\u064e\u0627\u0645\u064e\u0646\u064f\u0648\u0627\u06df \u064a\u064f\u062e\u0652\u0631\u0650\u062c\u064f\u0647\u064f\u0645 \u0645\u0651\u0650\u0646\u064e \u0671\u0644\u0638\u0651\u064f\u0644\u064f\u0645\u064e\u0670\u062a\u0650 \u0627\u0655\u034f\u0650\u0644\u064e\u0649 \u0671\u0644\u0646\u0651\u064f\u0648\u0631\u0650\u06d6",
    u"\u0648\u064e\u0671\u0644\u0651\u064e\u0630\u0650\u064a\u0646\u064e \u0643\u064e\u0641\u064e\u0631\u064f\u0648\u0653\u0627\u06df \u0627\u034f\u0654\u034f\u064e\u0648\u0652\u0644\u0650\u064a\u064e\u0627\u034f\u0653\u0648\u034f\u0654\u034f\u064f\u0647\u064f\u0645\u064f \u0671\u0644\u0637\u0651\u064e\u0670\u063a\u064f\u0648\u062a\u064f \u064a\u064f\u062e\u0652\u0631\u0650\u062c\u064f\u0648\u0646\u064e\u0647\u064f\u0645 \u0645\u0651\u0650\u0646\u064e",
    u"\u0671\u0644\u0646\u0651\u064f\u0648\u0631\u0650 \u0627\u0655\u034f\u0650\u0644\u064e\u0649 \u0671\u0644\u0638\u0651\u064f\u0644\u064f\u0645\u064e\u0670\u062a\u0650\u06d7 \u0627\u034f\u0654\u034f\u064f\u0648\u06df\u0644\u064e\u0670\u0653\u064a\u034f\u0654\u034f\u0650\u0643\u064e \u0627\u034f\u0654\u034f\u064e\u0635\u0652\u062d\u064e\u0670\u0628\u064f \u0671\u0644\u0646\u0651\u064e\u0627\u0631\u0650\u06d6 \u0647\u064f\u0645\u0652 \u0641\u0650\u064a\u0647\u064e\u0627",
    u"\u062e\u064e\u0670\u0644\u0650\u062f\u064f\u0648\u0646\u064e \u06dd\u0662\u0665\u0667 \u0627\u034f\u0654\u034f\u064e\u0644\u064e\u0645\u0652 \u062a\u064e\u0631\u064e \u0627\u0655\u034f\u0650\u0644\u064e\u0649 \u0671\u0644\u0651\u064e\u0630\u0650\u064a \u062d\u064e\u0627\u034f\u0653\u062c\u0651\u064e \u0627\u0655\u034f\u0650\u0628\u0652\u0631\u064e\u0670\u0647\u0650\u0640\u06e7\u0645\u064e \u0641\u0650\u064a \u0631\u064e\u0628\u0651\u0650\u0647\u0650\u06e6\u0653",
    u"\u0627\u034f\u0654\u034f\u064e\u0646\u0652 \u0621\u064e\u0627\u062a\u064e\u0649\u0670\u0647\u064f \u0671\u0644\u0644\u0651\u064e\u0647\u064f \u0671\u0644\u0652\u0645\u064f\u0644\u0652\u0643\u064e \u0627\u0655\u034f\u0650\u0630\u0652 \u0642\u064e\u0627\u0644\u064e \u0627\u0655\u034f\u0650\u0628\u0652\u0631\u064e\u0670\u0647\u0650\u0640\u06e7\u0645\u064f \u0631\u064e\u0628\u0651\u0650\u064a\u064e \u0671\u0644\u0651\u064e\u0630\u0650\u064a \u064a\u064f\u062d\u0652\u064a\u0650\u06e6",
    u"\u0648\u064e\u064a\u064f\u0645\u0650\u064a\u062a\u064f \u0642\u064e\u0627\u0644\u064e \u0627\u034f\u0654\u034f\u064e\u0646\u064e\u0627\u06e0 \u0627\u034f\u0654\u034f\u064f\u062d\u0652\u064a\u0650\u06e6 \u0648\u064e\u0627\u034f\u0654\u034f\u064f\u0645\u0650\u064a\u062a\u064f\u06d6 \u0642\u064e\u0627\u0644\u064e \u0627\u0655\u034f\u0650\u0628\u0652\u0631\u064e\u0670\u0647\u0650\u0640\u06e7\u0645\u064f \u0641\u064e\u0627\u0655\u034f\u0650\u0646\u0651\u064e \u0671\u0644\u0644\u0651\u064e\u0647\u064e \u064a\u064e\u0627\u034f\u0654\u034f\u0652\u062a\u0650\u064a",
    u"\u0628\u0650\u0671\u0644\u0634\u0651\u064e\u0645\u0652\u0633\u0650 \u0645\u0650\u0646\u064e \u0671\u0644\u0652\u0645\u064e\u0634\u0652\u0631\u0650\u0642\u0650 \u0641\u064e\u0627\u034f\u0654\u034f\u0652\u062a\u0650 \u0628\u0650\u0647\u064e\u0627 \u0645\u0650\u0646\u064e \u0671\u0644\u0652\u0645\u064e\u063a\u0652\u0631\u0650\u0628\u0650 \u0641\u064e\u0628\u064f\u0647\u0650\u062a\u064e \u0671\u0644\u0651\u064e\u0630\u0650\u064a",
    u"\u0643\u064e\u0641\u064e\u0631\u064e\u06d7 \u0648\u064e\u0671\u0644\u0644\u0651\u064e\u0647\u064f \u0644\u064e\u0627 \u064a\u064e\u0647\u0652\u062f\u0650\u064a \u0671\u0644\u0652\u0642\u064e\u0648\u0652\u0645\u064e \u0671\u0644\u0638\u0651\u064e\u0670\u0644\u0650\u0645\u0650\u064a\u0646\u064e \u06dd\u0662\u0665\u0668 \u0627\u034f\u0654\u034f\u064e\u0648\u0652 \u0643\u064e\u0671\u0644\u0651\u064e\u0630\u0650\u064a",
    u"\u0645\u064e\u0631\u0651\u064e \u0639\u064e\u0644\u064e\u0649\u0670 \u0642\u064e\u0631\u0652\u064a\u064e\u0629\u08f2 \u0648\u064e\u0647\u0650\u064a\u064e \u062e\u064e\u0627\u0648\u0650\u064a\u064e\u0629\u064c \u0639\u064e\u0644\u064e\u0649\u0670 \u0639\u064f\u0631\u064f\u0648\u0634\u0650\u0647\u064e\u0627 \u0642\u064e\u0627\u0644\u064e \u0627\u034f\u0654\u034f\u064e\u0646\u0651\u064e\u0649\u0670 \u064a\u064f\u062d\u0652\u064a\u0650\u06e6",
    u"\u0647\u064e\u0670\u0630\u0650\u0647\u0650 \u0671\u0644\u0644\u0651\u064e\u0647\u064f \u0628\u064e\u0639\u0652\u062f\u064e \u0645\u064e\u0648\u0652\u062a\u0650\u0647\u064e\u0627\u06d6 \u0641\u064e\u0627\u034f\u0654\u034f\u064e\u0645\u064e\u0627\u062a\u064e\u0647\u064f \u0671\u0644\u0644\u0651\u064e\u0647\u064f \u0645\u0650\u0627\u06df\u064a\u034f\u0654\u034f\u064e\u0629\u064e \u0639\u064e\u0627\u0645\u08f2 \u062b\u064f\u0645\u0651\u064e \u0628\u064e\u0639\u064e\u062b\u064e\u0647\u064f\u06e5\u06d6",
    u"\u0642\u064e\u0627\u0644\u064e \u0643\u064e\u0645\u0652 \u0644\u064e\u0628\u0650\u062b\u0652\u062a\u064e\u06d6 \u0642\u064e\u0627\u0644\u064e \u0644\u064e\u0628\u0650\u062b\u0652\u062a\u064f \u064a\u064e\u0648\u0652\u0645\u064b\u0627 \u0627\u034f\u0654\u034f\u064e\u0648\u0652 \u0628\u064e\u0639\u0652\u0636\u064e \u064a\u064e\u0648\u0652\u0645\u08f2\u06d6 \u0642\u064e\u0627\u0644\u064e \u0628\u064e\u0644",
    u"\u0644\u0651\u064e\u0628\u0650\u062b\u0652\u062a\u064e \u0645\u0650\u0627\u06df\u064a\u034f\u0654\u034f\u064e\u0629\u064e \u0639\u064e\u0627\u0645\u08f2 \u0641\u064e\u0671\u0646\u0638\u064f\u0631\u0652 \u0627\u0655\u034f\u0650\u0644\u064e\u0649\u0670 \u0637\u064e\u0639\u064e\u0627\u0645\u0650\u0643\u064e \u0648\u064e\u0634\u064e\u0631\u064e\u0627\u0628\u0650\u0643\u064e \u0644\u064e\u0645\u0652 \u064a\u064e\u062a\u064e\u0633\u064e\u0646\u0651\u064e\u0647\u0652\u06d6",
    u"\u0648\u064e\u0671\u0646\u0638\u064f\u0631\u0652 \u0627\u0655\u034f\u0650\u0644\u064e\u0649\u0670 \u062d\u0650\u0645\u064e\u0627\u0631\u0650\u0643\u064e \u0648\u064e\u0644\u0650\u0646\u064e\u062c\u0652\u0639\u064e\u0644\u064e\u0643\u064e \u0621\u064e\u0627\u064a\u064e\u0629\u08f0 \u0644\u0651\u0650\u0644\u0646\u0651\u064e\u0627\u0633\u0650\u06d6 \u0648\u064e\u0671\u0646\u0638\u064f\u0631\u0652 \u0627\u0655\u034f\u0650\u0644\u064e\u0649",
    u"\u0671\u0644\u0652\u0639\u0650\u0638\u064e\u0627\u0645\u0650 \u0643\u064e\u064a\u0652\u0641\u064e \u0646\u064f\u0646\u0634\u0650\u0632\u064f\u0647\u064e\u0627 \u062b\u064f\u0645\u0651\u064e \u0646\u064e\u0643\u0652\u0633\u064f\u0648\u0647\u064e\u0627 \u0644\u064e\u062d\u0652\u0645\u08f0\u0627\u06da \u0641\u064e\u0644\u064e\u0645\u0651\u064e\u0627",
    u"\u062a\u064e\u0628\u064e\u064a\u0651\u064e\u0646\u064e \u0644\u064e\u0647\u064f\u06e5 \u0642\u064e\u0627\u0644\u064e \u0627\u034f\u0654\u034f\u064e\u0639\u0652\u0644\u064e\u0645\u064f \u0627\u034f\u0654\u034f\u064e\u0646\u0651\u064e \u0671\u0644\u0644\u0651\u064e\u0647\u064e \u0639\u064e\u0644\u064e\u0649\u0670 \u0643\u064f\u0644\u0651\u0650 \u0634\u064e\u064a\u0652\u0621\u08f2 \u0642\u064e\u062f\u0650\u064a\u0631\u08f1 \u06dd\u0662\u0665\u0669",
};

constexpr std::array<uint64_t, 15> kPage43GlyphCounts{
    99, 88, 87, 85, 94, 104, 95, 82, 89, 98, 88, 95, 88, 74, 86};
constexpr uint64_t kPage43Digest = UINT64_C(0x1e7938ab38e1e661);

std::vector<char> read_file(const char* path) {
  std::ifstream stream(path, std::ios::binary);
  if (!stream) throw std::runtime_error("could not open runtime font");
  return {std::istreambuf_iterator<char>{stream}, {}};
}

struct ExtendedInput {
  dk_page_line_input_v1_t value{};
  uint64_t future = UINT64_C(0xf0e1d2c3b4a59687);
};

struct ExtendedLine {
  dk_page_line_v1_t value{};
  uint64_t canary = UINT64_C(0x39b5192f84f7d6a1);
};

struct ExtendedGlyph {
  dk_page_glyph_v1_t value{};
  uint64_t canary = UINT64_C(0xc42d73e8a9165fb0);
};

std::vector<ExtendedInput> page_inputs() {
  std::vector<ExtendedInput> result;
  result.reserve(kPage43Lines.size());
  for (const auto& text : kPage43Lines) {
    ExtendedInput input;
    input.value.struct_size = sizeof(input);
    input.value.text = reinterpret_cast<const uint16_t*>(text.data());
    input.value.text_length = text.size();
    input.value.desired_width = 16659;
    input.value.role = DK_PAGE_LINE_ROLE_ORDINARY;
    input.value.alignment = DK_PAGE_ALIGNMENT_DISTRIBUTE;
    result.push_back(input);
  }
  return result;
}

dk_page_options_v1_t page_options() {
  dk_page_options_v1_t result{};
  result.struct_size = DK_PAGE_OPTIONS_V1_SIZE;
  result.profile = DK_PAGE_PROFILE_MADINAH_1441_V1;
  result.page_width = 17000;
  result.line_stride = sizeof(ExtendedInput);
  return result;
}

Page shape_page(dk_engine_t* engine) {
  auto inputs = page_inputs();
  const auto options = page_options();
  dk_page_t* raw = reinterpret_cast<dk_page_t*>(uintptr_t{1});
  require(dk_engine_shape_page_utf16_v1(
              engine, &options,
              reinterpret_cast<const dk_page_line_input_v1_t*>(inputs.data()),
              inputs.size(), &raw) == DK_STATUS_OK && raw,
          "shape page 43");
  return {raw, dk_page_destroy};
}

uint64_t mix(uint64_t digest, uint64_t value) {
  for (unsigned index = 0; index < 8; ++index) {
    digest ^= (value >> (index * 8)) & UINT64_C(0xff);
    digest *= UINT64_C(1099511628211);
  }
  return digest;
}

uint64_t scaled(double value, double scale) {
  return static_cast<uint64_t>(static_cast<int64_t>(std::llround(value * scale)));
}

struct PathCount {
  size_t commands = 0;
};
dk_status_t path_command(void* data) {
  ++static_cast<PathCount*>(data)->commands;
  return DK_STATUS_OK;
}
dk_status_t move_to(void* data, float, float) { return path_command(data); }
dk_status_t line_to(void* data, float, float) { return path_command(data); }
dk_status_t quadratic_to(void* data, float, float, float, float) {
  return path_command(data);
}
dk_status_t cubic_to(void* data, float, float, float, float, float, float) {
  return path_command(data);
}
dk_status_t close_path(void* data) { return path_command(data); }

struct PageStats {
  uint64_t digest = UINT64_C(1469598103934665603);
  size_t glyphs = 0;
  size_t outlined = 0;
};

PageStats inspect_page(const dk_page_t* page, dk_engine_t* engine,
                       bool inspect_outlines, bool strict_page43) {
  require(dk_page_line_count(page) == kPage43Lines.size(),
          "page 43 line count");
  PageStats stats;
  for (size_t line_index = 0; line_index < kPage43Lines.size(); ++line_index) {
    ExtendedLine output;
    output.value.struct_size = sizeof(output);
    const auto canary = output.canary;
    require(dk_page_get_line_v1(page, line_index, &output.value) ==
                    DK_STATUS_OK &&
                output.value.struct_size == sizeof(output) &&
                output.canary == canary,
            "read extended line output");
    const auto& line = output.value;
    require(line.flags == 0 && line.role == DK_PAGE_LINE_ROLE_ORDINARY &&
                line.alignment == DK_PAGE_ALIGNMENT_DISTRIBUTE &&
                line.x_origin == 0 && line.desired_width == 16659 &&
                std::abs(line.final_width - 16659) < 1e-8 &&
                line.font_scale == 1 && line.x_scale == 1 &&
                (!strict_page43 ||
                 line.glyph_count == kPage43GlyphCounts[line_index]),
            "page 43 line geometry");

    stats.digest = mix(stats.digest, line_index);
    stats.digest = mix(stats.digest, line.glyph_count);
    stats.digest = mix(stats.digest, scaled(line.final_width, 1000000));
    stats.digest = mix(stats.digest, scaled(line.font_scale, 1000000));
    stats.digest = mix(stats.digest, scaled(line.x_scale, 1000000));
    stats.digest = mix(stats.digest, scaled(line.x_origin, 1000000));

    double width = 0;
    for (size_t glyph_index = 0; glyph_index < line.glyph_count;
         ++glyph_index) {
      ExtendedGlyph output_glyph;
      output_glyph.value.struct_size = sizeof(output_glyph);
      const auto glyph_canary = output_glyph.canary;
      require(dk_page_get_glyph_v1(page, line_index, glyph_index,
                                   &output_glyph.value) == DK_STATUS_OK &&
                  output_glyph.value.struct_size == sizeof(output_glyph) &&
                  output_glyph.canary == glyph_canary,
              "read extended glyph output");
      const auto& glyph = output_glyph.value;
      require(glyph.flags == 0 &&
                  glyph.cluster < kPage43Lines[line_index].size() &&
                  std::isfinite(glyph.x_advance) &&
                  std::isfinite(glyph.y_advance) &&
                  std::isfinite(glyph.x_offset) &&
                  std::isfinite(glyph.y_offset) &&
                  std::abs(glyph.left_tatweel) <= 1 &&
                  std::abs(glyph.right_tatweel) <= 1,
              "page 43 glyph fields");
      width += glyph.x_advance;
      ++stats.glyphs;

      stats.digest = mix(stats.digest, glyph.glyph_id);
      stats.digest = mix(stats.digest, glyph.cluster);
      stats.digest = mix(stats.digest, scaled(glyph.x_advance, 1000000));
      stats.digest = mix(stats.digest, scaled(glyph.y_advance, 1000000));
      stats.digest = mix(stats.digest, scaled(glyph.x_offset, 1000000));
      stats.digest = mix(stats.digest, scaled(glyph.y_offset, 1000000));
      stats.digest = mix(stats.digest, scaled(glyph.left_tatweel, 16384));
      stats.digest = mix(stats.digest, scaled(glyph.right_tatweel, 16384));

      if (inspect_outlines) {
        PathCount count;
        dk_path_sink_v1_t sink{DK_PATH_SINK_V1_SIZE, &count, move_to, line_to,
                               quadratic_to, cubic_to, close_path};
        const dk_glyph_variant_v1_t variant{
            DK_GLYPH_VARIANT_V1_SIZE, glyph.glyph_id, 0,
            glyph.left_tatweel, glyph.right_tatweel};
        require(dk_engine_emit_glyph_outline_v1(engine, &variant, &sink) ==
                    DK_STATUS_OK,
                "draw page glyph");
        if (count.commands != 0) ++stats.outlined;
      }
    }
    require(std::abs(width - line.final_width) < 1e-8,
            "page 43 glyph width sum");
  }
  return stats;
}

void expect_invalid(dk_engine_t* engine, dk_page_options_v1_t options,
                    std::vector<ExtendedInput> inputs, const char* message) {
  dk_page_t* output = reinterpret_cast<dk_page_t*>(uintptr_t{1});
  require(dk_engine_shape_page_utf16_v1(
              engine, &options,
              reinterpret_cast<const dk_page_line_input_v1_t*>(inputs.data()),
              inputs.size(), &output) == DK_STATUS_INVALID_ARGUMENT &&
              output == nullptr,
          message);
}

}  // namespace

int main(int argc, char** argv) {
  try {
    require(argc == 2 || argc == 3,
            "expected runtime font path and optional --strict");
    const bool strict_page43 = argc == 3 && std::string{argv[2]} == "--strict";
    require(argc == 2 || strict_page43, "unknown page conformance option");
    require(dk_engine_abi_version() == 1, "additive page API changed ABI major");
    const auto bytes = read_file(argv[1]);
    require(!bytes.empty(), "runtime font bytes");

    dk_engine_t* raw_file = nullptr;
    require(dk_engine_create_from_file(argv[1], &raw_file) == DK_STATUS_OK &&
                raw_file,
            "file engine");
    Engine file_engine{raw_file, dk_engine_destroy};
    Page file_page = shape_page(file_engine.get());
    const auto expected =
        inspect_page(file_page.get(), file_engine.get(), true, strict_page43);
    require(expected.glyphs != 0 && expected.outlined != 0,
            "page 43 visible glyphs");
    if (strict_page43) {
      require(expected.glyphs == 1352, "page 43 total glyphs");
      require(expected.outlined == 1230, "page 43 visible outlines");
      if (expected.digest != kPage43Digest) {
        std::cerr << "page 43 digest mismatch: " << std::hex
                  << expected.digest << " != " << kPage43Digest << std::dec
                  << '\n';
      }
      require(expected.digest == kPage43Digest, "page 43 canonical digest");
    }

    dk_engine_t* raw_memory = nullptr;
    require(dk_engine_create_from_memory(bytes.data(), bytes.size(),
                                         &raw_memory) == DK_STATUS_OK &&
                raw_memory,
            "memory engine");
    Engine memory_engine{raw_memory, dk_engine_destroy};
    Page memory_page = shape_page(memory_engine.get());
    require(inspect_page(memory_page.get(), memory_engine.get(), false,
                         strict_page43).digest == expected.digest,
            "file-memory page parity");

    dk_engine_t* raw_lifetime = nullptr;
    require(dk_engine_create_from_memory(bytes.data(), bytes.size(),
                                         &raw_lifetime) == DK_STATUS_OK &&
                raw_lifetime,
            "page lifetime engine");
    Page detached_page = shape_page(raw_lifetime);
    dk_engine_destroy(raw_lifetime);
    require(inspect_page(detached_page.get(), nullptr, false, strict_page43)
                    .digest == expected.digest,
            "page result outlives engine");

    const std::array<std::u16string, 2> semantic_text{
        u"\u0633\u064f\u0648\u0631\u064e\u0629\u064f \u0671\u0644\u0652\u0628\u064e\u0642\u064e\u0631\u064e\u0629\u0650",
        u"\u0628\u0650\u0633\u0652\u0645\u0650 \u0671\u0644\u0644\u0651\u064e\u0647\u0650 \u0671\u0644\u0631\u0651\u064e\u062d\u0652\u0645\u064e\u0670\u0646\u0650 \u0671\u0644\u0631\u0651\u064e\u062d\u0650\u064a\u0645\u0650"};
    std::array<dk_page_line_input_v1_t, 2> semantic_inputs{};
    semantic_inputs[0] = {DK_PAGE_LINE_INPUT_V1_SIZE, 0,
                          reinterpret_cast<const uint16_t*>(
                              semantic_text[0].data()),
                          semantic_text[0].size(), 0,
                          DK_PAGE_LINE_ROLE_SURAH_HEADING,
                          DK_PAGE_ALIGNMENT_CENTER};
    semantic_inputs[1] = {DK_PAGE_LINE_INPUT_V1_SIZE,
                          DK_PAGE_LINE_FLAG_ALTERNATE_BASMALA,
                          reinterpret_cast<const uint16_t*>(
                              semantic_text[1].data()),
                          semantic_text[1].size(), 0,
                          DK_PAGE_LINE_ROLE_BASMALA,
                          DK_PAGE_ALIGNMENT_CENTER};
    auto semantic_options = page_options();
    semantic_options.line_stride = sizeof(semantic_inputs[0]);
    dk_page_t* raw_semantic_page = nullptr;
    require(dk_engine_shape_page_utf16_v1(
                file_engine.get(), &semantic_options, semantic_inputs.data(),
                semantic_inputs.size(), &raw_semantic_page) == DK_STATUS_OK &&
                raw_semantic_page,
            "shape semantic page lines");
    Page semantic_page{raw_semantic_page, dk_page_destroy};
    for (size_t index = 0; index < semantic_inputs.size(); ++index) {
      dk_page_line_v1_t line{DK_PAGE_LINE_V1_SIZE};
      require(dk_page_get_line_v1(semantic_page.get(), index, &line) ==
                      DK_STATUS_OK &&
                  line.flags == semantic_inputs[index].flags &&
                  line.role == semantic_inputs[index].role &&
                  line.alignment == DK_PAGE_ALIGNMENT_CENTER &&
                  line.desired_width == 0 && line.final_width > 0 &&
                  std::abs(line.x_origin -
                           (semantic_options.page_width - line.final_width) /
                               2.0) < 1e-8 &&
                  line.glyph_count > 0,
              "semantic page line result");
    }

    dk_page_line_input_v1_t narrow_input{
        DK_PAGE_LINE_INPUT_V1_SIZE,
        0,
        reinterpret_cast<const uint16_t*>(kPage43Lines[0].data()),
        kPage43Lines[0].size(),
        8000,
        DK_PAGE_LINE_ROLE_ORDINARY,
        DK_PAGE_ALIGNMENT_DISTRIBUTE};
    auto narrow_options = page_options();
    narrow_options.line_stride = sizeof(narrow_input);
    dk_page_t* raw_narrow_page = nullptr;
    require(dk_engine_shape_page_utf16_v1(
                file_engine.get(), &narrow_options, &narrow_input, 1,
                &raw_narrow_page) == DK_STATUS_OK &&
                raw_narrow_page,
            "shape reduced-scale page");
    Page narrow_page{raw_narrow_page, dk_page_destroy};
    dk_page_line_v1_t narrow_line{DK_PAGE_LINE_V1_SIZE};
    const double upem = dk_engine_upem(file_engine.get());
    require(dk_page_get_line_v1(narrow_page.get(), 0, &narrow_line) ==
                    DK_STATUS_OK &&
                narrow_line.font_scale > 0 && narrow_line.font_scale < 1 &&
                std::abs(narrow_line.font_scale * upem -
                         std::round(narrow_line.font_scale * upem)) < 1e-9 &&
                std::abs(narrow_line.final_width - narrow_input.desired_width) <
                    10,
            "effective reduced font scale");

    std::array<std::future<uint64_t>, 4> futures;
    for (auto& future : futures) {
      future = std::async(std::launch::async, [&] {
        Page page = shape_page(file_engine.get());
        return inspect_page(page.get(), file_engine.get(), false,
                            strict_page43).digest;
      });
    }
    for (auto& future : futures) {
      require(future.get() == expected.digest, "concurrent page determinism");
    }

    auto options = page_options();
    auto inputs = page_inputs();
    options.profile = 0;
    expect_invalid(file_engine.get(), options, inputs, "unknown profile");
    options = page_options();
    options.flags = 1;
    expect_invalid(file_engine.get(), options, inputs, "reserved page flags");
    options = page_options();
    options.line_stride = DK_PAGE_LINE_INPUT_V1_SIZE - 1;
    expect_invalid(file_engine.get(), options, inputs, "short line stride");
    options = page_options();
    inputs[0].value.struct_size = DK_PAGE_LINE_INPUT_V1_SIZE - 1;
    expect_invalid(file_engine.get(), options, inputs, "short line input");
    inputs = page_inputs();
    inputs[0].value.flags = DK_PAGE_LINE_FLAG_ALTERNATE_BASMALA;
    expect_invalid(file_engine.get(), options, inputs,
                   "alternate ordinary line");
    inputs = page_inputs();
    inputs[0].value.desired_width = 0;
    expect_invalid(file_engine.get(), options, inputs,
                   "zero ordinary line width");
    const std::array<uint16_t, 1> invalid_utf16{UINT16_C(0xd800)};
    inputs = page_inputs();
    inputs[0].value.text = invalid_utf16.data();
    inputs[0].value.text_length = invalid_utf16.size();
    expect_invalid(file_engine.get(), options, inputs, "invalid UTF-16");

    dk_page_line_v1_t short_line{};
    short_line.struct_size = DK_PAGE_LINE_V1_SIZE - 1;
    short_line.flags = UINT32_MAX;
    require(dk_page_get_line_v1(file_page.get(), 0, &short_line) ==
                    DK_STATUS_INVALID_ARGUMENT &&
                short_line.flags == 0,
            "short line output");
    dk_page_glyph_v1_t short_glyph{};
    short_glyph.struct_size = DK_PAGE_GLYPH_V1_SIZE - 1;
    short_glyph.glyph_id = UINT32_MAX;
    require(dk_page_get_glyph_v1(file_page.get(), 0, 0, &short_glyph) ==
                    DK_STATUS_INVALID_ARGUMENT &&
                short_glyph.glyph_id == 0,
            "short glyph output");

    std::cout << "page43_lines=" << kPage43Lines.size() << '\n'
              << "page43_glyphs=" << expected.glyphs << '\n'
              << "page43_outlined=" << expected.outlined << '\n'
              << "page43_digest=" << std::hex << expected.digest << std::dec
              << '\n';
    return 0;
  } catch (const std::exception& error) {
    std::cerr << "page conformance failure: " << error.what() << '\n';
    return 1;
  }
}
