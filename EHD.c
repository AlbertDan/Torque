#include "tle987x.h"
#include "std_types.h"
#include "intrinsics.h"
#include "vstdlib.h"
#include "EDH.h"
#include "OS.h"

/******************************************************************************************************************************/
/*                                              GENERAL_DEFINITIONS                                                           */
/******************************************************************************************************************************/

/* Direction of the spin clock-wise, counter clock-wise */
typedef enum
{
	
	NoDirection_en = 0u,
	CCW_en = 1u,
	CW_en = 2u
	
}Direction_t;

/* Order in which the signal interrupts appear NoOrder means the signal hasn't been assigned and order yet or did not appear */
typedef enum
{
	
	NoOrder_en,
	First_en,
	Second_en,
	Third_en,
	Last_en
	
}Order_t;

Order_t SignalP13Rise_en = NoOrder_en;

Order_t SignalP13Fall_en = NoOrder_en;

Order_t SignalP12Rise_en = NoOrder_en;

Order_t SignalP12Fall_en = NoOrder_en;

Direction_t Direction_en = NoDirection_en;

/******************************************************************************************************************************/
/*                                              GLOBAL_VARIABLES                                                              */
/******************************************************************************************************************************/

/* Start time of the measurement */
static u16 EDHMeasurementStartTime_u16 = 0u;

/* End time of the measurement */
static u16 EDHMeasurementEndTime_u16;

/* Number of rising fronts fitting the pattern rising(signal 1 || signal 2) rising(signal 1 || signal 2) falling(signal 1 || signal 2) falling(signal 1 || signal 2) */
static u16 EDHCounterFronts_u16 = 0u;

static u16 EDHRotationSpeed_u16 = 0u;

static u16 EDHTotalRotationSpeed_u16 = 0u;

/* Speed ring buffer */
static u16 EDHSpeedBuffer_u16[64u];

static u16 EDHSpeedBufferIndex_u16 = 0u;

/* A counter that determines how many speed instances were calculated */
static u8 EDHCounterTotalSpeedLoopsCalculated_u8 = 0u;

/******************************************************************************************************************************/
/*                                              END OF DEFINITIONS                                                            */
/******************************************************************************************************************************/

/* Interrupt function for signal 1 falling edge, it checks the orders of the signals rising and falling edges to match the signal patterns for clock-wise and counter clock-wise spinning motions for example clock-wise signal order is: signa1 1 rising ; signal 2 rising ; signal 1 falling ; signal 2 falling ; for counter clock-wise the order is: signal 2 rising ; signal 1 rising ; signal 2 falling ; signal 1 falling ; */
void EDH_fallingEdgeSig1(void)
{
	
	/* Checking the orders to determine if this signal interrupt is the last in the order or third check the comment for EDH_fallingEdgeSig1 function to see the order */
	if(  First_en == SignalP12Rise_en   &&   Second_en == SignalP13Rise_en   &&   Third_en == SignalP12Fall_en  )
	{
		
		/* saving end_time here to calculate the speed because we know now the case we are in (CCW_en) */
		EDHMeasurementEndTime_u16 = OS_getTime();
		
		SignalP13Fall_en = Last_en; 
		
	}
	else
	if(  First_en == SignalP13Rise_en   &&   Second_en == SignalP12Rise_en   &&   NoOrder_en == SignalP12Fall_en  )
	{
		
		SignalP13Fall_en = Third_en;
		
	}
	/* Here we reset the order of the signals, speed and start_time counter because this case is a error, it does not correspond to the order checked in the previous 2 if statements */
	else
	{
		
		SignalP13Rise_en = NoOrder_en;
		SignalP12Rise_en = NoOrder_en;
		SignalP13Fall_en = NoOrder_en;
		SignalP12Fall_en = NoOrder_en;
                
		EDHRotationSpeed_u16 = 0u;
		EDHMeasurementStartTime_u16 = 0u;
		
	}
	
	/* Here we verify if we are in the CCW_en case, this condition being true means that we have all the conditions set for the CCW_en case */
	if(  Last_en == SignalP13Fall_en  )
	{
		
		/* This statement checks to see if we have a change in direction, if the previous case was CW_en it means that we have a change in direction entering the CCW_en case */
		if( CW_en == Direction_en )
		{
			
			EDHCounterFronts_u16 = 0u;
			EDHRotationSpeed_u16 = 0u;
			EDHMeasurementStartTime_u16 = 0u;
			
		}
		else
		{
			
			/* do nothing */
			
		}
		
		EDHCounterFronts_u16++;
		
		/* If counter_fronts counted to 250 rising fronts (125 fronts = 5 degrees) we calculate the speed */
		if( 0u == ( EDHCounterFronts_u16 % 250u ) ) 
		{
			
			/* For the first 64 calculations of the speed we use this calculation method */
			if( 64u > EDHCounterTotalSpeedLoopsCalculated_u8 )
			{
				
                              EDHRotationSpeed_u16 = 10000u / ( EDHMeasurementEndTime_u16 - EDHMeasurementStartTime_u16 );	
                              EDHCounterTotalSpeedLoopsCalculated_u8++;
				
			}
			else
			{
			
				/* do nothing */
				
			}
			
			/* While the speed buffer is null we add the values we put into it to the total rotation speed aswell, when the speed buffer is full we erase the previous value it had on position EDHSpeedBufferIndex_u16 from the total sum(EDHTotalRotationSpeed_u16) and add the new value to the total sum */
			if( 0u != EDHSpeedBuffer_u16[EDHSpeedBufferIndex_u16] )
			{
				
                              EDHTotalRotationSpeed_u16 = EDHTotalRotationSpeed_u16 - EDHSpeedBuffer_u16[EDHSpeedBufferIndex_u16];
                              EDHSpeedBuffer_u16[EDHSpeedBufferIndex_u16] = 10000u / ( EDHMeasurementEndTime_u16 - EDHMeasurementStartTime_u16 );
                              EDHTotalRotationSpeed_u16 = EDHTotalRotationSpeed_u16 + EDHSpeedBuffer_u16[EDHSpeedBufferIndex_u16];
				
			}
			else
			{
				
				EDHSpeedBuffer_u16[EDHSpeedBufferIndex_u16] = 10000u / ( EDHMeasurementEndTime_u16 - EDHMeasurementStartTime_u16 );
				EDHTotalRotationSpeed_u16 = EDHTotalRotationSpeed_u16 + EDHSpeedBuffer_u16[EDHSpeedBufferIndex_u16];
				
			}
			
			/* Index reset and increment if statement */
			if( 63u == EDHSpeedBufferIndex_u16 )
			{
				
				EDHSpeedBufferIndex_u16 = 0u;
				
			}
			else
			{
				
                                EDHSpeedBufferIndex_u16++;
			
			}
			
			/* After 64 loops of calculating the speed with the previous formula we change it to this new calculation */
			if( 64u == EDHCounterTotalSpeedLoopsCalculated_u8 )
			{
				
				EDHRotationSpeed_u16 = EDHTotalRotationSpeed_u16 / 64u;
				
			}
			else
			{
			
			  /* do nothing */
				
			}
			
			/* Measurement start time reset so the next rising edge interrupt can save the time for the next measurement */
			EDHMeasurementStartTime_u16 = 0u;
			
                }
		
		/* This is the CCW_en case so we set the direction to CCW_en */
		Direction_en = CCW_en;
		
		/* Reset of the signal orders */
		SignalP13Rise_en = NoOrder_en;
		SignalP12Rise_en = NoOrder_en;
		SignalP13Fall_en = NoOrder_en;
		SignalP12Fall_en = NoOrder_en;
		
		/* After 9000 rising fronts succesfully identified we reset the variable because the device did a 360 degree turn */
		if( 9000u == EDHCounterFronts_u16 )
		{
			
			EDHCounterFronts_u16 = 0u;
			
		}
		else
		{
			
			/* do nothing */
			
		}
		
	 }
	 else
	 {
		
	  	/* do nothing */
		
	 }
	
}

/* Interrupt function for signal 1 rising edge, it checks the orders of the signals rising and falling edges to match the signal patterns for clock-wise and counter clock-wise spinning motions for example clock-wise signal order is: signa1 1 rising ; signal 2 rising ; signal 1 falling ; signal 2 falling ; for counter clock-wise the order is: signal 2 rising ; signal 1 rising ; signal 2 falling ; signal 1 falling ; */
void EDH_risingEdgeSig1(void) 
{	
	
	/* If signal P1_2 rising doesn't have its order set it means it is after signal P1_3 and signal P1_3 is the first in the order */
	if(  NoOrder_en == SignalP12Rise_en  )
        {

		/* If the measurement didn't start yet, we save the time of the start for the measurement */
		if( 0u == EDHMeasurementStartTime_u16 )
		{
			
			EDHMeasurementStartTime_u16 = OS_getTime();
			
		}
		else
		{
			
			/* do nothing */
			
		}
		
		
		SignalP13Rise_en = First_en;	
		
        }
	else
	/* If signal P1_2 has its order set it means that signal P1_3 is the second in the order */
	if(  First_en == SignalP12Rise_en  )
	{
			
		SignalP13Rise_en = Second_en;
			
	}
	else
	{
		
		/* do nothing */
			
	}
		
	/* If statement that checks if the order of the signals has been disrupted in some way, if so we have an error case and reset the orders of the signals and start the speed calculation and measurement over again */
	if( ( First_en == SignalP12Rise_en  &&  First_en == SignalP13Rise_en ) || ( Second_en == SignalP12Rise_en  &&  Second_en == SignalP13Rise_en ) || ( Third_en == SignalP12Fall_en && Third_en == SignalP13Fall_en ) )
        {
		
                  SignalP13Rise_en = NoOrder_en;
                  SignalP12Rise_en = NoOrder_en;
                  SignalP13Fall_en = NoOrder_en;
                  SignalP12Fall_en = NoOrder_en;
                
                  EDHRotationSpeed_u16 = 0u;
                  EDHMeasurementStartTime_u16 = 0u;
		
        }
	else
	{
		
		/* do nothing */
		
	}
	
}

/* Interrupt function for signal 2 falling edge, it checks the orders of the signals rising and falling edges to match the signal patterns for clock-wise and counter clock-wise spinning motions for example clock-wise signal order is: signa1 1 rising ; signal 2 rising ; signal 1 falling ; signal 2 falling ; for counter clock-wise the order is: signal 2 rising ; signal 1 rising ; signal 2 falling ; signal 1 falling ; */
void EDH_fallingEdgeSig2(void) 
{
	
	/* Checking the orders to determine if this signal interrupt is the last in the order or third check the comment for EDH_fallingEdgeSig2 function to see the order */
	if(  First_en == SignalP13Rise_en   &&   Second_en == SignalP12Rise_en  &&  Third_en == SignalP13Fall_en  )
	{
		
		/* saving end_time here to calculate the speed because we know now the case we are in (CW_en) */
		EDHMeasurementEndTime_u16 = OS_getTime();
		
		SignalP12Fall_en = Last_en; 
		
	}
	else
	if(  First_en == SignalP12Rise_en   &&   Second_en == SignalP13Rise_en  &&  NoOrder_en == SignalP13Fall_en  )
	{
		
		SignalP12Fall_en = Third_en;
		
	}
	/* Here we reset the order of the signals, speed and start_time counter because this case is a error, it does not correspond to the order checked in the previous 2 if statements */
	else
	{
		
		SignalP13Rise_en = NoOrder_en;
		SignalP12Rise_en = NoOrder_en;
		SignalP13Fall_en = NoOrder_en;
		SignalP12Fall_en = NoOrder_en;
                
		EDHRotationSpeed_u16 = 0u;
		EDHMeasurementStartTime_u16 = 0u;
		
	}
	
	/* Here we verify if we are in the CW_en case, this condition being true means that we have all the conditions set for the CW_en case */
	if(  Last_en == SignalP12Fall_en  )
	{
		
		/* This statement checks to see if we have a change in direction, if the previous case was CCW_en it means that we have a change in direction entering the CW_en case */
		if( CCW_en == Direction_en )
		{
			
			EDHCounterFronts_u16 = 0u;
			EDHRotationSpeed_u16 = 0u;
			EDHMeasurementStartTime_u16 = 0u;
			
		}
		else
		{
			
			/* do nothing */
			
		}
		
		EDHCounterFronts_u16++;
		
		/* If counter_fronts counted to 250 rising fronts (125 fronts = 5 degrees) we calculate the speed */
		if( 0u == ( EDHCounterFronts_u16 % 250u ) ) 
		{
			
			/* For the first 64 calculations of the speed we use this calculation method */
			if( 64u > EDHCounterTotalSpeedLoopsCalculated_u8 )
			{
				
				EDHRotationSpeed_u16 = 10000u / ( EDHMeasurementEndTime_u16 - EDHMeasurementStartTime_u16 );	
				EDHCounterTotalSpeedLoopsCalculated_u8++;
				
			}
			else
			{
				
				/* do nothing */
				
			}
			
			/* While the speed buffer is null we add the values we put into it to the total rotation speed aswell, when the speed buffer is full we erase the previous value it had on position EDHSpeedBufferIndex_u16 from the total sum(EDHTotalRotationSpeed_u16) and add the new value to the total sum */
			if( 0u != EDHSpeedBuffer_u16[EDHSpeedBufferIndex_u16] )
			{
				
				EDHTotalRotationSpeed_u16 = EDHTotalRotationSpeed_u16 - EDHSpeedBuffer_u16[EDHSpeedBufferIndex_u16];
				EDHSpeedBuffer_u16[EDHSpeedBufferIndex_u16] = 10000u / ( EDHMeasurementEndTime_u16 - EDHMeasurementStartTime_u16 );
				EDHTotalRotationSpeed_u16 = EDHTotalRotationSpeed_u16 + EDHSpeedBuffer_u16[EDHSpeedBufferIndex_u16];
				
			}
			else
			{
				
				EDHSpeedBuffer_u16[EDHSpeedBufferIndex_u16] = 10000u / ( EDHMeasurementEndTime_u16 - EDHMeasurementStartTime_u16 );
				EDHTotalRotationSpeed_u16 = EDHTotalRotationSpeed_u16 + EDHSpeedBuffer_u16[EDHSpeedBufferIndex_u16];
				
			}
			
			/* Index reset and increment if statement */
			if( 63u == EDHSpeedBufferIndex_u16 )
			{
				
				EDHSpeedBufferIndex_u16 = 0u;
				
			}
			else
			{
				
				EDHSpeedBufferIndex_u16++;
			
			}
				
			/* After 64 loops of calculating the speed with the previous formula we change it to this new calculation */
			if( 64u == EDHCounterTotalSpeedLoopsCalculated_u8 )
			{
				
				EDHRotationSpeed_u16 = EDHTotalRotationSpeed_u16 / 64u;
				
			}
			else
			{
				
				/* do nothing */
				
			}
			
			/* Measurement start time reset so the next rising edge interrupt can save the time for the next measurement */
			EDHMeasurementStartTime_u16 = 0u;
			
                }
		else
		{
			
			/* do nothing */
			
		}
		
		/* This is the CW_en case so we set the direction to CW_en */
		Direction_en = CW_en;
		
                /* Reset of the signal orders */
		SignalP13Rise_en = NoOrder_en;
		SignalP12Rise_en = NoOrder_en;
		SignalP13Fall_en = NoOrder_en;
		SignalP12Fall_en = NoOrder_en;
		
		/* After 9000 rising fronts succesfully identified we reset the variable because the device did a 360 degree turn */
		if( 9000u == EDHCounterFronts_u16 )
		{
			
			EDHCounterFronts_u16 = 0u;
			
		}
		else
		{
			
			/* do nothing */
			
		}
		
	 }
	 else
	 {
		 
		 /* do nothing */
		 
	 }
	
}

/* Interrupt function for signal 2 rising edge, it checks the orders of the signals rising and falling edges to match the signal patterns for clock-wise and counter clock-wise spinning motions for example clock-wise signal order is: signa1 1 rising ; signal 2 rising ; signal 1 falling ; signal 2 falling ; for counter clock-wise the order is: signal 2 rising ; signal 1 rising ; signal 2 falling ; signal 1 falling ; */
void EDH_risingEdgeSig2(void) 
{
	
	/* If signal P1_3 rising doesn't have its order set it means it is after signal P1_2 and signal P1_2 is the first in the order */
	if(  NoOrder_en == SignalP13Rise_en  )
	{
		
		/* If the measurement didn't start yet, we save the time of the start for the measurement */
		if( 0u == EDHMeasurementStartTime_u16 )
		{
			
			EDHMeasurementStartTime_u16 = OS_getTime();
			
		}
		else
		{
			
			/* do nothing */
			
		}
		
		SignalP12Rise_en = First_en;
		
	}
	else
	/* If signal P1_3 has its order set it means that signal P1_2 is the second in the order */
	if(  First_en == SignalP13Rise_en  )
	{
		
		SignalP12Rise_en = Second_en;
		
	}
	
	/* If statement that checks if the order of the signals has been disrupted in some way, if so we have an error case and reset the orders of the signals and start the speed calculation and measurement over again */
	if( ( First_en == SignalP12Rise_en  &&  First_en == SignalP13Rise_en ) ||  ( Second_en == SignalP12Rise_en  &&  Second_en == SignalP13Rise_en ) || ( Third_en == SignalP12Fall_en && Third_en == SignalP13Fall_en ) )
	{
		
		SignalP13Rise_en = NoOrder_en;
		SignalP12Rise_en = NoOrder_en;
		SignalP13Fall_en = NoOrder_en;
		SignalP12Fall_en = NoOrder_en;
                
		EDHRotationSpeed_u16 = 0u;
		EDHMeasurementStartTime_u16 = 0u;
		
	}
	else
	{
			
		/* do nothing */
			
	}
	
}

u16 EDH_getSpeed(void)
{
  
  return EDHRotationSpeed_u16;
  
}

u16 EDH_getSpeedTime(void)
{
  
  return EDHMeasurementEndTime_u16 - EDHMeasurementStartTime_u16;
  
}

void EDH_resetSpeed(void)
{
  
    /* If statement that checks to see if 1 second passed after calculating the speed it then resets the speed to 0 */
    if( ( OS_getTime() > EDHMeasurementEndTime_u16 + 1000u ) || ( OS_getTime() > EDHMeasurementStartTime_u16 + 1000u ) )
    {
      
       EDHRotationSpeed_u16=0u;
      
    }
 
}
