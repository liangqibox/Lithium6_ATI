'<ADbasic Header, Headerversion 001.001>
' Process_Number                 = 1
' Initial_Processdelay           = 10000
' Eventsource                    = Timer
' Control_long_Delays_for_Stop   = No
' Priority                       = High
' Version                        = 1
' ADbasic_Version                = 6.0.0
' Optimize                       = Yes
' Optimize_Level                 = 2
' Stacksize                      = 1000
' Info_Last_Save                 = LITHIUM-MASTER  LITHIUM-MASTER\liang
'<Header End>
#include adwinpro_all.inc

'update 2017-04-26
dim i, index, count, a as long
'Each data -> channel  = ch1 = data_1 ... ch24 = data_24
dim data_1[800000] as long
dim data_2[800000] as long
dim data_3[800000] as long
dim data_4[800000] as long
dim data_5[800000] as long
dim data_6[800000] as long
dim data_7[800000] as long
dim data_8[800000] as long

dim data_9[800000] as long
dim data_10[800000] as long
dim data_11[800000] as long
dim data_12[800000] as long
dim data_13[800000] as long
dim data_14[800000] as long
dim data_15[800000] as long
dim data_16[800000] as long

dim data_17[800000] as long
dim data_18[800000] as long
dim data_19[800000] as long
dim data_20[800000] as long
dim data_21[800000] as long
dim data_22[800000] as long
dim data_23[800000] as long
dim data_24[800000] as long

dim data_25[800000] as long
dim data_26[800000] as long
dim data_27[800000] as long
dim data_28[800000] as long
dim data_29[800000] as long
dim data_30[800000] as long
dim data_31[800000] as long
dim data_32[800000] as long

dim data_41[800000] as long                                'DIO1
dim data_42[800000] as long                                'DIO2

dim data_51[800000] as long                                'Processdelay

dim data_50[35] as long                                     'Start/initialize values

#define ModuleDIO1 1                                        'Digital Output
#define ModuleDIO2 2                                        'Digital Input

#define ModuleAout1 1
#define ModuleAout2 2
#define ModuleAout3 3
#define ModuleAout4 4

#define ModuleAin 1                                         'Please change

#define Mode par_30

lowinit:
  for count = 1 to 800000
    data_1[count] = 32768
    data_2[count] = 32768
    data_3[count] = 32768
    data_4[count] = 32768
    data_5[count] = 32768
    data_6[count] = 32768
    data_7[count] = 32768
    data_8[count] = 32768
    data_9[count] = 32768
    data_10[count] = 32768
    data_11[count] = 32768
    data_12[count] = 32768
    data_13[count] = 32768
    data_14[count] = 32768
    data_15[count] = 32768
    data_16[count] = 32768
    data_17[count] = 32768
    data_18[count] = 32768
    data_19[count] = 32768
    data_20[count] = 32768
    data_21[count] = 32768
    data_22[count] = 32768
    data_23[count] = 32768
    data_24[count] = 32768
    data_25[count] = 32768
    data_26[count] = 32768
    data_27[count] = 32768
    data_28[count] = 32768
    data_29[count] = 32768
    data_30[count] = 32768
    data_31[count] = 32768
    data_32[count] = 32768
    data_41[count] = 0                             'DIO1
    data_42[count] = 0                             'DIO2
    data_51[count] = 80000                                'Processdelay
  next count
  
  for index = 1 to 32
    'set analog output voltage on all 32 channels to 0V
    data_50[index] = 32768 
  next index
  'set digital output on all digital channels to 0V
  data_50[33] = 0 
  data_50[34] = 0

  ' set event Timing
  processdelay = 80000


init:
  SetLED(ModuleAout1,da,1) 
  SetLED(ModuleAout2,da,1)
  SetLED(ModuleAout3,da,1)
  SetLED(ModuleAout4,da,1)
  'P2_Set_LED(ModuleDIO1, 1)   ' commented out, because done in TiCo
  
  'Enable Synchmode for all channel 1..8
  SyncEnable(ModuleAout1,da,11111111b) 
  SyncEnable(ModuleAout2,da,11111111b)
  SyncEnable(ModuleAout3,da,11111111b)
  SyncEnable(ModuleAout4,da,11111111b)
'''''P2_Sync_Enable(ModuleDIO1, 01)                            ' 

  'P2_digprog(ModuleDIO1, 1111b)                             ' DIO all output   commented out, because done in TiCo
  'P2_digprog(ModuleDIO2, 0000b)                             ' DIO all input
  'P2_digprog(ModuleDIO3, 0000b)                             ' DIO all input
  
  syncall()                                     'Start output 3 analog out
  Digout_long(ModuleDIO1, data_50[33])                   'set DIO start values
  Digout_long(ModuleDIO2, data_50[34]) 
  '++++++++++++++++++++++++++++++++++
  'Setup the output values for channel 1 and 2 on all modules
  For a = 1 to 8
    WriteDac(ModuleAout1,a,data_50[a])
    WriteDac(ModuleAout2,a,data_50[a+8])
    WriteDac(ModuleAout3,a,data_50[a+16])
    WriteDac(ModuleAout4,a,data_50[a+24])
  Next a
  Start_Dac(ModuleAout1)
  Start_Dac(ModuleAout2)
  Start_Dac(ModuleAout3)
  Start_Dac(ModuleAout4)
  
  Par_41 = 0      'Computer controled switch
  Par_28 = 5000000 'End of Sequences
  i = 1 
  Mode = 0
 
event:
  syncall()
        
  if(i>Par_28) then       'Waitting state
    mode = 0
    Par_41 = 0
    i = 1
    Processdelay = 80000
  endif 
  
  if (mode>0) then
    'p2_set_par(ModuleDIO1, 1, 1, data_41[i])        'Set Par_1 of DIO1
    
    WriteDac(ModuleAout1,1,data_1[i])
    WriteDac(ModuleAout1,2,data_2[i])
    WriteDac(ModuleAout1,3,data_3[i])
    WriteDac(ModuleAout1,4,data_4[i])
    WriteDac(ModuleAout1,5,data_5[i])
    WriteDac(ModuleAout1,6,data_6[i])
    WriteDac(ModuleAout1,7,data_7[i])
    WriteDac(ModuleAout1,8,data_8[i])
    
    WriteDac(ModuleAout2,1,data_9[i])
    WriteDac(ModuleAout2,2,data_10[i])
    WriteDac(ModuleAout2,3,data_11[i])
    WriteDac(ModuleAout2,4,data_12[i])
    WriteDac(ModuleAout2,5,data_13[i])
    WriteDac(ModuleAout2,6,data_14[i])
    WriteDac(ModuleAout2,7,data_15[i])
    WriteDac(ModuleAout2,8,data_16[i])
    
    WriteDac(ModuleAout3,1,data_17[i])
    WriteDac(ModuleAout3,2,data_18[i])
    WriteDac(ModuleAout3,3,data_19[i])
    WriteDac(ModuleAout3,4,data_20[i])
    WriteDac(ModuleAout3,5,data_21[i])
    WriteDac(ModuleAout3,6,data_22[i])
    WriteDac(ModuleAout3,7,data_23[i])
    WriteDac(ModuleAout3,8,data_24[i])
    
    WriteDac(ModuleAout4,1,data_25[i])
    WriteDac(ModuleAout4,2,data_26[i])
    WriteDac(ModuleAout4,3,data_27[i])
    WriteDac(ModuleAout4,4,data_28[i])
    WriteDac(ModuleAout4,5,data_29[i])
    WriteDac(ModuleAout4,6,data_30[i])
    WriteDac(ModuleAout4,7,data_31[i])
    WriteDac(ModuleAout4,8,data_32[i])
    
    Start_Dac(ModuleAout1)
    Start_Dac(ModuleAout2)
    Start_Dac(ModuleAout3)
    Start_Dac(ModuleAout4)
    
    Digout_Long_F(ModuleDio1,data_41[i])
    Digout_Long_F(ModuleDio2,data_42[i])
      
    Processdelay = data_51[i]
    inc (i)                                                 'Next time output values
    par_1 = i
  endif
    
  if ((mode = 0) and (Par_41 = 1)) then  'Computer Controled switch
    inc(mode)
  endif
  
finish:
  'Initialize DAC and DIO output values to be set with stop process
  'Set outputs to a value which will be outputed until next start
  '*****************************************************************
  '********************************************
  For a = 1 to 8
    WriteDac(ModuleAout1,a,data_50[a])
    WriteDac(ModuleAout2,a,data_50[a+8])
    WriteDac(ModuleAout3,a,data_50[a+16])
    WriteDac(ModuleAout4,a,data_50[a+24])
  Next a
  syncall()                                     'Start output 3 analog out
  
  'p2_set_par(ModuleDIO1, 1, 1, 0)                   'set DIO start values
  
  SetLED(ModuleAout1,da,0) 
  SetLED(ModuleAout2,da,0)
  SetLED(ModuleAout3,da,0)
  SetLED(ModuleAout4,da,0)

