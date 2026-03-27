#include <iostream>
#include "convergence_analyser.hpp"

void runAllConvergence(lsm::analysis::ConvergenceAnalyser& analyser, bool isLag) {
    std::string label = isLag ? "Laguerre" : "Monomial";
    std::cout << "\n--- CONVERGENCE REPORT (" << label << ") ---" << std::endl;

    for (const std::string& mode : {"order", "pathCount", "numExerciseDates"}) {
        analyser.runConvergence(mode, isLag);
        // analyser.runSeedStability(mode, isLag);
    }
}

int main(){
    lsm::analysis::ConvergenceAnalyser myAnalyser(100.0, 0.05, 0.2, 100.0, 1.0, true);

    myAnalyser.runBenchmark();

    for (bool isLag : {true, false})
        runAllConvergence(myAnalyser, isLag);

    return 0;
}

