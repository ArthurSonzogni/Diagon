// Copyright 2020 Arthur Sonzogni. All rights reserved.
// Use of this source code is governed by the MIT license that can be found in
// the LICENSE file.

#include <functional>
#include <iostream>
#include <sstream>
#include <vector>

#include "screen/Screen.h"
#include "translator/Translator.h"

namespace {

struct Node {
  std::wstring content;
  Node* parent = nullptr;
  std::vector<std::unique_ptr<Node>> children;
};

struct Line {
  int spaces = 0;
  std::wstring content;
  Node* tree;
};

std::string PrintUnicode1(std::unique_ptr<Node> tree) {
  std::stringstream output;
  std::function<void(std::unique_ptr<Node>&, std::string)> print_children;
  print_children = [&](std::unique_ptr<Node>& node, std::string prefix) {
    for (int i = 0; i < node->children.size(); ++i) {
      auto& child = node->children[i];
      if (i != node->children.size() - 1) {
        output << prefix + " ├─" << to_string(child->content) << std::endl;
        print_children(child, prefix + " │ ");
      } else {
        output << prefix + " └─" << to_string(child->content) << std::endl;
        print_children(child, prefix + "   ");
      }
    }
  };

  for (auto& it : tree->children) {
    output << to_string(it->content) << std::endl;
    print_children(it, "");
  }
  return output.str();
}

std::string PrintUnicode2(std::unique_ptr<Node> tree) {
  std::stringstream output;
  std::function<void(std::unique_ptr<Node>&, std::string)> print_children;
  print_children = [&](std::unique_ptr<Node>& node, std::string prefix) {
    for (int i = 0; i < node->children.size(); ++i) {
      auto& child = node->children[i];
      if (i != node->children.size() - 1) {
        output << prefix + " ├──" << to_string(child->content) << std::endl;
        print_children(child, prefix + " │  ");
      } else {
        output << prefix + " └──" << to_string(child->content) << std::endl;
        print_children(child, prefix + "    ");
      }
    }
  };

  for (auto& it : tree->children) {
    output << to_string(it->content) << std::endl;
    print_children(it, "");
  }
  return output.str();
}

std::string PrintASCII1(std::unique_ptr<Node> tree) {
  std::stringstream output;
  std::function<void(std::unique_ptr<Node>&, std::string)> print_children;
  print_children = [&](std::unique_ptr<Node>& node, std::string prefix) {
    for (int i = 0; i < node->children.size(); ++i) {
      auto& child = node->children[i];
      if (i != node->children.size() - 1) {
        output << prefix + " +-" << to_string(child->content) << std::endl;
        print_children(child, prefix + " |  ");
      } else {
        output << prefix + " `-" << to_string(child->content) << std::endl;
        print_children(child, prefix + "    ");
      }
    }
  };

  for (auto& it : tree->children) {
    output << to_string(it->content) << std::endl;
    print_children(it, "");
  }
  return output.str();
}

std::string PrintASCII2(std::unique_ptr<Node> tree) {
  std::stringstream output;
  std::function<void(std::unique_ptr<Node>&, std::string)> print_children;
  print_children = [&](std::unique_ptr<Node>& node, std::string prefix) {
    for (int i = 0; i < node->children.size(); ++i) {
      auto& child = node->children[i];
      if (i != node->children.size() - 1) {
        output << prefix + " +--" << to_string(child->content) << std::endl;
        print_children(child, prefix + " |  ");
      } else {
        output << prefix + " `--" << to_string(child->content) << std::endl;
        print_children(child, prefix + "    ");
      }
    }
  };

  for (auto& it : tree->children) {
    output << to_string(it->content) << std::endl;
    print_children(it, "");
  }
  return output.str();
}

std::string PrintASCII3(std::unique_ptr<Node> tree) {
  std::stringstream output;
  std::function<void(std::unique_ptr<Node>&, std::string)> print_children;
  print_children = [&](std::unique_ptr<Node>& node, std::string prefix) {
    for (int i = 0; i < node->children.size(); ++i) {
      auto& child = node->children[i];
      if (i != node->children.size() - 1) {
        output << prefix + " |--" << to_string(child->content) << std::endl;
        print_children(child, prefix + " |  ");
      } else {
        output << prefix + " `--" << to_string(child->content) << std::endl;
        print_children(child, prefix + "    ");
      }
    }
  };

  for (auto& it : tree->children) {
    output << to_string(it->content) << std::endl;
    print_children(it, "");
  }
  return output.str();
}

struct DisplayTree {
  int entrance;
  std::vector<std::wstring> content;
};

enum class Align {
  Top,
  Center,
  Bottom,
};

DisplayTree MergeDisplayTree(std::wstring content,
                             std::vector<DisplayTree> children,
                             Align align) {
  std::wstring space_to_add(content.size() + 3, L' ');
  DisplayTree ret;

  // Leaf of the recursion.
  if (children.size() == 0) {
    ret.content.push_back(content);
    ret.entrance = 0;
    return ret;
  }

  // Draw Childrens
  for (auto& child : children) {
    for (auto& line : child.content) {
      ret.content.push_back(space_to_add + line);
    }
  }

  // Draw the current content.
  switch (align) {
    case Align::Top:
      ret.entrance = 0;
      break;
    case Align::Center:
      ret.entrance = ret.content.size() / 2;
      break;
    case Align::Bottom:
      ret.entrance = ret.content.size() - 1;
      break;
  };

  for (int i = 0; i < content.size(); ++i) {
    ret.content[ret.entrance][i] = content[i];
  }

  // Draw vertex.
  int first_entrance = children[0].entrance;
  int last_entrance = 0;
  {
    int y = 0;
    for (auto& child : children) {
      last_entrance = y + child.entrance;
      y += child.content.size();
    }
  }

  int y = 0;
  for (auto& child : children) {
    int start = y;

    // Draw Child vertical connector.
    for (auto& line : child.content) {
      (void)line;
      if (y >= first_entrance && y <= last_entrance) {
        ret.content[y][content.size() + 1] = L'│';
      }
      ++y;
    }

    // Refine connector on child entrance points.
    int child_entrance = start + child.entrance;
    auto& connector = ret.content[child_entrance][content.size() + 1];
    if (first_entrance == last_entrance)
      connector = L'─';
    else if (child_entrance == first_entrance)
      connector = L'┌';
    else if (child_entrance < last_entrance)
      connector = L'├';
    else
      connector = L'└';

    // Draw connector to child entrance.
    ret.content[child_entrance][content.size() + 2] = L'─';
  }

  // Draw parent entrance to connector.
  ret.content[ret.entrance][content.size() + 0] = L'─';
  // Refine connector on parent entrance points.
  auto& connector = ret.content[ret.entrance][content.size() + 1];
  // clang-format off
  switch (connector) {
    case L'─': connector = L'─'; break;
    case L'┌': connector = L'┬'; break;
    case L'├': connector = L'┼'; break;
    case L'└': connector = L'┴'; break;
    case L'│': connector = L'┤'; break;
  }
  // clang-format on

  return ret;
}

DisplayTree MakeDisplayTree(Node* tree, Align align) {
  std::vector<DisplayTree> children_tree;
  for (auto& child : tree->children)
    children_tree.push_back(MakeDisplayTree(child.get(), align));
  return MergeDisplayTree(tree->content, std::move(children_tree), align);
}

std::string PrintUnicodeRightTop(std::unique_ptr<Node> tree) {
  DisplayTree display = MakeDisplayTree(tree.get(), Align::Top);
  std::stringstream output;
  for (auto& line : display.content) {
    output << to_string(line) << std::endl;
  }
  return output.str();
}

std::string PrintUnicodeRightCenter(std::unique_ptr<Node> tree) {
  DisplayTree display = MakeDisplayTree(tree.get(), Align::Center);
  std::stringstream output;
  for (auto& line : display.content) {
    output << to_string(line) << std::endl;
  }
  return output.str();
}

std::string PrintUnicodeRightBottom(std::unique_ptr<Node> tree) {
  DisplayTree display = MakeDisplayTree(tree.get(), Align::Bottom);
  std::stringstream output;
  for (auto& line : display.content) {
    output << to_string(line) << std::endl;
  }
  return output.str();
}

std::map<std::string, std::function<std::string(std::unique_ptr<Node>)>>
    print_function = {
        {"unicode 1", PrintUnicode1},
        {"unicode 2", PrintUnicode2},
        {"ASCII 1", PrintASCII1},
        {"ASCII 2", PrintASCII2},
        {"ASCII 3", PrintASCII3},
        {"unicode right top", PrintUnicodeRightTop},
        {"unicode right center", PrintUnicodeRightCenter},
        {"unicode right bottom", PrintUnicodeRightBottom},
};

}  // namespace

class Tree : public Translator {
 public:
  virtual ~Tree() = default;
  std::string Translate(const std::string& input,
                        const std::string& options_string) override {
    auto options = SerializeOption(options_string);

    // Style.
    std::string style_option = options["style"];

    // Parse the tree.
    std::vector<Line> lines;

    std::string line_text;
    std::stringstream ss(input);
    while (std::getline(ss, line_text)) {
      Line line;
      line.content = to_wstring(line_text);
      while (line.spaces < line.content.size() &&
             (line.content[line.spaces] == L' ' ||
              line.content[line.spaces] == L'\t')) {
        line.spaces++;
      }
      line.content = line.content.substr(line.spaces, -1);
      lines.push_back(line);
    }

    if (lines.size() == 0) {
      return "";
    }

    // Remove empty lines.
    // See https://github.com/ArthurSonzogni/Diagon/issues/58
    lines.erase(std::remove_if(
                    lines.begin(), lines.end(),
                    [](const Line& line) { return line.content.size() == 0; }),
                lines.end());

    // Build the tree.
    auto tree = std::make_unique<Node>();
    for (int i = 0; i < lines.size(); ++i) {
      auto child = std::make_unique<Node>();
      lines[i].tree = child.get();
      child->content = lines[i].content;
      for (int j = i - 1;; --j) {
        if (j == -1) {
          child->parent = tree.get();
          child->parent->children.push_back(std::move(child));
          break;
        }

        if (lines[j].spaces < lines[i].spaces) {
          child->parent = lines[j].tree;
          child->parent->children.push_back(std::move(child));
          break;
        }
      }
    }

    if (print_function.count(style_option)) {
      return print_function[style_option](std::move(tree));
    } else {
      return print_function["unicode 2"](std::move(tree));
    }
  }

  const char* Name() final { return "Tree"; }
  const char* Identifier() final { return "Tree"; }
  const char* Description() final { return "Draw a tree"; }

  std::vector<Translator::OptionDescription> Options() final {
    return {
        {
            "style",
            {
                "unicode 1",
                "unicode 2",
                "ASCII 1",
                "ASCII 2",
                "ASCII 3",
                "unicode right top",
                "unicode right center",
                "unicode right bottom",
            },
            "unicode 1",
            "The style of the tree.",
        },
    };
  }

  std::vector<Translator::Example> Examples() final {
    return {
        {"1 - Simple",
         "Linux\n"
         "  Android\n"
         "  Debian\n"
         "    Ubuntu\n"
         "      Lubuntu\n"
         "      Kubuntu\n"
         "      Xubuntu\n"
         "      Xubuntu\n"
         "    Mint\n"
         "  Centos\n"
         "  Fedora"},
    };
  }
};

std::unique_ptr<Translator> TreeTranslator() {
  return std::make_unique<Tree>();
}
