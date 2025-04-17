#!/bin/bash

# Build directory for tests
TEST_BUILD_DIR="build/tests"
mkdir -p $TEST_BUILD_DIR
cd $TEST_BUILD_DIR

# Configure tests
cmake ../.. \
    -DBUILD_TESTING=ON \
    -DCMAKE_BUILD_TYPE=Debug

# Build and run tests
cmake --build .
ctest --output-on-failure

# Generate coverage report
lcov --capture --directory . --output-file coverage.info
lcov --remove coverage.info '/usr/*' --output-file coverage.info
genhtml coverage.info --output-directory coverage_report