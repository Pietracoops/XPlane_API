#pragma once

#ifndef _LOGGING_H_
#define _LOGGING_H_

#include <iostream>

inline std::ostream& plugin_log()
{
    return std::cerr << "[Plugin  ] ";
}

inline std::ostream& modbus_log()
{
    return std::cerr << "[Modbus  ] ";
}

inline std::ostream& enip_log()
{
    return std::cerr << "[ENIP    ] ";
}

#endif