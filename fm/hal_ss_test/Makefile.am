bin_PROGRAMS = hal_ss_test

hal_ss_test_SOURCES:= Qualcomm_FM_Test_HAL_Layer.cpp
hal_ss_test_SOURCES+= FmRadioController_qcom.cpp
hal_ss_test_SOURCES+= ConfigFmThs.cpp
hal_ss_test_SOURCES+= FmPerformanceParams.cpp
hal_ss_test_SOURCES+= QcomFmIoctlsInterface.cpp
hal_ss_test_SOURCES+= ConfFileParser.cpp

hal_ss_test_LDFLAGS:=-pthread
hal_ss_test_LDADD:= $(LIB_PTHREAD)
