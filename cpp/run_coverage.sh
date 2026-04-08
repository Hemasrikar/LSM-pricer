#!/bin/bash
# run_coverage.sh
# Builds the project with instrumentation, runs all tests, and produces an HTML
# coverage report showing which lines in src/ are reached by the test suite.
#
# Requirements (one of):
#   gcovr  -> sudo apt-get install gcovr          (preferred)
#   lcov   -> sudo apt-get install lcov
#
# macOS note: Apple Clang ships llvm-cov, not gcov.  Either install GCC via
#   Homebrew (brew install gcc gcovr) and re-point cmake to it, or run inside
#   the provided devcontainer where GCC is the default compiler.

set -euo pipefail

BUILD_DIR="build_coverage"
REPORT_DIR="coverage_report"
REPORT_ONLY=false

if [[ "${1:-}" == "--report-only" ]]; then
    REPORT_ONLY=true
    if [ ! -d "$BUILD_DIR" ]; then
        echo "Error: no coverage build found at '$BUILD_DIR'. Run without --report-only first."
        exit 1
    fi
fi

# ── tool detection ────────────────────────────────────────────────────────────
if ! command -v gcov &>/dev/null && ! command -v llvm-cov &>/dev/null; then
    echo "Error: gcov not found. Install GCC or LLVM."
    exit 1
fi

if command -v gcovr &>/dev/null; then
    TOOL="gcovr"
elif command -v lcov &>/dev/null && command -v genhtml &>/dev/null; then
    TOOL="lcov"
else
    echo "Error: neither gcovr nor lcov/genhtml found."
    echo "  Install gcovr : sudo apt-get install gcovr"
    echo "  Install lcov  : sudo apt-get install lcov"
    exit 1
fi

echo "Using coverage tool: $TOOL"

# ── clean slate (skipped in report-only mode) ─────────────────────────────────
if [ "$REPORT_ONLY" = false ]; then
    rm -rf "$BUILD_DIR"
fi
rm -rf "$REPORT_DIR"
mkdir -p "$REPORT_DIR"

if [ "$REPORT_ONLY" = false ]; then
    # ── configure ─────────────────────────────────────────────────────────────
    echo ""
    echo "=== Configuring (coverage build) ==="
    cmake -S . -B "$BUILD_DIR" \
        -DCMAKE_BUILD_TYPE=Debug \
        -DCMAKE_CXX_FLAGS="--coverage -O0" \
        -DCMAKE_EXE_LINKER_FLAGS="--coverage"

    # ── build ─────────────────────────────────────────────────────────────────
    echo ""
    echo "=== Building ==="
    cmake --build "$BUILD_DIR" --parallel

    # ── run tests ─────────────────────────────────────────────────────────────
    echo ""
    echo "=== Running tests ==="
    (cd "$BUILD_DIR" && ctest --output-on-failure)
fi

# ── generate report ───────────────────────────────────────────────────────────
echo ""
echo "=== Generating coverage report ==="

if [ "$TOOL" = "gcovr" ]; then
    gcovr \
        --root . \
        --filter 'src/' \
        --exclude 'src/main\.cpp' \
        --exclude 'src/generate_convergence_output\.cpp' \
        --exclude-unreachable-branches \
        --exclude-throw-branches \
        --print-summary \
        --txt \
        --html-details "$REPORT_DIR/index.html" \
        "$BUILD_DIR"

else
    # ── lcov path ─────────────────────────────────────────────────────────────
    RAW_INFO="$REPORT_DIR/coverage_raw.info"
    FILTERED_INFO="$REPORT_DIR/coverage.info"

    lcov --capture \
         --directory "$BUILD_DIR" \
         --output-file "$RAW_INFO" \
         --rc lcov_branch_coverage=1

    # Strip everything except our own src/ files
    lcov --extract "$RAW_INFO" \
         "$(pwd)/src/*" \
         --output-file "$FILTERED_INFO" \
         --rc lcov_branch_coverage=1

    lcov --remove "$FILTERED_INFO" \
         "$(pwd)/src/main.cpp" \
         "$(pwd)/src/generate_convergence_output.cpp" \
         --output-file "$FILTERED_INFO" \
         --rc lcov_branch_coverage=1

    genhtml "$FILTERED_INFO" \
            --output-directory "$REPORT_DIR" \
            --branch-coverage \
            --title "LSM Pricer – Test Coverage"

    echo ""
    lcov --summary "$FILTERED_INFO" --rc lcov_branch_coverage=1
fi

echo ""
echo "Coverage report written to: $REPORT_DIR/index.html"

# Open the report in the default browser
if command -v open &>/dev/null; then
    open "$REPORT_DIR/index.html"          # macOS
elif command -v xdg-open &>/dev/null; then
    xdg-open "$REPORT_DIR/index.html"     # Linux
fi
