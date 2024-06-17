#include "tle_device.h"
#include "eval_board.h"
#include "std_types.h"
#include "OS.h"
#include "vstdlib.h"
#include "EDH.h"


/******************************************************************************************************************************/
/*                                              GLOBAL_VARIABLES                                                              */
/******************************************************************************************************************************/


///* Counter for 5MS interrupts. */
//static u16 Os_Counter5MS_u16;                                                                
///* Counter for 10MS interrupts. */
//static u16 Os_Counter10MS_u16;                                                               
///* Counter for 40MS interrupts. */
//static u16 Os_Counter40MS_u16;             


/* Counter for Systick timer interrupts. */
static u16 Os_Counter_u16 = 0u;  

static u16 Os_CounterTime_u16 = 0u;



/******************************************************************************************************************************/
/*                                              FUNCTION PROTOTYPES                                                           */
/******************************************************************************************************************************/


/* Function for 5 milliseconds time slice. */
void Os_TimeSlice5MS(void);                                                           
/* Function for 10 milliseconds time slice. */
void Os_TimeSlice10MS(void);                                                        
/* Function for 40 milliseconds time slice. */
void Os_TimeSlice40MS(void);


/******************************************************************************************************************************/
/*                                              END OF DEFINITIONS                                                            */
/******************************************************************************************************************************/

 

void Os_SysTick_Timer(void)                                                            
{   
    /* Timeslice time counter */
    Os_Counter_u16++; 
    /* System timer */
    Os_CounterTime_u16++; 
    
    /* If condition for timeslice 5 milliseconds */
    if( 1u == ( Os_Counter_u16 % 5 ) )
    {
      
      Os_TimeSlice5MS();
      
    }
    /* If condition for timeslice 10 milliseconds */
    else if( 2u == ( Os_Counter_u16 % 10 ) )
    {
      
      Os_TimeSlice10MS();
      
    }
    /* If condition for timeslice 40 milliseconds */
    else if( 3u == ( Os_Counter_u16 % 40 ) )
    {
      
      Os_TimeSlice40MS();
      
    }
    /* If reset condition for time counter */
    else if( 40u == Os_Counter_u16)
    {
      
      Os_Counter_u16 = 0u;
      
    }
    else
    {
      
      /* do nothing */
      
    }
    
}

void Os_TimeSlice5MS(void)                                                     
{
  
                                       
    
}

void Os_TimeSlice10MS(void)
{ 
    
    /* Function for reseting speed after 10 milliseconds passed */
    EDH_resetSpeed();
    
}

void  Os_TimeSlice40MS(void)
{ 
    

    
}

u16 OS_getTime(void)
{
  
    return   Os_CounterTime_u16;
    
}
