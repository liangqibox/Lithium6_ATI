#ifndef LUCAMSEQUENCING_H
#define LUCAMSEQUENCING_H

#define LUCAM_SEQUENCING_SETTING_TYPE_PROPERTY        1
typedef struct _LUCAM_SEQUENCE_SETTING_HEADER
{
   USHORT Type;     // see LUCAM_SEQUENCING_SETTING_TYPE_* above
   USHORT Size;     // set this to sizeof(LUCAM_SEQUENCE_SETTING)
   USHORT Frame;    // zero-based sequence number
   USHORT Reserved; // set this to 0
}LUCAM_SEQUENCE_SETTING_HEADER;
typedef struct _LUCAM_SEQUENCE_SETTING_PROPERTY
{
   LUCAM_SEQUENCE_SETTING_HEADER Header;
   ULONG Property; // LUCAM_PROP_*, for example LUCAM_PROP_STILL_EXPOSURE
   LONG Flags;     // LUCAM_PROP_FLAG_*
   FLOAT Value;
}LUCAM_SEQUENCE_SETTING_PROPERTY;
typedef struct _LUCAM_SEQUENCE_SETTING
{
   union 
   {
      LUCAM_SEQUENCE_SETTING_HEADER Hdr;
      LUCAM_SEQUENCE_SETTING_PROPERTY Property;
   };
}LUCAM_SEQUENCE_SETTING; // use with LucamSequencingSetup

#define LUCAM_SEQUENCING_CONTROL_OFF                  0
#define LUCAM_SEQUENCING_CONTROL_ON_IN_SNAPSHOT_MODE  2
#define LUCAM_SEQUENCING_CONTROL_CIRCULAR             0x80000000
typedef ULONG    LUCAM_SEQUENCING_CONTROL; // use with LucamSequencingControl and LucamSequencingGetStatus

typedef struct _LUCAM_SEQUENCING_STATUS
{
   ULONG MaximumFrameCountInSequence;  // if 0 then sequencing is not supported
   ULONG CurrentFrameCountInSequence;  // corresponds to LucamSequencingSetup's framesPerSequence
   ULONG NextFrameCount;               // 'timestamp' of the next frame
   ULONG NextFrameIndexInSequence;     // zero-based sequence of the next frame
   LUCAM_SEQUENCING_CONTROL State;     // will automatically clear if LUCAM_SEQUENCING_CONTROL_CIRCULAR is not set
   ULONG Reserved;
}LUCAM_SEQUENCING_STATUS; // use with LucamSequencingGetStatus

LUCAM_API BOOL LUCAM_EXPORT LucamSequencingSetup(HANDLE hCamera, ULONG framesPerSequence, ULONG settingsCount, LUCAM_SEQUENCE_SETTING pSettings[], LUCAM_SEQUENCING_CONTROL code);
LUCAM_API BOOL LUCAM_EXPORT LucamSequencingTakeSequence(HANDLE hCamera, ULONG frameCount, BYTE *pFrames[], ULONG expectedInitialSequenceIndex=0);
LUCAM_API BOOL LUCAM_EXPORT LucamSequencingGetStatus(HANDLE hCamera, LUCAM_SEQUENCING_STATUS *pStatus);
LUCAM_API LONG LUCAM_EXPORT LucamSequencingGetIndexForFrame(HANDLE hCamera, BYTE *pFrame);
#define LucamSequencingCancelTakeSequence LucamCancelTakeFastFrame


#endif // LUCAMSEQUENCING_H
