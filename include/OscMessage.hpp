#pragma once

#include <cstdint>
#include <string>
#include <variant>
#include <vector>


class OscMessage {
    public:
        using Argument = std::variant<int32_t, float, std::string>;

        OscMessage(std::string address, std::vector<Argument> arguments);

        const std::string& address() const;
        const std::vector<Argument>& arguments() const;

    private:
        std::string address_;
        std::vector<Argument> arguments_;
};
