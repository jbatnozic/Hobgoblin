// Copyright 2025 Jovan Batnozic. Released under MS-PL licence in Serbia.
// See https://github.com/jbatnozic/Hobgoblin?tab=readme-ov-file#licence

#ifndef UHOBGOBLIN_MANUAL_TEST_FRAMEWORK
#define UHOBGOBLIN_MANUAL_TEST_FRAMEWORK

#include <functional>
#include <iostream>
#include <regex>
#include <string>
#include <vector>

#include <Hobgoblin/Private/Pmacro_define.hpp>

HOBGOBLIN_NAMESPACE_BEGIN

class ManualTestRunner {
public:
    ManualTestRunner(int aArgc, const char** aArgv)
        : _argc{aArgc}
        , _argv{aArgv} {}

    ManualTestRunner()
        : ManualTestRunner{0, nullptr} {};

    using TestFunction = void (*)(int aArgc, const char** aArgv);

    void addTestCase(std::string aCaseName, TestFunction aTestFunction) {
        _testCases.push_back({std::move(aCaseName), aTestFunction});
    }

    void setTestPreamble(std::function<void()> aFunc) {
        _preamble = std::move(aFunc);
    }

    void setTestCleanup(std::function<void()> aFunc) {
        _cleanup = std::move(aFunc);
    }

    void runTest() {
        const std::regex inputRegex{R"_(^\s*([0-9]+)\s*$)_"};

        for (;;) {
            std::cerr
                << "================================================================================\n"
                << "* Select test case:\n";

            for (std::size_t i = 0; i < _testCases.size(); i += 1) {
                std::cerr << "  " << (i + 1) << ". " << _testCases[i].name << '\n';
            }

            std::string input;
            std::getline(std::cin, input);

            std::smatch smatch;
            if (std::regex_match(input, smatch, inputRegex)) {
                const auto index = std::stoul(smatch[1]) - 1ul;
                if (index >= _testCases.size()) {
                    continue;
                }
                if (_preamble) {
                    _preamble();
                }
                _testCases[static_cast<std::size_t>(index)].func(_argc, _argv);
                if (_cleanup) {
                    _cleanup();
                }
            } else {
                break;
            }
        }
    }

private:
    int          _argc;
    const char** _argv;

    std::function<void()> _preamble;
    std::function<void()> _cleanup;

    struct TestCase {
        std::string  name;
        TestFunction func;
    };

    std::vector<TestCase> _testCases;
};

#define HG_ADD_MANUAL_TEST(_runner_, _test_function_) \
    (_runner_).addTestCase(#_test_function_, _test_function_)

HOBGOBLIN_NAMESPACE_END

#include <Hobgoblin/Private/Pmacro_undef.hpp>

#endif // !UHOBGOBLIN_MANUAL_TEST_FRAMEWORK
