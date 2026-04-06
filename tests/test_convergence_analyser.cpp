#define CATCH_CONFIG_MAIN
#include "catch.hpp"
#include <memory>
#include <fstream>
#include <cstdlib>
#include "underlying_sde.hpp"
#include "convergence_analyser.hpp"

static std::function<std::unique_ptr<lsm::core::StochasticProcess>(double, double)> sdeFactory =
    [](double r, double vol) {
        return std::make_unique<lsm::core::GeometricBrownianMotion>(r, vol);
    };
static lsm::core::Call_payoff call(100.0);
static lsm::core::Put_payoff put(100.0);
static lsm::core::BasisSet basis = []() {
    lsm::core::BasisSet b;
    b.makeLaguerreSet(3);
    return b;
}();
static std::function<void(lsm::core::BasisSet&, int)> basisFactory = [](lsm::core::BasisSet& b, int order) {
    b.makeLaguerreSet(order);
};
static lsm::analysis::ConvergenceAnalyser callAnalyser(100.0, 0.05, 0.2, 100.0, 1.0, sdeFactory, call, basis, basisFactory, 3, 10000, 50);
static lsm::analysis::ConvergenceAnalyser putAnalyser(100.0, 0.05, 0.2, 100.0, 1.0, sdeFactory, put, basis, basisFactory, 3, 10000, 50);

using namespace lsm::analysis;


// ============================================ getBSPrice() tests ============================================
// Call >0
TEST_CASE("getBSPrice returns a positive price", "[convergence]") {
    REQUIRE(callAnalyser.getBSPrice() > 0.0);
}

// Put >0
TEST_CASE("getBSPrice returns a positive price for put", "[convergence]") {
    REQUIRE(putAnalyser.getBSPrice() > 0.0);
}

// Put Call Parity
TEST_CASE("BS call and put satisfy put-call parity", "[convergence]") {
    double callPrice = callAnalyser.getBSPrice();
    double putPrice  = putAnalyser.getBSPrice();
    double parity    = 100.0 - 100.0 * std::exp(-0.05 * 1.0);
    REQUIRE(std::abs((callPrice - putPrice) - parity) < 0.01);
}




// ============================================ getFDPrice() tests ============================================
// Call > 0
TEST_CASE("getFDPrice returns a positive price", "[convergence]") {
    REQUIRE(callAnalyser.getFDPrice() > 0.0);
}

// Put > 0
TEST_CASE("getFDPrice returns a positive price for put", "[convergence]") {
    REQUIRE(putAnalyser.getFDPrice() > 0.0);
}

// American put >= European put (early exercise premium)
TEST_CASE("getFDPrice American put is at least European BS put price", "[convergence]") {
    REQUIRE(putAnalyser.getFDPrice() >= putAnalyser.getBSPrice());
}

// Higher sigma -> higher price
TEST_CASE("getFDPrice increases with higher volatility", "[convergence]") {
    lsm::core::Put_payoff localPut(100.0);
    lsm::core::BasisSet localBasis;
    localBasis.makeLaguerreSet(3);
    lsm::analysis::ConvergenceAnalyser highVolAnalyser(100.0, 0.05, 0.4, 100.0, 1.0, sdeFactory, localPut, localBasis, basisFactory, 3, 10000, 50);
    REQUIRE(highVolAnalyser.getFDPrice() > putAnalyser.getFDPrice());
}

// Moneyness: ITM call > OTM call
TEST_CASE("getFDPrice call is higher for ITM than OTM", "[convergence]") {
    lsm::core::Call_payoff localCall(100.0);
    lsm::core::BasisSet itmBasis, otmBasis;
    itmBasis.makeLaguerreSet(3);
    otmBasis.makeLaguerreSet(3);
    lsm::analysis::ConvergenceAnalyser itmAnalyser(110.0, 0.05, 0.2, 100.0, 1.0, sdeFactory, localCall, itmBasis, basisFactory, 3, 10000, 50);
    lsm::analysis::ConvergenceAnalyser otmAnalyser( 90.0, 0.05, 0.2, 100.0, 1.0, sdeFactory, localCall, otmBasis, basisFactory, 3, 10000, 50);
    REQUIRE(itmAnalyser.getFDPrice() > otmAnalyser.getFDPrice());
}






// ============================================ getLSMPrice() tests ============================================
// Call > 0
TEST_CASE("getLSMPrice returns a positive price", "[convergence]") {
    REQUIRE(callAnalyser.getLSMPrice(24, 50, 3, 1000) > 0.0);
}

// Put > 0
TEST_CASE("getLSMPrice returns a positive price for put", "[convergence]") {
    REQUIRE(putAnalyser.getLSMPrice(24, 50, 3, 1000) > 0.0);
}

// American put >= European put (early exercise premium)
TEST_CASE("getLSMPrice American put is at least European BS put price", "[convergence]") {
    REQUIRE(putAnalyser.getLSMPrice(24, 50, 3, 1000) >= putAnalyser.getBSPrice());
}

// Higher sigma -> higher price
TEST_CASE("getLSMPrice increases with higher volatility", "[convergence]") {
    lsm::core::Put_payoff localPut(100.0);
    lsm::core::BasisSet localBasis;
    localBasis.makeLaguerreSet(3);
    lsm::analysis::ConvergenceAnalyser highVolAnalyser(100.0, 0.05, 0.4, 100.0, 1.0, sdeFactory, localPut, localBasis, basisFactory, 3, 10000, 50);
    REQUIRE(highVolAnalyser.getLSMPrice(24, 50, 3, 1000) > putAnalyser.getLSMPrice(24, 50, 3, 1000));
}

// Moneyness: ITM call > OTM call
TEST_CASE("getLSMPrice call is higher for ITM than OTM", "[convergence]") {
    lsm::core::Call_payoff localCall(100.0);
    lsm::core::BasisSet itmBasis, otmBasis;
    itmBasis.makeLaguerreSet(3);
    otmBasis.makeLaguerreSet(3);
    lsm::analysis::ConvergenceAnalyser itmAnalyser(110.0, 0.05, 0.2, 100.0, 1.0, sdeFactory, localCall, itmBasis, basisFactory, 3, 10000, 50);
    lsm::analysis::ConvergenceAnalyser otmAnalyser( 90.0, 0.05, 0.2, 100.0, 1.0, sdeFactory, localCall, otmBasis, basisFactory, 3, 10000, 50);
    REQUIRE(itmAnalyser.getLSMPrice(24, 50, 3, 1000) > otmAnalyser.getLSMPrice(24, 50, 3, 1000));
}

// Seed stability: two different seeds should give prices within 5% of each other
TEST_CASE("getLSMPrice is stable across different seeds", "[convergence]") {
    double price1 = putAnalyser.getLSMPrice(1,  50, 3, 5000);
    double price2 = putAnalyser.getLSMPrice(42, 50, 3, 5000);
    double relDiff = std::abs(price1 - price2) / ((price1 + price2) / 2.0);
    REQUIRE(relDiff < 0.05);
}




// ============================================ runBenchmark() tests ============================================
static int countLines(const std::string& path) {
    std::ifstream f(path);
    int n = 0;
    std::string line;
    while (std::getline(f, line)) n++;
    return n;
}

// CSV file is created
TEST_CASE("runBenchmark creates benchmark.csv", "[convergence]") {
    system("mkdir -p csv_output");
    putAnalyser.runBenchmark();
    REQUIRE(std::ifstream("csv_output/benchmark.csv").good());
}

// Correct header
TEST_CASE("runBenchmark CSV has correct header", "[convergence]") {
    std::ifstream f("csv_output/benchmark.csv");
    std::string header;
    std::getline(f, header);
    REQUIRE(header == "S0,Sigma,T,BSPrice,FDPrice,LSMPrice,EarlyExPremium");
}

// Correct number of rows and columns in data file
TEST_CASE("runBenchmark CSV has 12 data rows", "[convergence]") {
    REQUIRE(countLines("csv_output/benchmark.csv") == 13);
}


// ============================================ runPathConvergence() tests ============================================
static const std::string pathConvFile = "csv_output/path_convergence_S100_K100_r0.05_sig0.2_T1_put_Laguerre_L0_ord3_dates50_paths10000.csv";

// CSV file is created
TEST_CASE("runPathConvergence creates CSV file", "[convergence]") {
    system("mkdir -p csv_output");
    putAnalyser.runPathConvergence({100, 200});
    REQUIRE(std::ifstream(pathConvFile).good());
}

// Correct header
TEST_CASE("runPathConvergence CSV has correct header", "[convergence]") {
    std::ifstream f(pathConvFile);
    std::string header;
    std::getline(f, header);
    REQUIRE(header == "Paths,LSMPrice,FDPrice,Error,Time(ms)");
}

// Correct number of rows (one per path count)
TEST_CASE("runPathConvergence CSV has correct number of rows", "[convergence]") {
    REQUIRE(countLines(pathConvFile) == 3); // 1 header + 2 data rows
}




// ============================================ runDatesConvergence() tests ============================================
static const std::string datesConvFile = "csv_output/dates_convergence_S100_K100_r0.05_sig0.2_T1_put_Laguerre_L0_ord3_dates50_paths10000.csv";

// CSV file is created
TEST_CASE("runDatesConvergence creates CSV file", "[convergence]") {
    system("mkdir -p csv_output");
    putAnalyser.runDatesConvergence({10, 20});
    REQUIRE(std::ifstream(datesConvFile).good());
}

// Correct header
TEST_CASE("runDatesConvergence CSV has correct header", "[convergence]") {
    std::ifstream f(datesConvFile);
    std::string header;
    std::getline(f, header);
    REQUIRE(header == "ExerciseDates,LSMPrice,FDPrice,Error,Time(ms)");
}

// Correct number of rows (one per exercise date count)
TEST_CASE("runDatesConvergence CSV has correct number of rows", "[convergence]") {
    REQUIRE(countLines(datesConvFile) == 3); // 1 header + 2 data rows
}




// ============================================ runOrderConvergence() tests ============================================
static const std::string orderConvFile = "csv_output/order_convergence_S100_K100_r0.05_sig0.2_T1_put_Laguerre_L0_ord3_dates50_paths10000.csv";

// CSV file is created
TEST_CASE("runOrderConvergence creates CSV file", "[convergence]") {
    system("mkdir -p csv_output");
    putAnalyser.runOrderConvergence({2, 3});
    REQUIRE(std::ifstream(orderConvFile).good());
}

// Correct header
TEST_CASE("runOrderConvergence CSV has correct header", "[convergence]") {
    std::ifstream f(orderConvFile);
    std::string header;
    std::getline(f, header);
    REQUIRE(header == "Order,LSMPrice,FDPrice,Error,Time(ms)");
}

// Correct number of rows (one per order)
TEST_CASE("runOrderConvergence CSV has correct number of rows", "[convergence]") {
    REQUIRE(countLines(orderConvFile) == 3); // 1 header + 2 data rows
}




// ============================================ runFDConvergence() tests ============================================
static const std::string fdConvFile = "csv_output/fd_convergence_S100_K100_r0.05_sig0.2_T1_put_Laguerre_L0_ord3_dates50_paths10000.csv";

// CSV file is created
TEST_CASE("runFDConvergence creates CSV file", "[convergence]") {
    system("mkdir -p csv_output");
    putAnalyser.runFDConvergence({10, 20}, 50);
    REQUIRE(std::ifstream(fdConvFile).good());
}

// Correct header
TEST_CASE("runFDConvergence CSV has correct header", "[convergence]") {
    std::ifstream f(fdConvFile);
    std::string header;
    std::getline(f, header);
    REQUIRE(header == "TimeSteps,BSPrice,FDPrice,Error,Time(ms)");
}

// Correct number of rows (one per time step count)
TEST_CASE("runFDConvergence CSV has correct number of rows", "[convergence]") {
    REQUIRE(countLines(fdConvFile) == 3); // 1 header + 2 data rows
}




// ============================================ runSeedStability() tests ============================================
static const std::string seedFile = "csv_output/seed_stability_S100_K100_r0.05_sig0.2_T1_put_Laguerre_L0_ord3_dates50_paths10000.csv";

// CSV file is created
TEST_CASE("runSeedStability creates CSV file", "[convergence]") {
    system("mkdir -p csv_output");
    putAnalyser.runSeedStability();
    REQUIRE(std::ifstream(seedFile).good());
}

// Correct header
TEST_CASE("runSeedStability CSV has correct header", "[convergence]") {
    std::ifstream f(seedFile);
    std::string header;
    std::getline(f, header);
    REQUIRE(header == "Seed,PlainPrice,PlainSE,AntitheticPrice,AntitheticSE");
}

// Correct number of rows (10 seeds + Mean + StdDev summary rows)
TEST_CASE("runSeedStability CSV has correct number of rows", "[convergence]") {
    REQUIRE(countLines(seedFile) == 13); // 1 header + 10 seed rows + 2 summary rows
}
