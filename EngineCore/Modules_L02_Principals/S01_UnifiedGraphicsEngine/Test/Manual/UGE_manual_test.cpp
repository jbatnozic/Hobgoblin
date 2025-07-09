// Copyright 2024 Jovan Batnozic. Released under MS-PL licence in Serbia.
// See https://github.com/jbatnozic/Hobgoblin?tab=readme-ov-file#licence

#include <Hobgoblin/HGExcept.hpp>
#include <Hobgoblin/Logging.hpp>
#include <Hobgoblin/Private/Manual_test_framework.hpp>

#include <iostream>
#include <stdexcept>
#include <filesystem>

#include "Test_list.hpp"

namespace hg = jbatnozic::hobgoblin;

int main() try {
    hg::log::SetMinimalLogSeverity(hg::log::Severity::Debug);

    hg::ManualTestRunner testRunner;
    HG_ADD_MANUAL_TEST(testRunner, RunEmptyRenderWindowTest);
    HG_ADD_MANUAL_TEST(testRunner, RunSquareInRenderWindowTest);
    HG_ADD_MANUAL_TEST(testRunner, RunTransformInRenderWindowTest);

    testRunner.runTest();

} catch (const hg::TracedException& ex) {
    std::cout << "Traced exception caught: " << ex.getFullFormattedDescription() << '\n';
    return 1;
} catch (const std::exception& ex) {
    std::cout << "Exception caught: " << ex.what() << '\n';
    return 1;
}
