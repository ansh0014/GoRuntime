#pragma once

#include <string>

namespace goruntime {

enum class ErrorCode {
    Ok = 0,
    InvalidArgument,
    Closed,
    Shutdown,
    Unknown
};

struct Error {
    ErrorCode code{ErrorCode::Ok};
    std::string message;

    static Error ok() { return {}; }
    static Error invalid_argument(const std::string& msg) {
        return {ErrorCode::InvalidArgument, msg};
    }
    static Error closed(const std::string& msg) {
        return {ErrorCode::Closed, msg};
    }
    static Error shutdown(const std::string& msg) {
        return {ErrorCode::Shutdown, msg};
    }
};

} // namespace goruntime