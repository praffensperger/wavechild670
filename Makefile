CC=g++-4.0
CFLAGS=-c -Wall
LDFLAGS=-L/sw/lib -lsndfile 
SOURCES=main.cpp wavechild670.cpp basicdsp.cpp variablemuamplifier.cpp sidechainamplifier.cpp Misc.cpp getopt_pp.cpp gnuplot_i.cpp scope.cpp tubemodel.cpp wdfcircuits.cpp
OBJECTS=$(SOURCES:.cpp=.o)
EXECUTABLE=wavechild670

all: $(SOURCES) $(EXECUTABLE)
	
$(EXECUTABLE): $(OBJECTS) 
	$(CC) $(LDFLAGS) $(OBJECTS) -o $@

.cpp.o:
	$(CC) $(CFLAGS) $< -o $@

clean:
	rm *.o $(EXECUTABLE)