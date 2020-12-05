#include <algorithm>
#include <map>
#include <set>
#include <sstream>
#include <string>
#include <string_view>
#include <vector>
#include "screen/Screen.h"

namespace {

struct Node {
  // Parser:
  std::set<int> upward;
  std::set<int> downward;

  // Layering
  int layer = 0;
  int row = 0;
  std::set<int> downward_closure;
  std::vector<int> upward_sorted;
  std::vector<int> downward_sorted;


  // Render:
  int width = 0;
  int height = 0;
  int x = 0;
  int y = 0;

  // Misc
  bool is_connector = false;
  bool padding = 1;
};

// A Edge is a vertical line that connect two node: up and down.
struct Edge {
  int up = 0;
  int down = 0;
  int x = 0;
};

struct Layer {
  std::vector<int> nodes;
  std::vector<Edge> edges;
};

struct Context {
  // Node "label" toward Node's "ID".
  std::vector<std::wstring> labels;

  // Node "ID" toward Node's "label".
  std::map<std::wstring, int> id;

  std::vector<Node> nodes;

  std::vector<Layer> layers;

  // --------------------------------------------
  std::string Process(const std::wstring& input);
  void Parse(const std::wstring& input);
  void AddNode(std::wstring name);
  void AddConnector(int a, int b);
  void AddVertex(std::wstring a, std::wstring b);
  bool Toposort();
  void Complete();
  void AddToLayers();
  void OptimizeRowOrder();
  void Layout();
  bool LayoutNodeDoNotTouch();
  bool LayoutEdgesDoNotTouch();
  bool LayoutGrowNode();
  bool LayoutShiftEdges();
  bool LayoutShiftConnectorNode();
  std::string Render();
};

void Context::AddNode(std::wstring name) {
  if (id.find(name) != id.end())
    return;
  int i = nodes.size();
  nodes.emplace_back();

  id[name] = i;
  labels.push_back(name);
}

void Context::AddConnector(int a, int b) {
  int c = nodes.size();
  nodes.emplace_back();
  labels.push_back(L"connector");

  nodes[c].layer = nodes[a].layer + 1;
  nodes[c].is_connector = true;
  nodes[c].padding = 0;

  // Disconnect A from B
  nodes[a].downward.erase(b);
  nodes[b].upward.erase(a);

  // Connect A and C
  nodes[a].downward.insert(c);
  nodes[c].upward.insert(a);

  // Connect C and B
  nodes[c].downward.insert(b);
  nodes[b].upward.insert(c);
}

void Context::AddVertex(std::wstring a, std::wstring b) {
  nodes[id[a]].downward.insert(id[b]);
  nodes[id[b]].upward.insert(id[a]);
}

bool Context::Toposort() {
  bool has_work = true;
  int iteration = 0;
  while (has_work) {
    has_work = false;
    for (int a = 0; a < nodes.size(); ++a) {
      for (const auto& b : nodes[a].downward) {
        if (nodes[b].layer <= nodes[a].layer) {
          nodes[b].layer = nodes[a].layer + 1;
          has_work = true;
        }
      }
    }

    // Check for cycles.
    if (++iteration > nodes.size() * nodes.size())
      return false;
  }
  return true;
}

std::wstring_view Trim(std::wstring_view str) {
  int left = 0;
  while (str[left] == ' ' && left + 1< str.size())
    ++left;

  int right = str.size() - 1;
  while (str[right] == ' ' && right >= 1)
    --right;

  return str.substr(left, right - left + 1);
}

std::vector<std::wstring_view> Split(std::wstring_view str,
                                     std::wstring_view delim) {
  std::vector<std::wstring_view> out;

  size_t start = 0;
  while (true) {
    size_t pos = str.find(delim, start);

    // Not found
    if (pos == std::wstring_view::npos) {
      if (str.size() - start)
        out.emplace_back(str.substr(start, str.size() - start));
      break;
    }

    if (pos - start)
      out.emplace_back(str.substr(start, pos - start));
    start = pos + delim.size();
  }
  return out;
}

void Context::Parse(const std::wstring& input) {
  for (auto line : Split(input, L"\n")) {
    std::wstring_view previous_part;
    for (auto part : Split(line, L"->")) {
      std::wstring_view name = Trim(part);
      AddNode(std::wstring(name));
      if (previous_part.size() != 0)
        AddVertex(std::wstring(previous_part), std::wstring(name));
      previous_part = name;
    }
  }
}

void Context::Complete() {
  bool work_to_do = true;
  while (work_to_do) {
    work_to_do = false;
    for (int a = 0; a < nodes.size(); ++a) {
      for (int b : nodes[a].downward) {
        if (nodes[a].layer + 1 != nodes[b].layer) {
          work_to_do = true;
          AddConnector(a, b);
          break;
        }
      }
    }
  }
}

void Context::AddToLayers() {
  // Compute the number of layers necessary.
  int last_layer = 0;
  for (int i = 0; i < nodes.size(); ++i) {
    last_layer = std::max(last_layer, nodes[i].layer);
  }
  layers.resize(last_layer + 1);

  // Put the elements in the layers.
  for (int i = 0; i < nodes.size(); ++i)
    layers[nodes[i].layer].nodes.push_back(i);

  OptimizeRowOrder();

  // Precompute upward_sorted, downward_sorted.
  for (Node& node : nodes) {
    for (int i : node.upward)
      node.upward_sorted.push_back(i);
    for (int i : node.downward)
      node.downward_sorted.push_back(i);
    auto ByRow = [&](int a, int b) { return nodes[a].row < nodes[b].row; };
    std::sort(node.upward_sorted.begin(), node.upward_sorted.end(), ByRow);
    std::sort(node.downward_sorted.begin(), node.downward_sorted.end(), ByRow);
  }

  // Add the edges 
  for (auto& layer : layers) {
    for (int up : layer.nodes) {
      for (int down : nodes[up].downward_sorted) {
        layer.edges.push_back({up, down, 0});
      }
    }
  }
}

void Context::OptimizeRowOrder() {
  // Compute the downward_closure.
  for (int y = layers.size() - 2; y; --y) {
    auto& layer = layers[y];
    for (int up : layer.nodes) {
      auto& node_up = nodes[up];
      for (int down : node_up.downward) {
        auto& node_down = nodes[down];
        node_up.downward_closure.insert(down);
        for (int i : node_down.downward_closure)
          node_up.downward_closure.insert(i);
      }
    }
  }

  // Optimize each layers.
  for(auto& layer : layers) {
    int layer_width = layer.nodes.size();

    // Compute inter node downward distances.
    std::vector<std::vector<int>> distances(
        layer_width, std::vector<int>(layer_width, 2*nodes.size()));
    for (int a = 0; a < layer_width; ++a) {
      for (int b = 0; b < layer_width; ++b) {
        auto& node_a = nodes[layer.nodes[a]];
        auto& node_b = nodes[layer.nodes[b]];
        std::vector<int> intersection;
        std::set_intersection(
            node_a.downward_closure.begin(), node_a.downward_closure.end(),
            node_b.downward_closure.begin(), node_b.downward_closure.end(),
            std::back_inserter(intersection));
        for (int ancestor : intersection) {
          auto& d = distances[a][b];
          d = std::min(d, nodes[ancestor].layer - nodes[a].layer);
        }
      }
    }

    std::vector<float> parent_mean(layer_width);
    for (int a = 0; a < layer_width; ++a) {
      float d = 0.f;
      Node& node_down = nodes[layer.nodes[a]];
      for (auto& b : node_down.upward) {
        Node& node_up = nodes[b];
        d += node_up.row;
      }
      parent_mean[a] = d / (node_down.upward.size() + 0.01f);
    }

    std::vector<int> permutation;
    for(int i = 0; i<layer_width; ++i)
      permutation.push_back(i);
    auto evaluate_score = [&] {
      float score = 0.f;
      for (int i = 0; i < layer_width - 1; ++i) {
        score += distances[permutation[i + 0]]  //
                          [permutation[i + 1]];
      }
      for (int i = 0; i < layer_width; ++i) {
        float d = i - parent_mean[permutation[i]];
        score += d*d;
      }
      return score;
    };
    float score = evaluate_score();
    float score_last_loop = 0.f;
    while (score != score_last_loop) {
      score_last_loop = score;
      for (int a = 0; a < layer_width; ++a)
        for (int b = 0; b < layer_width; ++b) {
          std::swap(permutation[a], permutation[b]);
          float new_score = evaluate_score();
          if (new_score < score) {
            score = new_score;
          } else {
            std::swap(permutation[a], permutation[b]);
          }
        }
    }

    // Reorder the nodes inside the layer.
    {
      std::vector<int> nodes;
      for (auto& p : permutation)
        nodes.push_back(layer.nodes[p]);
      layer.nodes = std::move(nodes);
    }

    // Objective = distance[row[i]][row[i+1]] + 
    // Distance(a,b) = distance[a][b]
    // Reorder nodes

    // Precompute every nodes row.
    for (int i = 0; i < layer_width; ++i) {
      Node& node = nodes[layer.nodes[i]];
      node.row = i;
    }
  }
}

void Context::Layout() {
  // y-axis: size and position.
  for (Node& node : nodes) {
    node.y = node.layer * 3 + 1;
    node.height = 3;
  }

  // x-axis: minimal size to draw their content.
  for (int i = 0; i < nodes.size(); ++i) {
    Node& node = nodes[i];
    if (node.is_connector) {
      node.width = 1;
    } else {
      node.width = labels[i].size() + 2;
    }
  }

  // x-axis: increase size, so that nodes and connectors fit together.
  for(int i = 0; i<1000; ++i) {
    if (!LayoutNodeDoNotTouch())
      continue;
    if (!LayoutEdgesDoNotTouch())
      continue;
    if (!LayoutGrowNode())
      continue;
    if (!LayoutShiftEdges())
      continue;
    if (!LayoutShiftConnectorNode())
      continue;
    break;
  }
}

bool Context::LayoutNodeDoNotTouch() {
  bool done = true;
  // Shift nodes to the right so that they don't touch.
  for (auto& layer : layers) {
    int x = 0;
    for (auto& e : layer.nodes) {
      if (nodes[e].x < x) {
        nodes[e].x = x;
        done = false;
      }
      x = nodes[e].x + nodes[e].width;
    }
  }
  return done;
}

bool Context::LayoutEdgesDoNotTouch() {
  int done = true;
  // Shift nodes to the right so that they don't touch.
  for (auto& layer : layers) {
    int x = 0;
    for (auto& e : layer.nodes) {
      if (nodes[e].x < x) {
        nodes[e].x = x;
        done = false;
      }
      x = nodes[e].x + nodes[e].width;
    }
  }
  return done;
}

bool Context::LayoutGrowNode() {
  // Grow the nodes to fit their edges
  for (auto& layer : layers) {
    for (auto& edge : layer.edges) {
      Node& node_up = nodes[edge.up];
      if (node_up.x + node_up.width - 1 - 1 < edge.x &&
          !node_up.is_connector) {
        node_up.width = edge.x + 1 + 1 - node_up.x;
        return false;
      }

      Node& node_down = nodes[edge.down];
      if (node_down.x + node_down.width - 1 - 1 < edge.x &&
          !node_down.is_connector) {
        node_down.width = edge.x + 1 + 1 - node_down.x;
        return false;
      }
    }
  }
  return true;
}

// Shift the edges to the right, so that they reach their nodes.
bool Context::LayoutShiftEdges() {
  for (auto& layer : layers) {
    for (auto& edge : layer.edges) {
      Node& up = nodes[edge.up];
      Node& down = nodes[edge.down];
      int min = std::max(up.x + up.padding,     //
                         down.x + down.padding  //
      );
      if (edge.x < min) {
        edge.x = min;
        return false;
      }
    }
  }
  return true;
}

bool Context::LayoutShiftConnectorNode() {
  for (int i = 0; i < nodes.size(); ++i) {
    Node& node = nodes[i];
    if (!node.is_connector)
      continue;

    int min = 0;
    for (auto& edge : layers[node.layer-1].edges) {
      if (edge.down == i)
        min = std::max(min, edge.x);
    }
    for (auto& edge : layers[node.layer].edges) {
      if (edge.up == i)
        min = std::max(min, edge.x);
    }

    if (node.x < min) {
      node.x = min;
      return false;
    }
  }
  return true;
}

std::string Context::Render() {
  int width = 0;
  int height = 0;
  for (const Node& node : nodes) {
    width = std::max(width, node.x + node.width);
    height = std::max(height, node.y + node.height);
  }

  // Draw the nodes.
  auto screen = Screen(width, height);
  for (int i = 0; i < nodes.size(); ++i) {
    const Node& node = nodes[i];
    if (node.is_connector) {
      if (node.width == 1)
        screen.DrawVerticalLine(node.y , node.y + 2, node.x);
      else
        screen.DrawBox(node.x, node.y, node.width, node.height);
    } else {
      screen.DrawBox(node.x, node.y, node.width, node.height);
      screen.DrawText(node.x + 1, node.y + 1, labels[i]);
    }
  }

  // Draw the edges.
  for (int y = 0; y < layers.size(); ++y) {
    auto& layer = layers[y];
    for (Edge& edge : layer.edges) {
      int x = edge.x;
      wchar_t up = nodes[edge.up].is_connector ? U'│' : U'┬';
      wchar_t down = nodes[edge.down].is_connector ? U'│' : U'▽';
      screen.DrawPixel(x, 3 * y + 3, up);
      screen.DrawPixel(x, 3 * y + 4, down);
    }
  }

  return screen.ToString();
}

std::string Context::Process(const std::wstring& input) {
  Parse(input.c_str());
  if (nodes.size() == 0)
    return "";
  if (!Toposort())
    return "There are cycles";
  Complete();
  AddToLayers();
  Layout();
  return Render();
}

}  // namespace

std::string DagToText(const std::string& input) {
  Context context;
  return context.Process(to_wstring(input));
}

// Copyright 2020 Arthur Sonzogni. All rights reserved.
// Use of this source code is governed by the MIT license that can be found in
// the LICENSE file.
