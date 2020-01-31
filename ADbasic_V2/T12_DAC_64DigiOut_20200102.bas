'<ADbasic Header, Headerversion 001.001>
' Process_Number                 = 1
' Initial_Processdelay           = 10000
' Eventsource                    = Timer
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

dim i, index as long
'Each data -> channel  = ch1 = data_1 ... ch24 = data_24
dim data_1[5000000] as long
dim data_2[5000000] as long
dim data_3[5000000] as long
dim data_4[5000000] as long
dim data_5[5000000] as long
dim data_6[5000000] as long
dim data_7[5000000] as long
dim data_8[5000000] as long

dim data_9[5000000] as long
dim data_10[5000000] as long
dim data_11[5000000] as long
dim data_12[5000000] as long
dim data_13[5000000] as long
dim data_14[5000000] as long
dim data_15[5000000] as long
dim data_16[5000000] as long

dim data_17[5000000] as long
dim data_18[5000000] as long
dim data_19[5000000] as long
dim data_20[5000000] as long
dim data_21[5000000] as long
dim data_22[5000000] as long
dim data_23[5000000] as long
dim data_24[5000000] as long

dim data_41[5000000] as long                                'DIO1
dim data_42[5000000] as long                                'DIO2

dim data_51[5000000] as long                                'Processdelay

dim data_50[30] as long                                     'Start/initialize values

#define ModuleDIO1 1                                        'Digital Output
#define ModuleDIO2 2                                        'Digital Output
#define ModuleDIO3 3                                        'Digital Input OR Output customer

#define ModuleAout1 5
#define ModuleAout2 6
#define ModuleAout3 7

#define ModuleAin 4                                         'Please change

#define Mode par_30

lowinit:
  '  for i = 1 to 1000000
  '    data_41[i] = 0155h
  '    
  '  next
  '  
  for index = 1 to 24
    'set analog output voltage on all 24 channels to 0V
    data_50[index] = 32768 
  next
  'set digital output on all digital channels to 0V
  data_50[25] = 0 

  ' set event Timing
#if processor = T11 then
  processdelay = 15000                                       'T11   '100kHz = 10µs
#else
  processdelay = 50000                                       'T12   '100kHz = 10µs
#endif


init:
  P2_Set_LED(ModuleAout1, 1) 
  P2_Set_LED(ModuleAout2, 1)
  P2_Set_LED(ModuleAout3, 1)
  'P2_Set_LED(ModuleDIO1, 1)   ' commented out, because done in TiCo
  
  'Enable Synchmode for all channel 1..8
  P2_Sync_Enable(ModuleAout1, 11111111b) 
  P2_Sync_Enable(ModuleAout2, 11111111b)
  P2_Sync_Enable(ModuleAout3, 11111111b)
'''''P2_Sync_Enable(ModuleDIO1, 01)                            ' 

  'P2_digprog(ModuleDIO1, 1111b)                             ' DIO all output   commented out, because done in TiCo
  'P2_digprog(ModuleDIO2, 0000b)                             ' DIO all input
  'P2_digprog(ModuleDIO3, 0000b)                             ' DIO all input
  
  p2_sync_all(1110000b)                                     'Start output 3 analog out
  p2_digout_long(ModuleDIO1, data_50[25])                   'set DIO start values
  '++++++++++++++++++++++++++++++++++
  'Setup the output values for channel 1 and 2 on all modules
  p2_write_dac32(ModuleAout1, 0, (data_50[1]) or (shift_left(data_50[2], 16)))
  p2_write_dac32(ModuleAout2, 0, (data_50[9]) or (shift_left(data_50[10], 16)))
  p2_write_dac32(ModuleAout3, 0, (data_50[17]) or (shift_left(data_50[18], 16)))   
  'Setup the output values for channel 3 and 4 on all modules
  p2_write_dac32(ModuleAout1, 1, (data_50[3]) or (shift_left(data_50[4], 16)))
  p2_write_dac32(ModuleAout2, 1, (data_50[11]) or (shift_left(data_50[12], 16)))
  p2_write_dac32(ModuleAout3, 1, (data_50[19]) or (shift_left(data_50[20], 16)))   
  'Setup the output values for channel 5 and 6 on all modules
  p2_write_dac32(ModuleAout1, 2, (data_50[5]) or (shift_left(data_50[6], 16)))
  p2_write_dac32(ModuleAout2, 2, (data_50[13]) or (shift_left(data_50[14], 16)))
  p2_write_dac32(ModuleAout3, 2, (data_50[21]) or (shift_left(data_50[22], 16)))
  'Setup the output values for channel 7 and 8 on all modules
  p2_write_dac32(ModuleAout1, 3, (data_50[7]) or (shift_left(data_50[8], 16)))
  p2_write_dac32(ModuleAout2, 3, (data_50[15]) or (shift_left(data_50[16], 16)))
  p2_write_dac32(ModuleAout3, 3, (data_50[23]) or (shift_left(data_50[24], 16)))  
  
  Par_41 = 0      'Computer controled switch
  Par_28 = 5000000 'End of Sequences
  i = 1 
  Mode = 0
 
event:
        
  if(i>Par_28) then       'Waitting state
    mode = 0
    Par_41 = 0
    i = 1
    Processdelay = 500000
  endif 
  
  if (mode>0) then
    'Preload DAC registers for first output sequenz inside "event:"
    'Setup the output values for channel 1 and 2 on all modules
    p2_write_dac32(ModuleAout1, 0, (data_1[i]) or (shift_left(data_2[i], 16)))
    p2_write_dac32(ModuleAout2, 0, (data_9[i]) or (shift_left(data_10[i], 16)))
    p2_write_dac32(ModuleAout3, 0, (data_17[i]) or (shift_left(data_18[i], 16)))   
    'Setup the output values for channel 3 and 4 on all modules
    p2_write_dac32(ModuleAout1, 1, (data_3[i]) or (shift_left(data_4[i], 16)))
    p2_write_dac32(ModuleAout2, 1, (data_11[i]) or (shift_left(data_12[i], 16)))
    p2_write_dac32(ModuleAout3, 1, (data_19[i]) or (shift_left(data_20[i], 16)))   
    'Setup the output values for channel 5 and 6 on all modules
    p2_write_dac32(ModuleAout1, 2, (data_5[i]) or (shift_left(data_6[i], 16)))
    p2_write_dac32(ModuleAout2, 2, (data_13[i]) or (shift_left(data_14[i], 16)))
    p2_write_dac32(ModuleAout3, 2, (data_21[i]) or (shift_left(data_22[i], 16)))
    'Setup the output values for channel 7 and 8 on all modules
    p2_write_dac32(ModuleAout1, 3, (data_7[i]) or (shift_left(data_8[i], 16)))
    p2_write_dac32(ModuleAout2, 3, (data_15[i]) or (shift_left(data_16[i], 16)))
    p2_write_dac32(ModuleAout3, 3, (data_23[i]) or (shift_left(data_24[i], 16))) 
    
    p2_set_par(ModuleDIO1, 1, 1, data_41[i])        'Set Par_1 of DIO1
    p2_set_par(ModuleDIO2, 1, 1, data_42[i])        'Set Par_1 of DIO2
      
    inc (i)                                                 'Next time output values
    Processdelay = data_51[i]
    par_1 = i
  endif
    
  if ((mode = 0) and (Par_41 = 1)) then  'Computer Controled switch
    inc(mode)
  endif
  
  p2_sync_all(1110000b)                                   'Start output 3 analog out
  
finish:
  'Initialize DAC and DIO output values to be set with stop process
  'Set outputs to a value which will be outputed until next start
  '*****************************************************************
  '********************************************
  'HINT: The Order of the values has to be:
  '      ModuleAout1: data_50 index 1..8 channel 1..8 
  '      ModuleAout2: data_50 index 9..16 channel 1..8 
  '      ModuleAout3: data_50 index 17..24 channel 1..8 
  '      ModuleDIO1: data_50 index 25 all channels
  'Setup the output values for channel 1 and 2 on all modules
  p2_write_dac32(ModuleAout1, 0, (data_50[1]) or (shift_left(data_50[2], 16)))
  p2_write_dac32(ModuleAout2, 0, (data_50[9]) or (shift_left(data_50[10], 16)))
  p2_write_dac32(ModuleAout3, 0, (data_50[17]) or (shift_left(data_50[18], 16)))   
  'Setup the output values for channel 3 and 4 on all modules
  p2_write_dac32(ModuleAout1, 1, (data_50[3]) or (shift_left(data_50[4], 16)))
  p2_write_dac32(ModuleAout2, 1, (data_50[11]) or (shift_left(data_50[12], 16)))
  p2_write_dac32(ModuleAout3, 1, (data_50[19]) or (shift_left(data_50[20], 16)))   
  'Setup the output values for channel 5 and 6 on all modules
  p2_write_dac32(ModuleAout1, 2, (data_50[5]) or (shift_left(data_50[6], 16)))
  p2_write_dac32(ModuleAout2, 2, (data_50[13]) or (shift_left(data_50[14], 16)))
  p2_write_dac32(ModuleAout3, 2, (data_50[21]) or (shift_left(data_50[22], 16)))
  'Setup the output values for channel 7 and 8 on all modules
  p2_write_dac32(ModuleAout1, 3, (data_50[7]) or (shift_left(data_50[8], 16)))
  p2_write_dac32(ModuleAout2, 3, (data_50[15]) or (shift_left(data_50[16], 16)))
  p2_write_dac32(ModuleAout3, 3, (data_50[23]) or (shift_left(data_50[24], 16)))  
  p2_sync_all(1110000b)                                     'Start output 3 analog out
  
  p2_set_par(ModuleDIO1, 1, 1, 0)                   'set DIO start values
  p2_set_par(ModuleDIO2, 1, 1, 0)                   'set DIO start values
  
  
  P2_Set_LED(ModuleAout1, 0) 
  P2_Set_LED(ModuleAout2, 0)
  P2_Set_LED(ModuleAout3, 0)
  'P2_Set_LED(ModuleDIO1, 0) ' commented out, because done in TiCo

