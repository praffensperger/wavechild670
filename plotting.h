/************************************************************************************
* 
* Wavechild670 v0.1 
* 
* plotting.h
* 
* By Peter Raffensperger 11 March 2014
* 
* Reference:
* Toward a Wave Digital Filter Model of the Fairchild 670 Limiter, Raffensperger, P. A., (2012). 
* Proc. of the 15th International Conference on Digital Audio Effects (DAFx-12), 
* York, UK, September 17-21, 2012.
* 
* Note:
* Fairchild (R) a registered trademark of Avid Technology, Inc., which is in no way associated or 
* affiliated with the author.
* 
* License:
* Wavechild670 is licensed under the GNU GPL v2 license. If you use this
* software in an academic context, we would appreciate it if you referenced the original
* paper.
* 
************************************************************************************/



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