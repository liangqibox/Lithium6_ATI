'<ADbasic Header, Headerversion 001.001>
' Process_Number                 = 2
' Initial_Processdelay           = 1000000
' Eventsource                    = Timer
' Control_long_Delays_for_Stop   = No
' Priority                       = Low
' Priority_Low_Level             = 1
' Version                        = 1
' ADbasic_Version                = 6.0.0
' Optimize                       = Yes
' Optimize_Level                 = 1
' Stacksize                      = 1000
' Info_Last_Save                 = LITHIUM-MASTER  LITHIUM-MASTER\liang
'<Header End>
#include adwinpro_all.inc

dim data_101[12000000] as long
dim i as long
dim time as long

#define ModuleDIO1 1                                        'Digital Output
#define ModuleDIO2 2                                        'Digital Input
'#define ModuleDIO3 3                                        'Digital Input OR Output customer

'#define ModuleAout1 5
'#define ModuleAout2 6
'#define ModuleAout3 7
#define ModuleAout4 4
#define ModuleAout1 1
#define ModuleAout2 2
#define ModuleAout3 3

#define ModuleAin 4  
#define Sampling_rate Par_50
#define Mode par_30
#define Switch Par_51
#define Record_Start Par_52
'#define Record_End Par_53
#define Record_length Par_54

init:
  
  Sampling_rate = 1000 'us
#if processor = T11 then
  processdelay = 300000                                       
#else
  processdelay = 1000000                                      
#endif
  P2_Set_LED(ModuleAin, 1) 
  i = 0
  time = 0
  Switch = 0
 
event:
  if ((Switch>0) and (mode>0)) then
    inc(time)
    Par_53 = time
    P2_Start_ConvF(ModuleAin,0FFh)
    if (time > Record_Start) then
      P2_Wait_EOCF(ModuleAin,0FFh)
      P2_Read_ADCF8(ModuleAin,data_101,1+i*8)
      P2_Start_ConvF(ModuleAin,0FFh)
      processdelay = Sampling_rate*1000
      inc(i)
    endif
    
    'P2_Start_ConvF(ModuleAin,0FFh)
    'if (time > Record_Start) then
    'P2_Wait_EOCF(ModuleAin,0FFh)
    'P2_Read_ADCF8(ModuleAin,data_101,1+i*8)
    'P2_Start_ConvF(ModuleAin,0FFh)
    'processdelay = Sampling_rate*1000
    'inc(i)
    'endif
  endif
  
  if (i>Record_length) then 
    Switch = 0
    i = 0
    time = 0
    processdelay = 1000000
  endif
  
finish:
  P2_Set_LED(ModuleAin, 0) 

