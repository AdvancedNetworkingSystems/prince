from distutils.core import setup, Extension
setup(name='GraphParser', 
      version='1.0',  \
      ext_modules=[Extension('GraphParser', [
          "pygp.c",
          "biconnected.c",
          "brandes.c",
          "graph/graph.c",
          "graph/list.c",
          "graph_parser.c",
          "network_change.c",
          "../../prince/src/topology_parser.c",
          "../../prince/src/topology.c"],
      include_dirs=[
          "../../prince/include",
          "../../graph-parser_c/include"],
      libraries=[
          "m",
          "json-c"]
      )])
