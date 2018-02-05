#include "commands/genkey.hpp"
#include "commands/root-ca.hpp"
#include "commands/auth-ca.hpp"
#include "commands/show-cert.hpp"
#include "commands/sign-ticket.hpp"
#include "options.hpp"
#include <boost/program_options.hpp>
#include <iostream>
#include <memory>

namespace po = boost::program_options;

std::unique_ptr<Command> parse_options(int argc, const char *argv[])
{
    po::options_description global("Global options");
    global.add_options()
        ("command", po::value<std::string>(), "Command to execute.")
        ("subargs", po::value<std::vector<std::string> >(), "Arguments for command.")
    ;

    po::positional_options_description pos;
    pos.add("command", 1);
    pos.add("subargs", -1);

    po::variables_map vm;

    po::parsed_options parsed = po::command_line_parser(argc, argv)
        .options(global)
        .positional(pos)
        .allow_unregistered()
        .run();

    po::store(parsed, vm);
    po::notify(vm);

    if (!vm.count("command")) {
        std::cerr << global << std::endl;
        std::cerr << "Available commands: root-ca, auth-ca, sign-ticket, show-cert" << std::endl;

        return nullptr;
    }

    std::string cmd = vm["command"].as<std::string>();
    std::unique_ptr<Command> command;

    if (cmd == "--help") {
        std::cerr << global << std::endl;
        std::cerr << "Available commands: root-ca, auth-ca, sign-ticket, show-cert" << std::endl;
    } else if (cmd == "genkey") {
        command.reset(new GenkeyCommand());
    } else if (cmd == "root-ca") {
        command.reset(new RootCaCommand());
    } else if (cmd == "auth-ca") {
        command.reset(new AuthCaCommand());
    } else if (cmd == "sign-ticket") {
        command.reset(new SignTicketCommand());
    } else if (cmd == "show-cert") {
        command.reset(new ShowCertCommand());
    } else {
        // unrecognized command
        throw po::invalid_option_value(cmd);
    }

    std::vector<std::string> opts = po::collect_unrecognized(parsed.options, po::include_positional);

    if (!command->parse(opts)) {
        return nullptr;
    }

    return command;
}
