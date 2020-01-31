#-------------------------------------------------
#
# Project created by QtCreator 2019-12-01T13:55:18
#
#-------------------------------------------------

QT       += core gui serialport network script scripttools

LIBS += -L$$PWD/include/PCO -lSC2_Cam
LIBS += -L$$PWD/include/PCO -lPco_conv
LIBS += -L$$PWD/include/PCO_PCI540 -lpf_cam
LIBS += -L$$PWD/include/png++ -llibpng
LIBS += -L$$PWD/include/Lucam -llucamapi
LIBS += -L$$PWD/iXonControl/Lib/ -latmcd32m
LIBS += -L$$PWD/include/IVI -lvisa32
LIBS += -L$$PWD/include/IVI -livi

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = Experiment_Controller_V6
TEMPLATE = app

RC_ICONS = appico.ico

SOURCES += main.cpp\
    Addon/adcscompare.cpp \
    Addon/analogrecording.cpp \
    Addon/autosave.cpp \
    Addon/calculationmonitor.cpp \
    Addon/channelediter.cpp \
    Addon/cycledisplayer.cpp \
    Addon/ddscontrol.cpp \
    Addon/ddsdevicepanel.cpp \
    Addon/displaywindow.cpp \
    Addon/editerwidget.cpp \
    Addon/external.cpp \
    Addon/indicator.cpp \
    Addon/offsetlock_frequencycounter.cpp \
    Addon/qienginedebug.cpp \
    Addon/realtime.cpp \
    Addon/remotecontrolserver.cpp \
    Addon/safety.cpp \
    Addon/sectionediter.cpp \
    Addon/sectiontab.cpp \
    Addon/setting.cpp \
    Addon/systemlog.cpp \
    Addon/variableexpleror.cpp \
    Addon/variablemonitor.cpp \
    Addon/variablewidget.cpp \
    DataType/sectionactivity.cpp \
    DataType/sequence.cpp \
    DataType/variable.cpp \
    include/ddscontroller.cpp \
    include/variableclassdisplay.cpp \
    include/variableeditdialog.cpp \
    iXonControl/datahandler.cpp \
    iXonControl/ixoncamera.cpp \
    iXonControl/ixoncontrol.cpp \
    iXonControl/photoncountsetup.cpp \
    iXonControl/randomtrackinterface.cpp \
    iXonControl/roipreview.cpp \
    iXonControl/settingloader.cpp \
    iXonControl/temperaturecontroller.cpp \
    iXonControl/versioninformation.cpp \
    LucamControl/cameracontrol_lucam.cpp \
    Optotune/lenscontrol_optotune.cpp \
    Pixelfly/camera_screen.cpp \
    Pixelfly/cameracontrol.cpp \
    Pixelfly/cameracontrolqe.cpp \
    Pixelfly/pixelfly_cam.cpp \
    Pixelfly/pixelflyqe_cam.cpp \
    Adwin.cpp \
    maincontroller.cpp \
    include/dsg_signalgenerator.cpp

HEADERS  += \
    Addon/adcscompare.h \
    Addon/analogrecording.h \
    Addon/autosave.h \
    Addon/calculationmonitor.h \
    Addon/channelediter.h \
    Addon/cycledisplayer.h \
    Addon/ddscontrol.h \
    Addon/ddsdevicepanel.h \
    Addon/displaywindow.h \
    Addon/editerwidget.h \
    Addon/external.h \
    Addon/indicator.h \
    Addon/offsetlock_frequencycounter.h \
    Addon/qienginedebug.h \
    Addon/realtime.h \
    Addon/remotecontrolserver.h \
    Addon/safety.h \
    Addon/sectionediter.h \
    Addon/sectiontab.h \
    Addon/setting.h \
    Addon/systemlog.h \
    Addon/variableexpleror.h \
    Addon/variablemonitor.h \
    Addon/variablewidget.h \
    DataType/sectionactivity.h \
    DataType/sequence.h \
    DataType/variable.h \
    include/Lucam/callbacktrigger.h \
    include/Lucam/lucamapi.h \
    include/Lucam/lucamerr.h \
    include/Lucam/lucamsci.h \
    include/Lucam/lucamsequencing.h \
    include/PCO/Pco_ConvDlgExport.h \
    include/PCO/Pco_ConvExport.h \
    include/PCO/Pco_ConvStructures.h \
    include/PCO/PCO_err.h \
    include/PCO/PCO_errt.h \
    include/PCO/PCO_Structures.h \
    include/PCO/SC2_CamExport.h \
    include/PCO/sc2_common.h \
    include/PCO/sc2_defs.h \
    include/PCO/SC2_DialogExport.h \
    include/PCO/SC2_SDKAddendum.h \
    include/PCO/sc2_SDKStructures.h \
    include/PCO_PCI540/errcodes.h \
    include/PCO_PCI540/pcc_struct.h \
    include/PCO_PCI540/Pccam.h \
    include/PCO_PCI540/pccamdef.h \
    include/PCO_PCI540/Pco_ConvDlgExport.h \
    include/PCO_PCI540/Pco_ConvExport.h \
    include/PCO_PCI540/Pco_ConvStructures.h \
    include/PCO_PCI540/PCO_err.h \
    include/PCO_PCI540/PCO_errt.h \
    include/PCO_PCI540/PCO_Structures.h \
    include/PCO_PCI540/PfcamExport.h \
    include/png++/color.hpp \
    include/png++/config.hpp \
    include/png++/consumer.hpp \
    include/png++/convert_color_space.hpp \
    include/png++/end_info.hpp \
    include/png++/error.hpp \
    include/png++/ga_pixel.hpp \
    include/png++/generator.hpp \
    include/png++/gray_pixel.hpp \
    include/png++/image.hpp \
    include/png++/image_info.hpp \
    include/png++/index_pixel.hpp \
    include/png++/info.hpp \
    include/png++/info_base.hpp \
    include/png++/io_base.hpp \
    include/png++/packed_pixel.hpp \
    include/png++/palette.hpp \
    include/png++/pixel_buffer.hpp \
    include/png++/pixel_traits.hpp \
    include/png++/png.h \
    include/png++/png.hpp \
    include/png++/pngconf.h \
    include/png++/reader.hpp \
    include/png++/require_color_space.hpp \
    include/png++/rgb_pixel.hpp \
    include/png++/rgba_pixel.hpp \
    include/png++/solid_pixel_buffer.hpp \
    include/png++/streaming_base.hpp \
    include/png++/tRNS.hpp \
    include/png++/types.hpp \
    include/png++/writer.hpp \
    include/controllerdatagroup.h \
    include/ddscontroller.h \
    include/image_save.h \
    include/qicombobox.h \
    include/scanchangeaction.h \
    include/variableclass.h \
    include/variableclassdisplay.h \
    include/variableeditdialog.h \
    iXonControl/png++/color.hpp \
    iXonControl/png++/config.hpp \
    iXonControl/png++/consumer.hpp \
    iXonControl/png++/convert_color_space.hpp \
    iXonControl/png++/end_info.hpp \
    iXonControl/png++/error.hpp \
    iXonControl/png++/ga_pixel.hpp \
    iXonControl/png++/generator.hpp \
    iXonControl/png++/gray_pixel.hpp \
    iXonControl/png++/image.hpp \
    iXonControl/png++/image_info.hpp \
    iXonControl/png++/index_pixel.hpp \
    iXonControl/png++/info.hpp \
    iXonControl/png++/info_base.hpp \
    iXonControl/png++/io_base.hpp \
    iXonControl/png++/packed_pixel.hpp \
    iXonControl/png++/palette.hpp \
    iXonControl/png++/pixel_buffer.hpp \
    iXonControl/png++/pixel_traits.hpp \
    iXonControl/png++/png.h \
    iXonControl/png++/png.hpp \
    iXonControl/png++/pngconf.h \
    iXonControl/png++/reader.hpp \
    iXonControl/png++/require_color_space.hpp \
    iXonControl/png++/rgb_pixel.hpp \
    iXonControl/png++/rgba_pixel.hpp \
    iXonControl/png++/solid_pixel_buffer.hpp \
    iXonControl/png++/streaming_base.hpp \
    iXonControl/png++/tRNS.hpp \
    iXonControl/png++/types.hpp \
    iXonControl/png++/writer.hpp \
    iXonControl/acquisitionmonitor.h \
    iXonControl/ATMCD32D.H \
    iXonControl/datahandler.h \
    iXonControl/ixoncamera.h \
    iXonControl/ixoncontrol.h \
    iXonControl/overheatmonitor.h \
    iXonControl/photoncountsetup.h \
    iXonControl/previewlabel.h \
    iXonControl/randomtrackinterface.h \
    iXonControl/roipreview.h \
    iXonControl/settingloader.h \
    iXonControl/temperaturecontroller.h \
    iXonControl/versioninformation.h \
    LucamControl/cameracontrol_lucam.h \
    Optotune/lenscontrol_optotune.h \
    Pixelfly/camera_screen.h \
    Pixelfly/cameracontrol.h \
    Pixelfly/cameracontrolqe.h \
    Pixelfly/pixelfly_cam.h \
    Pixelfly/pixelflyqe_cam.h \
    Adwin.h \
    ChannelCalculator.h \
    definition.h \
    maincontroller.h \
    md5.h \
    qiscriptenginecluster.h \
    qiserial.h \
    SequenceCalculator.h \
    spline.h \
    StdAfx.h \
    include/IVI/ivi.h \
    include/IVI/IviACPwr_ni.h \
    include/IVI/IviCounter_ni.h \
    include/IVI/IviDCPwr.h \
    include/IVI/IviDCPwrObsolete.h \
    include/IVI/IviDigitizer_ni.h \
    include/IVI/IviDmm.h \
    include/IVI/IviDmmObsolete.h \
    include/IVI/IviDownconverter_ni.h \
    include/IVI/IviFgen.h \
    include/IVI/IviFgenObsolete.h \
    include/IVI/IviPwrMeter.h \
    include/IVI/IviRFSigGen.h \
    include/IVI/IviScope.h \
    include/IVI/IviScopeObsolete.h \
    include/IVI/IviSpecAn.h \
    include/IVI/IviSwtch.h \
    include/IVI/IviSwtchObsolete.h \
    include/IVI/IviUpconverter_ni.h \
    include/IVI/visa.h \
    include/IVI/visatype.h \
    include/IVI/vpptype.h \
    include/dsg_signalgenerator.h

FORMS    += \
    Addon/adcscompare.ui \
    Addon/analogrecording.ui \
    Addon/autosave.ui \
    Addon/calculationmonitor.ui \
    Addon/channelediter.ui \
    Addon/cycledisplayer.ui \
    Addon/ddscontrol.ui \
    Addon/ddsdevicepanel.ui \
    Addon/displaywindow.ui \
    Addon/editerwidget.ui \
    Addon/external.ui \
    Addon/indicator.ui \
    Addon/offsetlock_frequencycounter.ui \
    Addon/qienginedebug.ui \
    Addon/realtime.ui \
    Addon/safety.ui \
    Addon/sectionediter.ui \
    Addon/sectiontab.ui \
    Addon/setting.ui \
    Addon/systemlog.ui \
    Addon/variableexpleror.ui \
    Addon/variablemonitor.ui \
    Addon/variablewidget.ui \
    include/variableclassdisplay.ui \
    include/variableeditdialog.ui \
    iXonControl/ixoncontrol.ui \
    iXonControl/photoncountsetup.ui \
    iXonControl/randomtrackinterface.ui \
    iXonControl/roipreview.ui \
    iXonControl/settingloader.ui \
    iXonControl/versioninformation.ui \
    LucamControl/cameracontrol_lucam.ui \
    Optotune/lenscontrol_optotune.ui \
    Pixelfly/camera_screen.ui \
    Pixelfly/cameracontrol.ui \
    Pixelfly/cameracontrolqe.ui \
    maincontroller.ui

DISTFILES += \
    iXonControl/Lib/atmcd32d.lib \
    include/Lucam/lucamapi.lib \
    include/Lucam/lucamapi64.lib \
    include/PCO/Pco_conv.lib \
    include/PCO/SC2_Cam.lib \
    include/PCO_PCI540/pccam.lib \
    include/PCO_PCI540/pf_cam.lib \
    include/png++/libpng.lib \
    iXonControl/Lib/atmcd32m.lib \
    iXonControl/Lib/atmcd64m.lib \
    iXonControl/png++/libpng.lib \
    include/Lucam/lucamapi.dll \
    include/Lucam/lucamapi64.dll \
    iXonControl/libpng12.dll \
    iXonControl/libpng3.dll \
    iXonControl/zlib1.dll \
    New folder/libpng12.dll \
    New folder/libpng3.dll \
    New folder/PCO_CDlg.dll \
    New folder/PCO_Conv.dll \
    New folder/PCO_CryptDll.dll \
    New folder/SC2_Cam.dll \
    New folder/SC2_DLG.dll \
    New folder/zlib1.dll \
    libpng12.dll \
    libpng3.dll \
    PCO_CDlg.dll \
    PCO_Conv.dll \
    PCO_CryptDll.dll \
    SC2_Cam.dll \
    SC2_DLG.dll \
    zlib1.dll \
    appico.ico \
    New folder/Config.txt \
    New folder/Default.txt \
    New folder/Setting_List.txt \
    New folder/System_log.txt \
    Default.txt \
    include/IVI/ivi.lib \
    include/IVI/visa32.lib \
    DLLs/ATMCD32CS.dll \
    DLLs/atmcd32d.dll \
    DLLs/ATMCD64CS.dll \
    DLLs/atmcd64d.dll \
    DLLs/libpng12.dll \
    DLLs/libpng3.dll \
    DLLs/PCO_CDlg.dll \
    DLLs/PCO_Conv.dll \
    DLLs/PCO_CryptDll.dll \
    DLLs/SC2_Cam.dll \
    DLLs/SC2_DLG.dll \
    DLLs/WdfCoInstaller01011.dll \
    DLLs/winusbcoinstaller2.dll \
    DLLs/zlib1.dll \
    DLLs/Config.txt \
    DLLs/Default.txt \
    DLLs/Setting_List.txt \
    DLLs/System_log.txt \
    include/IVI/visa32.bas \
    include/IVI/vpptype.bas

SUBDIRS += \
    iXonControl/iXonControl.pro \
    iXonControl/iXonControl.pro
