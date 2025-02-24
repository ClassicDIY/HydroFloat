#pragma once

namespace HydroFloat
{
    typedef enum
    {
        NotConnected,
        APMode,
        WSMode,

    } NetworkStatus;

    typedef enum
    {
        off,
        stop,
        slead,
        slag,
        overflow

    } relayStatus;
}