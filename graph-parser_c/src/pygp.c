#include <Python.h>
#include "topology_parser.h"
#include "graph_parser.h"
#include <stdio.h>
#include <stdlib.h>

static PyObject *
pygraph_parser(PyObject *self, PyObject *args)
{
  multithread = false;
  PyObject * py_bcmap = PyDict_New();
  const char *nj_graph;
  int weight, heuristic, penalization;
  map_id_degree_bc *bc_degree_map = (map_id_degree_bc *) malloc(sizeof(map_id_degree_bc));
  if (!PyArg_ParseTuple(args, "siii", &nj_graph, &weight, &heuristic, &penalization))
      return NULL;
  printf("%d %d %d\n", weight, heuristic, penalization);
  c_graph_parser *cgp = new_graph_parser(weight, heuristic, penalization); //Weight + Heuristic + Cutpoint
  struct topology *topo = parse_netjson(nj_graph);
  graph_parser_parse_simplegraph(cgp, topo);
  graph_parser_calculate_bc(cgp);
  graph_parser_compose_degree_bc_map(cgp, bc_degree_map);    
  int i;
	for (i = 0; i < bc_degree_map->size; i++) {
		PyDict_SetItemString(py_bcmap, bc_degree_map->map[i].id, Py_BuildValue("f", bc_degree_map->map[i].bc));
	}
  return py_bcmap;
}

static PyMethodDef module_methods[] = {
   { "GraphParser", (PyCFunction)pygraph_parser, METH_VARARGS, NULL },
   {NULL}
};


PyMODINIT_FUNC initGraphParser() {
   Py_InitModule3("GraphParser", module_methods, "docstring...");
}
