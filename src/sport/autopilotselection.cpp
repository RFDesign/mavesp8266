#include "autopilotselection.h"
#include "mavesp8266.h"
#include "mavesp8266_parameters.h"
#include "txmod_debug.h"

namespace sport::autopilotselection
{

static bool gRxSysID = false;
static TMAVAddress gLowestMAVAddr = {0xFF, 0xFF};
static uint32_t gSPORTRxPktCount = 0;

void Ingest(mavlink_message_t * msg)
{
    if (msg->msgid == MAVLINK_MSG_ID_HEARTBEAT)
    {
      debug_println("Got HB msg"); 

      if (mavlink_msg_heartbeat_get_autopilot(msg) != MAV_AUTOPILOT_INVALID)
      {
        debug_println("AP type is not invalid"); 

        if (msg->sysid < gLowestMAVAddr.SysID ||
            (msg->sysid == gLowestMAVAddr.SysID && msg->compid < gLowestMAVAddr.CompID))
        {
            debug_println("Adopting as autopilot " + String(msg->sysid) + " " + String(msg->compid)); 
            gLowestMAVAddr.SysID = msg->sysid;
            gLowestMAVAddr.CompID = msg->compid;
        }
        gRxSysID = true;
      }
    }
}

bool GetChosenMAVAddr(TMAVAddress &Addr)
{
    MavESP8266Parameters *pParams = getWorld()->getParameters();
    
    if (pParams->getSPORTRxAddressMode() == MavESP8266Parameters::TSPortRxAddressMode::MANUAL)
    {
        Addr.SysID = pParams->getSPORTRxSysID();
        Addr.CompID = pParams->getSPORTRxCompID();
        return true;
    }
    else
    {
        if (gRxSysID)
        {
            Addr = gLowestMAVAddr;
            return true;    
        }
        else
        {
            return false;
        }
    }
}

bool IsThisFromTheChosenAutoPilot(mavlink_message_t * msg)
{
    TMAVAddress Addr;

    if (GetChosenMAVAddr(Addr))
    {
        bool Result = Addr.SysID == msg->sysid && Addr.CompID == msg->compid;

        if (!Result)
        {
            debug_println("Message not from AP - " + String(msg->sysid) + " " + String(msg->compid));
            debug_println("\tMessage type - " + String(msg->msgid));
            debug_println("\tAP is " + String(Addr.SysID) + " " + String(Addr.CompID));
        }
        
        return Result;
    }
    else
    {
        debug_println("Don't have AP address");
        return false;
    }
}

void IncrementCount(void)
{
    gSPORTRxPktCount++;
}

uint32_t GetCount(void)
{
    return gSPORTRxPktCount;
}

}