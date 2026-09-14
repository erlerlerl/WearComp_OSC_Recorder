#pragma once

#include <cstdint>
#include <string>
#include <variant>
#include <vector>
#include <chrono>


class OscMessage {
    public:
        using Argument = std::variant<int32_t, float, std::string>;
        using Timestamp = std::chrono::duration<double>;

        OscMessage(Timestamp timestamp, std::string address, std::vector<Argument> arguments);

        Timestamp timestamp() const;
        const std::string& address() const;
        const std::vector<Argument>& arguments() const;

    private:
        Timestamp timestamp_;
        std::string address_;
        std::vector<Argument> arguments_;
};
