================Camera HAL Fuzzing====================
Steps to use fuzzing utility for Qualcomm Camera2 HAL

1. Compile camera-fuzz to generate "libFuzzQCamera2HALFuzzLib.so"
2. Copy 'libFuzzQCamera2HALFuzzLib.so' to '/data/cFuzzer/cFuzzerLibs' on device.
3. Push 'testHAL.xml' to '/data/cFuzzer/cFuzzerTests'
4. Set Display off - adb shell setprop persist.camera.nodisplay 1
5. Run fuzzer tool ( for more info about this visit go/cfuzzer)

