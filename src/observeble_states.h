#ifndef CNC_3018_OBSERVEBLE_STATES_H
#define CNC_3018_OBSERVEBLE_STATES_H

#include <etl/observer.h>
#include <etl/message_types.h>

const etl::message_router_id_t JOB_FSM_NUMBER = 1;

enum class JobEvent {
    REFRESH,
    DONE
};

typedef etl::observer<JobEvent> JobObserver;

enum class DeviceEvent {
    REFRESH
};

using DeviceObserver = etl::observer<DeviceEvent>;


#endif //CNC_3018_OBSERVEBLE_STATES_H
