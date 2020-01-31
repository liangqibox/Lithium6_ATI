/*****************************************************************************
*
* Copyright (c) Lumenera Corporation 2002-2008. All rights reserved.
*
*****************************************************************************/
#ifndef __LUCAMSCI_H
#define __LUCAMSCI_H

typedef enum {
  DP_LOW2HIGH_X2        = 0x01, // 4 images   (Xx1 Yx1)      i.e 1280 x 1024 -> 1280 x 1024
  DP_LOW2HIGH_X4        = 0x02, // 16 images  (Xx2 Yx2)
  DP_LOW2HIGH_X6        = 0x04, // 36 images  (Xx3 Yx3)
  DP_LOW2HIGH_X8        = 0x08, // 64 images  (Xx4 Yx4)
  // -------------------------------------------------------------------
  DP_LOW2HIGH_IMG2A     = 0x21, // 2 images   (Xx1 Yx1)      i.e 1280 x 1024 -> 1280 x 1024
  DP_LOW2HIGH_IMG3A     = 0x22, // 3 images   (Xx1.5 Yx1.5)  i.e 1280 x 1024 -> 1.5x1280 x 1.5x1024
  DP_LOW2HIGH_IMG4A     = 0x23, // 4 images   (Xx2 Yx2)      -
  DP_LOW2HIGH_IMG5A     = 0x24, // 5 images   (Xx2.5 Yx2.5)  -
  DP_LOW2HIGH_IMG6A     = 0x25, // 6 images   (Xx1.5 Yx1.5)  -
  DP_LOW2HIGH_IMG8A     = 0x26, // 8 images   (Xx2 Yx2)      -
  DP_LOW2HIGH_IMG16A    = 0x27, // 16 images  (Xx4 Yx4)      -
  // -------------------------------------------------------------------
  DP_LOW2HIGH_IMG2B     = 0x41, // 2 images   (Xx2 Yx2)      -
  DP_LOW2HIGH_IMG3B     = 0x42, // 3 images   (Xx2 Yx2)      -
  DP_LOW2HIGH_IMG4B     = 0x43, // 4 images   (Xx4 Yx4)      -
  DP_LOW2HIGH_IMG5B     = 0x44, // 5 images   (Xx2.5 Yx2.5)  i.e 1280 x 1024 -> 3200 x 2560
  DP_LOW2HIGH_IMG6B     = 0x45, // 6 images   (Xx3 Yx3)      -
  DP_LOW2HIGH_IMG8B     = 0x46, // 8 images   (Xx4 Yx4)      -
  DP_LOW2HIGH_IMG9B     = 0x47, // 9 images   (Xx3 Yx3)      i.e 1280 x 1024 -> 3x1280 x 3x1024
  // -------------------------------------------------------------------
  DP_LOW2HIGH_MODEMASK  = 0x007f,  //
  DP_LOW2HIGH_24BPP     = 0x0000,  // Camera captures 16bpp, DeltaVu generates 24bpp RGB
  DP_LOW2HIGH_48BPP     = 0x0080,  // Camera captures 16bpp, DeltaVu generates 48bpp RGB
  // -------------------------------------------------------------------
} DP_PROC_T;


// Callback for monitoring the progress. May be NULL.
// Return false from the callback if you want to cancel the operation.
typedef BOOL (CALLBACK * BUILD_HIGH_RES_CALLBACK)(PVOID context, int processing, int percentageCompleted);


LUCAM_API 
BOOL LUCAM_EXPORT LucamBuildHighResImage(HANDLE hCamera, 
                                         LUCAM_FRAME_FORMAT *pFormat, 
                                         UCHAR *pBmpOutputImage, 
                                         ULONG cmd);

LUCAM_API 
BOOL LUCAM_EXPORT LucamBuildHighResImageEx(HANDLE hCamera, 
                                         LUCAM_FRAME_FORMAT *pFormat, 
                                         UCHAR *pBmpOutputImage, 
                                         ULONG cmd,
                                         BUILD_HIGH_RES_CALLBACK callback,
                                         PVOID contextForCallback);


#endif // __LUCAMSCI_H




