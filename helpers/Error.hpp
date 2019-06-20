/**
 * This helper is part of Vulkan4D (open source project)
 * 
 * @author Olivier St-Laurent
 * @date 2019-06-18
 */
#pragma once

#include <csignal>
#include "helpers/Logger.hpp"

// Fatal errors that should terminate the application
// error message in console and exit(1)
#define FATAL(x) {v4d::Logger::ConsoleInstance().LogError(ostringstream("FATAL: ").flush() << x << " [" << __FILE__ << ":" << __LINE__ << "]"); std::exit(1);}
// error message in console and abort() (causes breakpoint in debug)
#define FATAL_ABORT(x) {v4d::Logger::ConsoleInstance().LogError(ostringstream("FATAL(abort): ").flush() << x << " [" << __FILE__ << ":" << __LINE__ << "]"); std::abort();}
// error message in console and raise SIGINT (causes breakpoint in debug)
#define FATAL_INTERUPT(x) {v4d::Logger::ConsoleInstance().LogError(ostringstream("FATAL(interupt): ").flush() << x << " [" << __FILE__ << ":" << __LINE__ << "]"); raise(SIGINT);}
// error message in console and raise SIGKILL (emergency kill the application)
#define FATAL_KILL(x) {v4d::Logger::ConsoleInstance().LogError(ostringstream("FATAL(kill): ").flush() << x << " [" << __FILE__ << ":" << __LINE__ << "]"); raise(SIGKILL);}
