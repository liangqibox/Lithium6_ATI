'<TiCoBasic Header, Headerversion 001.001>
' Process_Number                 = 1
' Initial_Processdelay           = 1000
' Eventsource                    = Timer
' Priority                       = High
' Version                        = 1
' TiCoBasic_Version              = 1.5.0
' Optimize                       = Yes
' Optimize_Level                 = 2
' Info_Last_Save                 = FERMI-PC  fermi-PC\fermi
'<Header End>
#Include DIO32TiCo.inc 

Init:
  Set_LED(1)   
  DigProg(01111b)                                           ' Set channels 0.31 as outputs
 
  Processdelay = 50                                         '  1MHz
  PAR_1 = 0

Event:
  Digout_Long(PAR_1)
  
  
Finish:
  Set_LED(0)   
