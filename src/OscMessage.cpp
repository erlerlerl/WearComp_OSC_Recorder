#include "OscMessage.hpp"

OscMessage::OscMessage(Timestamp timestamp, std::string address, std::vector<Argument> arguments)
    : timestamp_(timestamp), address_(std::move(address)), arguments_(std::move(arguments)) {}

OscMessage::Timestamp OscMessage::timestamp() const {
    return timestamp_;
}

const std::string &OscMessage::address() const {
    return address_;
}

const std::vector<OscMessage::Argument> &OscMessage::arguments() const {
    return arguments_;
}
