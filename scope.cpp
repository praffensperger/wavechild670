/************************************************************************************
Wavechild670 v0.1 

scope.cpp

By Peter Raffensperger 10 July 2012

Reference:
Toward a Wave Digital Filter Model of the Fairchild 670 Limiter, Raffensperger, P. A., (2012). 
Proc. of the 15th International Conference on Digital Audio Effects (DAFx-12), 
York, UK, September 17-21, 2012.

Note:
Fairchild (R) a registered trademark of Avid Technology, Inc., which is in no way associated or 
affiliated with the author.

License:
Wavechild670 is licensed under the GNU GPL v2 license. If you use this
software in an academic context, we would appreciate it if you referenced the original
paper.
************************************************************************************/

#include "scope.h"

gnuplot_ctrl* MultiLinePlot(double* x, vector<double*> ys, uint numSamples, vector<string> labels, string title){
	gnuplot_ctrl *graph;
	graph = gnuplot_init();
	gnuplot_resetplot(graph);
	//gnuplot_cmd(graph, "set terminal x11");
	gnuplot_setstyle(graph, "lines");
	stringstream titlecmd;
	titlecmd << "set title \"" << title << "\"";
	gnuplot_cmd(graph, const_cast<char *>(titlecmd.str().c_str()));


	for (uint i = 0; i < ys.size(); ++i) {
		gnuplot_plot_xy(graph, x, ys[i], numSamples, const_cast<char *> (labels[i].c_str()));
		//cout << "Plot #" << i << endl;
		//for (uint j = 0; j < numSamples; j++){
		//	cout << "x=" << x[j] << ", y=" << ys[i][j] << endl;
		//}
	}
	return graph;
}

Scope globalScope;

Scope& GScope(){
	return globalScope;
}

