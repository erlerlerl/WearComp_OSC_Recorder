#include "OscMessage.hpp"

OscMessage::OscMessage(std::string address, std::vector<Argument> arguments)
    : address_(std::move(address)), arguments_(std::move(arguments)) {}

const std::string &OscMessage::address() const {
    return address_;
}

const std::vector<OscMessage::Argument> &OscMessage::arguments() const {
    return arguments_;
}
