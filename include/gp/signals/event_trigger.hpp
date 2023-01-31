#pragma once

#include <gp/exception/cancelled_exception.hpp>
#include <gp/signals/signals.hpp>

#include <atomic>
#include <memory>

namespace gp::signals
{
    //! \brief Thread-safe event triggering.
    class EventTrigger
    {
    public:
        //! \{
        //! \brief Constructs a new CancelTrigger object.
        //! \param [in] writeMemoryOrder Memory order used to write changes to internal flag.
        //! \param [in] readMemoryOrder Memory order used to read changes from internal flag.
        EventTrigger(std::memory_order writeMemoryOrder = std::memory_order_seq_cst, std::memory_order readMemoryOrder = std::memory_order_seq_cst);
        //!\}

        EventTrigger(const EventTrigger &other) = default;

        EventTrigger(EventTrigger &&other) = default;

        //! \brief Default call - invokes trigger method.
        void operator()() noexcept;

        //! \brief Set internal flag state using preset write memory order.
        void trigger() noexcept;

        //! \brief Read internal flag state using preset write memory order.
        bool isTriggered() const noexcept;

    private:
        //! \brief Memory order used to write changes to internal flag.
        const std::memory_order m_writeMemoryOrder;
        //! \brief Memory order used to read changes from internal flag.
        const std::memory_order m_readMemoryOrder;
        //! \brief Internal state of the trigger.
        std::shared_ptr<std::atomic_bool> m_isTriggered;
    };

    //! \brief Get stop instance of EventTrigger from Signals.
    inline const EventTrigger getStopEventTriggerFromSignals(const Signals &signals)
    {
        const EventTrigger *const maybeEventTrigger = signals.stop.target<EventTrigger>();
        if (!maybeEventTrigger)
        {
            std::logic_error("Signals must use EventTrigger objects as functors.");
        }
        return *maybeEventTrigger;
    }

    //! \brief Check status of stop signal and throw CancelledException if it is triggered.
    inline void checkStopSignalAndThrow(const Signals &signals)
    {
        const EventTrigger cancelTrigger = getStopEventTriggerFromSignals(signals);
        if (cancelTrigger.isTriggered())
        {
            throw gp::exception::CancelledException();
        }
    }
}
