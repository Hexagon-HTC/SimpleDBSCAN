#pragma once

#include <stdexcept>

namespace gp::exception
{
    //! \brief Exception thrown when cancellation has been triggered.
    class CancelledException : std::exception
    {
    public:
        CancelledException() : std::exception()
        {
        }

        virtual ~CancelledException()
        {
        }

        CancelledException(const CancelledException &) = default;
        CancelledException &operator=(const CancelledException &) = default;
        CancelledException(CancelledException &&) = default;
        CancelledException &operator=(CancelledException &&) = default;

        virtual const char *what() const noexcept
        {
            return "CancelledException";
        }
    };
}