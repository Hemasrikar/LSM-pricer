#include <iostream>
#include "convergence_analyser.hpp"

int main(){

	// code to check whether the benchmark works
	lsm::analysis::ConvergenceAnalyser myAnalyser(100.0, 0.05, 0.2, 100.0, 1.0, true);


	std::cout << "--- BENCHMARK REPORT ---" << std::endl;
	
	std::cout << "\nRunning Benchmark Test..." << std::endl;
	myAnalyser.runBenchmark();
	std::cout << "Benchmark Test Complete" << std::endl;


	std::cout << "\n--- CONVERGENCE REPORT ---" << std::endl;

	std::cout << "\nRunning Basis Order Test..." << std::endl;
	myAnalyser.runConvergence("order");
	std::cout << "Basis Order Test Complete" << std::endl;

	std::cout << "\nRunning Path Test..." << std::endl;
	myAnalyser.runConvergence("pathCount");
	std::cout << "Path Test Complete" << std::endl;

	std::cout << "\nRunning Granularity Test..." << std::endl;
	myAnalyser.runConvergence("numExerciseDates");
	std::cout << "Granularity Test Complete" << std::endl;

    
	return 0;
}