#ifndef TRANSLATOR_PLANAR_GRAPH_PLANAR_GRAPH_H
#define TRANSLATOR_PLANAR_GRAPH_PLANAR_GRAPH_H

#include <map>
#include <vector>
#include <string>

#include "translator/Translator.h"
#include "translator/planar_graph/PlanarGraphParser.h"

struct Box {
  int left;
  int right;
  int top;
  int bottom;
  static Box Union(Box A, Box B);
  static Box Translate(Box A, int x, int y);
};

class PlanarGraph : public Translator {
 public:
  virtual ~PlanarGraph() = default;
  std::string Translate(const std::string& input,
                        const std::string& options) override;

  //----------------------------------------------------------------------------
 private:
  bool ascii_only_;
  std::string output_;

  std::map<std::wstring, int> name_to_id;
  std::vector<std::wstring> id_to_name;
  int next_id = 0;

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
  std::vector<Edge> vertex;

  enum class ArrowStyle {
    NONE,
    LINE,
    ARROW
  };
  std::map<int, std::map<int, ArrowStyle>> arrow_style;

  struct DrawnEdge;
  struct DrawnVertex;

  struct Node {
    int id;
    std::vector<Node> childs;
  };

  //----------------------------------------------------------------------------
  void Read(const std::string& input);
  void ReadGraph(PlanarGraphParser::GraphContext* graph);
  void ReadEdges(PlanarGraphParser::EdgesContext* edges);
  int ReadNode(PlanarGraphParser::NodeContext* node);
  Arrow ReadArrow(PlanarGraphParser::ArrowContext* arrow);

  void Write();
  void ComputeArrowStyle();
  //----------------------------------------------------------------------------

  const char* Name() override;
  const char* Description() override;
  std::vector<OptionDescription> Options() override;
  std::vector<Example> Examples() override;
};

#endif /* end of include guard: TRANSLATOR_PLANAR_GRAPH_PLANAR_GRAPH_H */
