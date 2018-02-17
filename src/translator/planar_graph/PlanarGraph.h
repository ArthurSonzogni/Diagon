#ifndef TRANSLATOR_PLANAR_GRAPH_PLANAR_GRAPH_H
#define TRANSLATOR_PLANAR_GRAPH_PLANAR_GRAPH_H

#include <map>
#include <vector>
#include <string>

#include "translator/Translator.h"
#include "translator/planar_graph/PlanarGraphParser.h"

class PlanarGraph : public Translator {
 public:
  virtual ~PlanarGraph() = default;
  void Process(const std::string& input) override;
  std::string Output() override { return output_; }

  //----------------------------------------------------------------------------
 private:
  enum class Arrow {
    RIGHT,
    LEFT_RIGHT,
    NONE,
    LEFT,
  };


  struct Vertex {
    int from;
    int to;
    Arrow arrow;
  };

  std::vector<Vertex> vertex;
  std::map<std::wstring, int> name_to_id;
  std::vector<std::wstring> id_to_name;
  int next_id = 0;

  std::string output_;

  //----------------------------------------------------------------------------
  void Read(const std::string& input);
  void ReadGraph(PlanarGraphParser::GraphContext* graph);
  void ReadEdges(PlanarGraphParser::EdgesContext* edges);
  int ReadNode(PlanarGraphParser::NodeContext* node);
  Arrow ReadArrow(PlanarGraphParser::ArrowContext* arrow);
  void Write();
  std::wstring ArrowToString(Arrow arrow);
  //----------------------------------------------------------------------------
};

#endif /* end of include guard: TRANSLATOR_PLANAR_GRAPH_PLANAR_GRAPH_H */
