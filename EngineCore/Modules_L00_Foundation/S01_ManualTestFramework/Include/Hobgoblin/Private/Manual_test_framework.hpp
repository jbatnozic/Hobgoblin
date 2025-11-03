// Copyright 2025 Jovan Batnozic. Released under MS-PL licence in Serbia.
// See https://github.com/jbatnozic/Hobgoblin?tab=readme-ov-file#licence

#ifndef UHOBGOBLIN_MANUAL_TEST_FRAMEWORK
#define UHOBGOBLIN_MANUAL_TEST_FRAMEWORK

#include <cstring>
#include <functional>
#include <iostream>
#include <regex>
#include <string>
#include <vector>

#include <Hobgoblin/Private/Pmacro_define.hpp>

HOBGOBLIN_NAMESPACE_BEGIN
namespace {

class ManualTestRunner {
public:
    ManualTestRunner(int aArgc, const char** aArgv) {
        _parseCommandLineArguments(aArgc, aArgv);
    }

    ManualTestRunner()
        : ManualTestRunner{0, nullptr} {};

    using TestFunction = void (*)(const std::vector<const char*>& aArgs);

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
            std::string input;
            std::smatch smatch;

            if (!_specialParams.specificCase) {
                std::cerr << "=========================================================================="
                             "======\n"
                          << "* Select test case:\n";

                for (std::size_t i = 0; i < _testCases.size(); i += 1) {
                    std::cerr << "  " << (i + 1) << ". " << _testCases[i].name << '\n';
                }

                std::getline(std::cin, input);
            }

            if (_specialParams.specificCase || std::regex_match(input, smatch, inputRegex)) {
                const auto index = _specialParams.specificCase ? _specialParams.specificCaseIndex
                                                               : (std::stoul(smatch[1]) - 1ul);
                if (index < _testCases.size()) {
                    auto tries = _specialParams.repeat ? _specialParams.repeatCount : 1;
                    for (; tries > 0; --tries) {
                        if (_preamble) {
                            _preamble();
                        }
                        _testCases[static_cast<std::size_t>(index)].func(_args);
                        if (_cleanup) {
                            _cleanup();
                        }
                    }
                }
            } else {
                break;
            }

            if (_specialParams.specificCase) {
                break;
            }
        }
    }

private:
    std::vector<const char*> _args;

    struct SpecialParams {
        bool          specificCase      = false;
        unsigned long specificCaseIndex = 0;
        bool          repeat            = false;
        unsigned long repeatCount       = 0;
    } _specialParams;

    std::function<void()> _preamble;
    std::function<void()> _cleanup;

    struct TestCase {
        std::string  name;
        TestFunction func;
    };

    std::vector<TestCase> _testCases;

    void _parseCommandLineArguments(int aArgc, const char** aArgv) {
        const auto argCount = static_cast<std::size_t>(aArgc);

        if (argCount == 0u) {
            return;
        }

        if (argCount == 1u) {
            _args.push_back(aArgv[0]);
            return;
        }

        const auto begin                 = aArgv + 1;
        const auto end                   = aArgv + argCount;
        const auto testArgsSeparatorIter = std::find_if(begin, end, [](const char* aArg) -> bool {
            return (std::strcmp(aArg, "--") == 0);
        });

        if (testArgsSeparatorIter == end) {
            _args.resize(argCount);
            std::memcpy(_args.data(), aArgv, sizeof(aArgv[0]) * argCount);
            return;
        }

        _args.resize(static_cast<std::size_t>(end - testArgsSeparatorIter));
        _args[0] = aArgv[0];
        std::memcpy(&_args[1], testArgsSeparatorIter + 1, sizeof(aArgv[0]) * (_args.size() - 1));

        auto         remainingSpecialParams = testArgsSeparatorIter - begin;
        const char** arg                    = begin;
        for (; remainingSpecialParams > 0; --remainingSpecialParams, ++arg) {
            if (std::strcmp(*arg, "--case") == 0 && remainingSpecialParams > 0) {
                _specialParams.specificCase      = true;
                _specialParams.specificCaseIndex = std::stoul(*(arg + 1)) - 1;
                --remainingSpecialParams, ++arg;
                continue;
            }

            if (std::strcmp(*arg, "--repeat") == 0 && remainingSpecialParams > 0) {
                _specialParams.repeat      = true;
                _specialParams.repeatCount = std::stoul(*(arg + 1));
                --remainingSpecialParams, ++arg;
                continue;
            }
        }
    }
};

#define HG_ADD_MANUAL_TEST(_runner_, _test_function_) \
    (_runner_).addTestCase(#_test_function_, _test_function_)

} // namespace
HOBGOBLIN_NAMESPACE_END

#include <Hobgoblin/Private/Pmacro_undef.hpp>

#endif // !UHOBGOBLIN_MANUAL_TEST_FRAMEWORK
