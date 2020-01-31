'<ADbasic Header, Headerversion 001.001>
' Process_Number                 = 5
' Initial_Processdelay           = 1000
' Eventsource                    = External
' Control_long_Delays_for_Stop   = No
' Priority                       = High
' Version                        = 1
' ADbasic_Version                = 6.0.0
' Optimize                       = Yes
' Optimize_Level                 = 1
' Stacksize                      = 1000
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

dim SaftyInput as Long

init:
  CPU_Dig_IO_Config(000011b)
  CPU_Event_Config(1,2,1)
 
event:
  mode = 0
  Par_41 = 0
  Par_1 = 1
  Par_3 = 1
  
Finish:
  
