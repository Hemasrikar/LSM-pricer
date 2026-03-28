#include <iostream>
#include "convergence_analyser.hpp"

void runAllConvergence(lsm::analysis::ConvergenceAnalyser& analyser, bool isLag, bool isCall) {
    std::string label = isLag ? "Laguerre" : "Monomial";
    std::cout << "\n--- CONVERGENCE REPORT (" << label << ") ---" << std::endl;

    for (const std::string& mode : {"order", "pathCount", "numExerciseDates"}) {
        analyser.runConvergence(mode, isLag, isCall);
    }

    analyser.runSeedStability(isLag, isCall);
}

int main(){
    lsm::analysis::ConvergenceAnalyser callAnalyser(100.0, 0.05, 0.2, 100.0, 1.0, true);
    lsm::analysis::ConvergenceAnalyser putAnalyser(100.0, 0.05, 0.2, 100.0, 1.0, false);

    putAnalyser.runBenchmark(false);

    for (bool isLag : {true, false})
        runAllConvergence(callAnalyser, isLag, true);

    for (bool isLag : {true, false})
        runAllConvergence(putAnalyser, isLag, false);

    return 0;
}