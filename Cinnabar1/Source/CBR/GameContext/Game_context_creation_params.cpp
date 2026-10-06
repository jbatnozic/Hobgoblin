// Copyright 2026 Jovan Batnozic. Released under MS-PL licence in Serbia.
// See https://github.com/jbatnozic/Hobgoblin?tab=readme-ov-file#licence

#include "Game_context_creation_params.hpp"

#include <argparse/argparse.hpp>

#include <cstdlib>

namespace cinnabar {

void FillInGameContextCreationParamsFromInteractiveCLI(GameContextCreationParams& aParams) {}

int FillInGameContextCreationParamsFromCLIArguments(GameContextCreationParams& aParams,
                                                    int                        aArgumentCount,
                                                    const char* const*         aArgumentArray) {
    argparse::ArgumentParser program{"cinnabar1"};
    program.add_argument("-l", "--log-level")
        .choices("all", "debug", "info", "warning", "error", "fatal")
        .default_value("info")
        .nargs(1)
        .help("Minimum logging level for the application.");

    // start-client

    argparse::ArgumentParser startClientCommand("start-client");
    startClientCommand.add_description("Add file contents to the index");
    startClientCommand.add_argument("--local-port")
        .default_value(0)
        .help("Local port to which the server will bind its socket. Valid values are 0-65535. Passing 0 "
              "(which is also the default) will let the OS choose any valid currently unused port.");
    startClientCommand.add_argument("--server-ip")
        .default_value("127.0.0.1")
        .help("IP address of the server. Defaults to 127.0.0.1 (loopback).");
    startClientCommand.add_argument("--server-port").required().help("Port of the server's socket.");

    // start-server

    argparse::ArgumentParser startServerCommand("start-server");
    startServerCommand.add_description("Start a headless server instance that clients can connect to.");
    startServerCommand.add_argument("--local-port")
        .default_value(0)
        .help("Local port to which the server will bind its socket. Valid values are 0-65535. Passing 0 "
              "(which is also the default) will let the OS choose any valid currently unused port.");
    startServerCommand.add_argument("--players")
        .default_value(1)
        .help("The max number of players that may connect to the game.");

    // start-dev

    argparse::ArgumentParser startDevCommand("start-dev");
    startDevCommand.add_description("Start game in developer mode.");

    program.add_subparser(startClientCommand);
    program.add_subparser(startServerCommand);
    program.add_subparser(startDevCommand);

    try {
        program.parse_args(aArgumentCount, aArgumentArray);
    } catch (const std::exception& err) {
        std::cerr << err.what() << std::endl;
        std::cerr << program;
        return EXIT_FAILURE;
    }

    if (program.is_subcommand_used(startClientCommand)) {
        aParams.mode = GameContextMode::CLIENT;
        std::cerr << "Starting client with logging level " << program.get<std::string>("--log-level")
                  << ", local port " << startClientCommand.get<std::string>("--local-port")
                  << ", server ip " << startClientCommand.get<std::string>("--server-ip")
                  << ", and server port " << startClientCommand.get<std::string>("--server-port")
                  << std::endl;
        return EXIT_SUCCESS;
    }

    if (program.is_subcommand_used(startServerCommand)) {
        aParams.mode = GameContextMode::SERVER;
        std::cerr << "Starting server with logging level " << program.get<std::string>("--log-level")
                  << " and local port " << startServerCommand.get<std::string>("--local-port") << " for "
                  << startServerCommand.get<std::string>("--players") << " players." << std::endl;
        return EXIT_SUCCESS;
    }

    if (program.is_subcommand_used(startDevCommand)) {
        aParams.mode = GameContextMode::DEV;
        std::cerr << "Starting game in developer mode with logging level "
                  << program.get<std::string>("--log-level") << "." << std::endl;
        return EXIT_SUCCESS;
    }

    return EXIT_FAILURE;
}

} // namespace cinnabar
