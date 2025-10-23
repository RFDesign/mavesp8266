#include <stdint.h>
#include <mavlink_types.h>
#include <common/mavlink.h>

namespace sport::autopilotselection
{

typedef struct
{
    uint8_t SysID;
    uint8_t CompID;
} TMAVAddress;

void Ingest(mavlink_message_t * msg);
bool IsThisFromTheChosenAutoPilot(mavlink_message_t * msg);
void IncrementCount(void);
uint32_t GetCount(void);

uint8_t GetSysID(void);
uint8_t GetCompID(void);
bool GetChosenMAVAddr(TMAVAddress &Addr);

}