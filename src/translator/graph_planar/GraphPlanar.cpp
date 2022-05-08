// Copyright 2020 Arthur Sonzogni. All rights reserved.
// Use of this source code is governed by the MIT license that can be found in
// the LICENSE file.

#include <iostream>
#include <map>
#include <queue>
#include <sstream>
#include <string>
#include <vector>
#include "screen/Screen.h"
#include "translator/Translator.h"
#include "translator/antlr_error_listener.h"
#include "translator/graph_planar/GraphPlanarLexer.h"
#include "translator/graph_planar/GraphPlanarParser.h"

// With emscripten, you may need to execute in the src directory.
// # ln -s /usr/include/boost/ boost
#include <boost/graph/adjacency_list.hpp>
#include <boost/graph/boyer_myrvold_planar_test.hpp>
#include <boost/graph/chrobak_payne_drawing.hpp>
#include <boost/graph/graph_traits.hpp>
#include <boost/graph/make_biconnected_planar.hpp>
#include <boost/graph/make_connected.hpp>
#include <boost/graph/make_maximal_planar.hpp>
#include <boost/graph/planar_canonical_ordering.hpp>
#include <boost/graph/properties.hpp>
#include <boost/property_map/property_map.hpp>

using Graph = boost::adjacency_list<boost::vecS,
                                    boost::vecS,
                                    boost::undirectedS,
                                    boost::property<boost::vertex_index_t, int>,
                                    boost::property<boost::edge_index_t, int>>;

using Embedding =
    std::vector<std::vector<boost::graph_traits<Graph>::edge_descriptor>>;

using Coordinates = struct {
  size_t x;
  size_t y;
};

using StraightLineDrawing = std::vector<Coordinates>;

struct Box {
  int left;
  int right;
  int top;
  int bottom;
  static Box Union(Box A, Box B);
  static Box Translate(Box A, int x, int y);
};

void InitializeEdgeIndex(Graph& graph) {
  boost::property_map<Graph, boost::edge_index_t>::type e_index =
      boost::get(boost::edge_index, graph);
  boost::graph_traits<Graph>::edges_size_type edge_count = 0;
  boost::graph_traits<Graph>::edge_iterator ei, ei_end;
  for (boost::tie(ei, ei_end) = boost::edges(graph); ei != ei_end; ++ei)
    boost::put(e_index, *ei, edge_count++);
}

bool ComputePlanarEmbedding(const Graph& graph, Embedding& embedding) {
  embedding = Embedding(boost::num_vertices(graph));
  return boost::boyer_myrvold_planarity_test(
      boost::boyer_myrvold_params::graph = graph,
      boost::boyer_myrvold_params::embedding = embedding.data());
}

class GraphPlanar;

struct DrawnEdge {
  int x;
  int vertex_up;
  int vertex_down;
  int y_up;
  int y_down;

  void Draw(Screen& screen, GraphPlanar& graph);
};

struct DrawnVertex {
  int left;
  int right;
  int y;
  std::wstring text;
  std::vector<DrawnEdge> edges;

  void Draw(Screen& screen);
};

enum class Arrow {
  RIGHT,
  LEFT_RIGHT,
  NONE,
  LEFT,
};

struct Edge {
  int from;
  int to;
  Arrow arrow;
};

enum class ArrowStyle {
  NONE,
  LINE,
  ARROW,
};

struct Node {
  int id;
  std::vector<Node> childs;
};

class GraphPlanar : public Translator {
 public:
  virtual ~GraphPlanar() = default;
  std::map<int, std::map<int, ArrowStyle>> arrow_style;

 private:
  const char* Name() final { return "Graph - planar"; }
  const char* Identifier() final { return "GraphPlanar"; }
  const char* Description() final {
    return "Build a graph from node and edges";
  }
  std::vector<Translator::OptionDescription> Options() final;
  std::vector<Translator::Example> Examples() final;
  std::string Translate(const std::string& input,
                        const std::string& options_string) final;

  void Read(const std::string& input);
  void ReadGraph(GraphPlanarParser::GraphContext* graph);
  void ReadEdges(GraphPlanarParser::EdgesContext* edges);
  int ReadNode(GraphPlanarParser::NodeContext* node);
  Arrow ReadArrow(GraphPlanarParser::ArrowContext* arrow);

  void Write();
  void ComputeArrowStyle();

  bool ascii_only_;
  std::string output_;

  std::map<std::wstring, int> name_to_id;
  std::vector<std::wstring> id_to_name;
  int next_id = 0;

  std::vector<Edge> vertex;
};

std::vector<Translator::OptionDescription> GraphPlanar::Options() {
  return {
      {
          "ascii_only",
          {
              "false",
              "true",
          },
          "false",
          "Use the full unicode charset or only ASCII.",
          Widget::Checkbox,
      },
  };
}

std::vector<Translator::Example> GraphPlanar::Examples() {
  return {
      {
          "if then else loop",
          "if -> \"then A\" -> end\n"
          "if -> \"then B\" -> end\n"
          "end -> loop -> if",
      },
      {
          "test",
          "A -- B\n"
          "A -- C\n"
          "A -- D -- G\n"
          "B -- Z\n"
          "C -- Z",
      },
  };
}

std::string GraphPlanar::Translate(const std::string& input,
                                   const std::string& options_string) {
  *this = GraphPlanar();
  auto options = SerializeOption(options_string);
  ascii_only_ = (options["ascii_only"] == "true");

  Read(input);
  Write();
  return output_;
}

void GraphPlanar::Read(const std::string& input) {
  antlr4::ANTLRInputStream input_stream(input);

  // Lexer.
  GraphPlanarLexer lexer(&input_stream);
  antlr4::CommonTokenStream tokens(&lexer);
  tokens.fill();

  // Parser:
  AntlrErrorListener error_listener;
  GraphPlanarParser parser(&tokens);
  parser.addErrorListener(&error_listener);
  GraphPlanarParser::GraphContext* context = nullptr;
  try {
    context = parser.graph();
  } catch (...) {
    return;
  }

  ReadGraph(context);
}

void GraphPlanar::ReadGraph(GraphPlanarParser::GraphContext* graph) {
  for (GraphPlanarParser::EdgesContext* edges : graph->edges()) {
    ReadEdges(edges);
  }
}

void GraphPlanar::ReadEdges(GraphPlanarParser::EdgesContext* edges) {
  std::vector<int> nodes;
  std::vector<Arrow> arrows;
  for (GraphPlanarParser::NodeContext* node : edges->node()) {
    nodes.push_back(ReadNode(node));
  }
  for (GraphPlanarParser::ArrowContext* arrow : edges->arrow()) {
    arrows.push_back(ReadArrow(arrow));
  }
  for (int i = 0; i < arrows.size(); ++i) {
    vertex.push_back(Edge{nodes[i], nodes[i + 1], arrows[i]});
  }
}

int GraphPlanar::ReadNode(GraphPlanarParser::NodeContext* node) {
  std::wstring label;
  if (auto id = node->ID()) {
    label = to_wstring(id->getSymbol()->getText());
  } else if (auto text = node->STRING()) {
    label = to_wstring(text->getSymbol()->getText());
    // Remove the two '"'.
    label = label.substr(1, label.size() - 2);
  } else {
    // NOTREACHED();
  }

  auto it = name_to_id.find(label);
  if (it != name_to_id.end()) {
    return it->second;
  } else {
    name_to_id[label] = next_id;
    id_to_name.push_back(label);
    return next_id++;
  }
}

Arrow GraphPlanar::ReadArrow(GraphPlanarParser::ArrowContext* arrow) {
  if (arrow->RIGHT_ARROW())
    return Arrow::RIGHT;
  if (arrow->NONE_ARROW())
    return Arrow::NONE;
  if (arrow->LEFT_RIGHT_ARROW())
    return Arrow::LEFT_RIGHT;
  if (arrow->LEFT_ARROW())
    return Arrow::LEFT;
  // NOTREACHED();
  return Arrow::RIGHT;
}

void GraphPlanar::ComputeArrowStyle() {
  // Compute ArrowStyle
  for (auto& v : vertex) {
    switch (v.arrow) {
      case Arrow::RIGHT:
        arrow_style[v.from][v.to] = ArrowStyle::ARROW;
        arrow_style[v.to][v.from] = ArrowStyle::LINE;
        break;
      case Arrow::LEFT_RIGHT:
        arrow_style[v.from][v.to] = ArrowStyle::ARROW;
        arrow_style[v.to][v.from] = ArrowStyle::ARROW;
        break;
      case Arrow::LEFT:
        arrow_style[v.from][v.to] = ArrowStyle::LINE;
        arrow_style[v.to][v.from] = ArrowStyle::ARROW;
        break;
      case Arrow::NONE:
        arrow_style[v.from][v.to] = ArrowStyle::LINE;
        arrow_style[v.to][v.from] = ArrowStyle::LINE;
        break;
    }
  }
}

void GraphPlanar::Write() {
  ComputeArrowStyle();

  if (id_to_name.size() <= 2) {
    return;
  }

  int num_vertices = id_to_name.size();

  // Create a graph.
  Graph graph(num_vertices);
  for (auto& it : vertex)
    add_edge(it.from, it.to, graph);
  InitializeEdgeIndex(graph);

  // Make it connected.
  boost::make_connected(graph);
  InitializeEdgeIndex(graph);

  // Make it biconnected.
  Embedding embedding;
  bool is_planar = ComputePlanarEmbedding(graph, embedding);
  if (!is_planar) {
    output_ = "Graph is not planar.\n";
    return;
  }

  boost::make_biconnected_planar(graph, embedding.data());
  InitializeEdgeIndex(graph);
  Graph biconnected_graph(graph);

  ComputePlanarEmbedding(graph, embedding);
  boost::make_maximal_planar(graph, embedding.data());
  InitializeEdgeIndex(graph);

  // Find a canonical ordering.
  ComputePlanarEmbedding(graph, embedding);
  std::vector<boost::graph_traits<Graph>::vertex_descriptor> ordering;
  boost::planar_canonical_ordering(graph, embedding.data(),
                                   std::back_inserter(ordering));

  std::vector<size_t> inverse_ordering(num_vertices);
  for (int i = 0; i < inverse_ordering.size(); ++i) {
    inverse_ordering[ordering[i]] = i;
  }

  StraightLineDrawing straight_line_drawing(num_vertices);

  // Compute the straight line drawing
  boost::chrobak_payne_straight_line_drawing(graph, embedding, ordering.begin(),
                                             ordering.end(),
                                             straight_line_drawing.data());

  auto compare_with = [&](int i) {
    return [&, i](int a, int b) {
      int a_dx = straight_line_drawing[a].x - straight_line_drawing[i].x;
      int a_dy = straight_line_drawing[a].y - straight_line_drawing[i].y;
      int b_dx = straight_line_drawing[b].x - straight_line_drawing[i].x;
      int b_dy = straight_line_drawing[b].y - straight_line_drawing[i].y;
      return a_dx * b_dy - b_dx * a_dy < 0;
    };
  };

  // Compute children.
  std::vector<std::vector<int>> children(num_vertices);
  for (int i = 0; i < num_vertices; ++i) {
    auto& down = children[i];
    auto adjacent = boost::adjacent_vertices(i, graph);
    for (auto j = adjacent.first; j != adjacent.second; ++j) {
      if (inverse_ordering[i] < inverse_ordering[*j]) {
        down.push_back(*j);
      }
    }
    std::sort(down.begin(), down.end(), compare_with(i));
  }

  // Compute Y.
  std::vector<size_t> y(num_vertices, 0);
  for (size_t i : ordering) {
    for (size_t j : children[i]) {
      if (inverse_ordering[i] < inverse_ordering[j]) {
        y[j] = std::max(y[j], y[i] + 1);
      }
    }
  }

  std::vector<DrawnVertex> drawn_vertices(num_vertices);
  std::vector<bool> is_drawn(num_vertices, false);
  std::vector<int> x(y.size(), -1);

  std::function<void(int)> refresh_x = [&](int y) {
    int i = y + 1;
    while (i < x.size() && x[i] < x[y]) {
      x[i] = x[y];
      ++i;
    };
  };

  auto Draw = [&]() {
    // Determine the screen dimension
    int width = 0;
    int height = 0;
    for (int i = 0; i < num_vertices; ++i) {
      if (!is_drawn[i])
        continue;
      width = std::max(width, drawn_vertices[i].right + 1);
      height = std::max(height, 3 * drawn_vertices[i].y + 3);
    }
    Screen screen(width, height);
    for (int i = 0; i < num_vertices; ++i) {
      if (!is_drawn[i])
        continue;
      drawn_vertices[i].Draw(screen);
    }
    for (int i = 0; i < num_vertices; ++i) {
      if (!is_drawn[i])
        continue;
      for (auto& edge : drawn_vertices[i].edges) {
        edge.Draw(screen, *this);
      }
    }

    if (ascii_only_)
      screen.ASCIIfy(1);
    output_ += screen.ToString();
  };

  std::function<void(int)> DrawNode = [&](int i) -> void {
    if (is_drawn[i])
      return;
    DrawnVertex& vertex = drawn_vertices[i];
    int child_left = -1;
    int child_right = -1;
    // Draw our childrens.
    for (auto j : children[i]) {
      DrawNode(j);

      auto& child = drawn_vertices[j];
      DrawnEdge edge;
      edge.vertex_up = i;
      edge.vertex_down = j;
      edge.y_up = y[i] + 1;
      edge.y_down = y[j] - 1;

      // There are no real edge between those two vertex. in this case, do not
      // draw anything.
      if (arrow_style[i][j] == ArrowStyle::NONE) {
        continue;
      }

      // Make the child bigger if we need to.
      child.right = std::max(child.right, x[y[j] - 1] + 3);
      x[y[j]] = child.right;
      refresh_x(y[j]);

      edge.x = child.left + 1;
      edge.x = std::max(edge.x, x[y[i]] + 2);
      if (y[j] != y[i] + 1) {
        edge.x = std::max(edge.x, x[child.y - 1] + 1);
      }
      vertex.edges.push_back(edge);

      x[y[i] + 1] = std::max(x[y[i] + 1], edge.x);
      refresh_x(y[i] + 1);

      if (child_left == -1) {
        child_left = edge.x - 1;
      }
      child_right = edge.x + 1;
    }

    // Draw ourself.
    vertex.text = id_to_name[i];
    vertex.y = y[i];
    vertex.left = std::max(0, std::max(child_left, x[y[i]] + 1));
    vertex.right =
        std::max(child_right, (int)(vertex.left + vertex.text.size() + 1));
    x[y[i]] = vertex.right;
    refresh_x(y[i]);

    // Mark ourself as drawn.
    is_drawn[i] = true;
  };

  for (int i : ordering) {
    if (y[i] == 0) {
      DrawNode(i);
    }
  }

  return Draw();
}

void DrawnVertex::Draw(Screen& screen) {
  screen.DrawBox(left, 3 * y, right - left + 1, 3);
  int text_position = left + 1 + (right - left - 1 - text.size()) / 2;
  screen.DrawText(text_position, 3 * y + 1, text);
}

void DrawnEdge::Draw(Screen& screen, GraphPlanar& graph) {
  int top = 3 * y_up - 1;
  int bottom = 3 * y_down + 3;

  screen.DrawVerticalLine(top + 1, bottom - 1, x);

  if (graph.arrow_style[vertex_down][vertex_up] == ArrowStyle::LINE)
    screen.DrawPixel(x, top, L'┬');
  else
    screen.DrawPixel(x, top, L'△');

  if (graph.arrow_style[vertex_up][vertex_down] == ArrowStyle::LINE)
    screen.DrawPixel(x, bottom, L'┴');
  else
    screen.DrawPixel(x, bottom, L'▽');
}

std::unique_ptr<Translator> GraphPlanarTranslator() {
  return std::make_unique<GraphPlanar>();
}
