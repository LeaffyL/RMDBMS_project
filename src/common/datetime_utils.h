/* Copyright (c) 2023 Renmin University of China
RMDB is licensed under Mulan PSL v2.
You can use this software according to the terms and conditions of the Mulan PSL v2.
You may obtain a copy of Mulan PSL v2 at:
        http://license.coscl.org.cn/MulanPSL2
THIS SOFTWARE IS PROVIDED ON AN "AS IS" BASIS, WITHOUT WARRANTIES OF ANY KIND,
EITHER EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO NON-INFRINGEMENT,
MERCHANTABILITY OR FIT FOR A PARTICULAR PURPOSE.
See the Mulan PSL v2 for more details. */

#pragma once

#include <array>
#include <cctype>
#include <cstdint>
#include <cstdio>
#include <string>

#include "errors.h"

inline bool is_leap_year(int year) {
    return (year % 4 == 0 && year % 100 != 0) || (year % 400 == 0);
}

inline int days_in_month(int year, int month) {
    static const std::array<int, 12> days = {31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31};
    if (month == 2 && is_leap_year(year)) {
        return 29;
    }
    return days[month - 1];
}

inline int parse_datetime_component(const std::string &value, int begin, int len) {
    int result = 0;
    for (int i = 0; i < len; ++i) {
        char ch = value[begin + i];
        if (!std::isdigit(static_cast<unsigned char>(ch))) {
            throw InvalidDateTimeError(value);
        }
        result = result * 10 + (ch - '0');
    }
    return result;
}

inline int64_t parse_datetime_string(const std::string &value) {
    if (value.size() != 19 || value[4] != '-' || value[7] != '-' || value[10] != ' ' || value[13] != ':' ||
        value[16] != ':') {
        throw InvalidDateTimeError(value);
    }

    int year = parse_datetime_component(value, 0, 4);
    int month = parse_datetime_component(value, 5, 2);
    int day = parse_datetime_component(value, 8, 2);
    int hour = parse_datetime_component(value, 11, 2);
    int minute = parse_datetime_component(value, 14, 2);
    int second = parse_datetime_component(value, 17, 2);

    if (year < 1000 || year > 9999 || month < 1 || month > 12 || day < 1 || day > days_in_month(year, month) ||
        hour < 0 || hour > 23 || minute < 0 || minute > 59 || second < 0 || second > 59) {
        throw InvalidDateTimeError(value);
    }

    int64_t encoded = year;
    encoded = encoded * 100 + month;
    encoded = encoded * 100 + day;
    encoded = encoded * 100 + hour;
    encoded = encoded * 100 + minute;
    encoded = encoded * 100 + second;
    return encoded;
}

inline std::string format_datetime_value(int64_t encoded) {
    int second = encoded % 100;
    encoded /= 100;
    int minute = encoded % 100;
    encoded /= 100;
    int hour = encoded % 100;
    encoded /= 100;
    int day = encoded % 100;
    encoded /= 100;
    int month = encoded % 100;
    encoded /= 100;
    int year = encoded % 10000;

    char buffer[20];
    std::snprintf(buffer, sizeof(buffer), "%04d-%02d-%02d %02d:%02d:%02d", year, month, day, hour, minute, second);
    return std::string(buffer);
}
