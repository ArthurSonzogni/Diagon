#ifndef SCREEN_H
#define SCREEN_H

#include <string>
#include <string_view>
#include <vector>

std::string to_string(const std::wstring& s);
std::wstring to_wstring(const std::string& s);

class Screen {
 public:
  Screen() = default;
  Screen(int width, int height);
  void DrawPixel(int x, int y, wchar_t c);
  void DrawText(int x, int y, std::wstring_view text);
  void DrawBox(int x, int y, int w, int h);
  void DrawBoxedText(int x, int y, const std::wstring& text);
  void DrawHorizontalLine(int left, int right, int y, wchar_t c = U'─');
  void DrawVerticalLine(int top, int bottom, int x, wchar_t c = U'│');
  void DrawVerticalLineComplete(int top, int bottom, int x);
  void ASCIIfy(int style = 0);
  std::string ToString();
  wchar_t& Pixel(int x, int y);

  void Resize(int dim_x, int dim_y);
  void Append(const Screen& other, int x, int y);

  int width() const { return dim_x_; }
  int height() const { return dim_y_; }

 private:
  int dim_x_ = 0;
  int dim_y_ = 0;
  std::vector<std::wstring> lines_;
};

#endif /* end of include guard: SCREEN_H */
