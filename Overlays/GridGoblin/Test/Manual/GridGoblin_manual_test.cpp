// Copyright 2024 Jovan Batnozic. Released under MS-PL licence in Serbia.
// See https://github.com/jbatnozic/Hobgoblin?tab=readme-ov-file#licence

#include <Hobgoblin/HGExcept.hpp>
#include <Hobgoblin/Logging.hpp>
#include <Hobgoblin/Private/Manual_test_framework.hpp>

#include <exception>
#include <filesystem>
#include <iostream>

#include "Test_list.hpp"

namespace hg = jbatnozic::hobgoblin;

int main() try {
    hg::log::SetMinimalLogSeverity(hg::log::Severity::Debug);

    hg::ManualTestRunner testRunner;

    testRunner.setTestPreamble([]() {
        std::filesystem::remove_all("GGManualTest_WorkDir");
        std::filesystem::create_directory("GGManualTest_WorkDir");
    });
    testRunner.setTestCleanup([]() {
        std::filesystem::remove_all("GGManualTest_WorkDir");
    });

    HG_ADD_MANUAL_TEST(testRunner, RunChunkHolderTest);
    HG_ADD_MANUAL_TEST(testRunner, RunDefaultDiskIoTest);
    // HG_ADD_MANUAL_TEST(testRunner, RunDimetricRenderingTest);
    HG_ADD_MANUAL_TEST(testRunner, RunOpennessTest);
    HG_ADD_MANUAL_TEST(testRunner, RunSpoolingTest);
    HG_ADD_MANUAL_TEST(testRunner, RunVisibilityCalculatorTest);

    testRunner.runTest();

} catch (const hg::TracedException& ex) {
    std::cout << "Traced exception caught: " << ex.getFullFormattedDescription() << '\n';
    return 1;
} catch (const std::exception& ex) {
    std::cout << "Exception caught: " << ex.what() << '\n';
    return 1;
}
