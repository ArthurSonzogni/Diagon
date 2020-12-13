add_library(translator_graph_dag STATIC
  GraphDAG.cpp
  dag_to_graph.cpp
)
target_link_libraries(translator_graph_dag PRIVATE diagon_base)
target_set_common(translator_graph_dag)
