#ifndef SCREEN_H
#define SCREEN_H

#include <vector>
#include <string>

std::string to_string(const std::wstring& s);
std::wstring to_wstring(const std::string& s);

class Screen {
 public:
  Screen(int width, int height);
  void DrawPixel(int x, int y, wchar_t c);
  void DrawText(int x, int y, const std::wstring& text);
  void DrawBox(int x, int y, int w, int h);
  void DrawBoxedText(int x, int y, const std::wstring& text);
  void DrawHorizontalLine(int left, int right, int y, wchar_t c = U'─');
  void DrawVerticalLine(int top, int bottom, int x, wchar_t c = U'│');
  void ASCIIfy(int style = 0);
  std::string ToString();
  wchar_t& Pixel(int x, int y);

 private:
  int width_;
  int height_;
  std::vector<std::wstring> lines_;
};

#endif /* end of include guard: SCREEN_H */
