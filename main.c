#include "std_types.h"
#include "system_tle987x.h"
#include "tle_device.h"
#include "eval_board.h"
#include "OS.h"

 
void main()
{
     
    SystemInit();
    TLE_Init();
    
    while(1)
    {
        (void)WDT1_Service();
    }
}
