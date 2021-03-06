'<ADbasic Header, Headerversion 001.001>
' Process_Number                 = 3
' Initial_Processdelay           = 1000000
' Eventsource                    = Timer
' Control_long_Delays_for_Stop   = No
' Priority                       = Low
' Priority_Low_Level             = 1
' Version                        = 1
' ADbasic_Version                = 6.0.0
' Optimize                       = Yes
' Optimize_Level                 = 3
' Stacksize                      = 1000000
' Info_Last_Save                 = LITHIUM-MASTER  LITHIUM-MASTER\liang
'<Header End>
#include adwinpro_all.inc

#define ModuleDIO1 1                                        'Digital Output
#define ModuleDIO2 2                                        'Digital Input
#define ModuleDIO3 3                                        'Digital Input OR Output customer

#define ModuleAout1 5
#define ModuleAout2 6
#define ModuleAout3 7

#define ModuleAin 4   
#define Mode par_30
#define i par_1

dim DigiIn as long
dim DigiCheck as long
dim AnalogIn as long

init:
#if processor = T11 then
  processdelay = 300000
#else
  processdelay = 1000000
#endif

  Par_40 = 0      'Analog input lock lower bound
  Par_39 = 65535  'Analog input lock upper bound
  Par_38 = 0      'Digital lock input
  Par_37 = -1     'Digital lock switch
  Par_2 = 0       'Analog lock report
  Par_3 = 0       'Digital lock report
 
event:  
  DigiIn = P2_Digin_Long(ModuleDIO1)
  AnalogIn = P2_ADCF(ModuleAin, 8)
  DigiCheck = Shift_Left(Par_38,24)
  
  if ((AnalogIn < par_40) or (AnalogIn > par_39)) then 'Analog input lock
    mode = 0
    Par_41 = 0
    i = 1
    Par_2 = 1
  endif
  
  if(par_37 > 0) then
    if((DigiCheck And DigiIn)<>0) then
      mode = 0
      Par_41 = 0
      i = 1
      Par_3 = 1
    Endif
    
    if(Not((DigiCheck+00000000111111111111111111111111b) Or DigiIn) <> 0) then
      mode = 0
      Par_41 = 0
      i = 1
      Par_3 = 1
    Endif
  Endif
  
  'Testing code
  Par_78 = AnalogIn
  Par_79 = P2_Digin_Long(ModuleDIO1)
  'Par_80 = Not((DigiCheck+00000000111111111111111111111111b) Or Par_78)
  
finish:
 
