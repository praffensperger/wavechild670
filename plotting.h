#ifndef PROBE_H
#define PROBE_H

#include "gnuplot_i.h"

void Plot(double* x, double** ys, ){
	gnuplot_ctrl *graph;
	graph = gnuplot_init();
	gnuplot_resetplot(graph);
	gnuplot_setstyle(graph, "lines");

	for (int i = 0; i < Y.nrows(); ++i) {
		RowVector R(Y.row(i+1));
		gnuplot_plot_xy(graph, X.data(), R.data(), X.ncols(), const_cast<char *> (msg[i].c_str()));
	}

}
#endif