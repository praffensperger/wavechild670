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

