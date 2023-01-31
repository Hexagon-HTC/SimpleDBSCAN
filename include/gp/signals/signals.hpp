#pragma once

#include <functional>

namespace gp::signals
{
    //! \brief Structure to hold callbacks which influence execution of program.
    struct Signals
    {
        //! \brief Sets internal callback.
        Signals();

        //! \brief Callback function to terminate execution.
        const std::function<void()> stop;
    };
}
