#include <sstream>
#include <vector>

#include "screen/Screen.h"
#include "translator/Translator.h"

class LineNumber : public Translator {
 public:
  virtual ~LineNumber() = default;
  virtual void Process(const std::string& input) {
    // cut by lines.
    std::stringstream ss(input);
    std::vector<std::wstring> lines;
    std::string line;
    while (std::getline(ss, line)) {
      lines.push_back(to_wstring(line));
    }

    int number_length = 0;
    int max_number = 1;
    while (max_number <= (int)lines.size()) {
      max_number *= 10;
      number_length++;
    }

    int text_max_width = 0;
    for (const auto& line : lines) {
      text_max_width = std::max(text_max_width, (int)line.size());
    }

    int width = number_length + text_max_width + 3;
    int height = lines.size() + 2;
    Screen screen(width, height);

    int y = 1;
    for (const auto& line : lines) {
      screen.DrawText(number_length + 2, y, line);
      screen.DrawText(1, y, to_wstring(std::to_string(y)));
      ++y;
    }

    screen.DrawBox(0, 0, width, height);
    screen.DrawVerticalLine(1, height - 2, number_length + 1);
    screen.DrawPixel(number_length + 1, 0, U'┬');
    screen.DrawPixel(number_length + 1, height - 1, U'┴');

    output_ = screen.ToString();
  }
  virtual std::string Output() { return output_; }

 private:
  std::string output_;
};

std::unique_ptr<Translator> LineNumberTranslator() {
  return std::make_unique<LineNumber>();
}
