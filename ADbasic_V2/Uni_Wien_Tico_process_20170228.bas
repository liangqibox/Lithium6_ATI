'<TiCoBasic Header, Headerversion 001.001>
' Process_Number                 = 3
' Initial_Processdelay           = 1000
' Eventsource                    = Timer
' Priority                       = High
' Version                        = 1
' TiCoBasic_Version              = 1.5.0
' Optimize                       = Yes
' Optimize_Level                 = 2
' Info_Last_Save                 = LITHIUM-MASTER  LITHIUM-MASTER\liang
'<Header End>
#Include DIO32TiCo.inc 

Init:
  Set_LED(1)   
  DigProg(0111b)                                           ' Set channels 0.31 as outputs
 
  Processdelay = 50                                         '  1MHz
  PAR_1 = 0
  PAR_2 = 0

Event:
  Digout_Long(PAR_1)
  PAR_2 = Digin_Long()
  
Finish:
  Set_LED(0)   
