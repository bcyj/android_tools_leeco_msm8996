#---------------------------------------------------------#
# 1.Introduction                                          #
#---------------------------------------------------------#

  This tool is used to pull all apks from DUT, include all preloaded apks(application and framework-res apk),

  to decompile them, to export all string resources, and to analyze and genarate some summary csv/txt files,

  these csv files could be opened using office excel, well-formatted,

  from these generated summary files, user is able to find how many strings are existed in different locales,

  and be able to find which strings are translated or not.

  User can find the tool from "vendor/qcom/proprietary/qrdplus/globalization/multi-language/tool/" directory.


  Note that

  This tool includes 3 files, apktool.jar, DUTStringsAnalysisTool.py, and README.txt

  "apktool.jar" is a jar released by google, which is used to decomple apk by DUTStringsAnalysisTool.py,

  and can be download it from "http://code.google.com/p/android-apktool/downloads/list",

  for the initial version of DUTStringsAnalysisTool.py, download "apktool_2.0.0rc2.jar" from the link above, change name to "apktool.jar".

#---------------------------------------------------------#
# 2.Prerequisition                                        #
#---------------------------------------------------------#

- Make sure available Ubuntu Linux PC or Windows PC

- Make sure Python2.7 is installed on PC.

- Make sure JDK1.7 is installed on PC, if multiple JDKs are installed, make sure 'java -version" is 1.7.0.

- Make sure when executes "adb devices" command in terminal, device can be found.

- Make sure there's enough disk space, suggest to reserve 5 GB available disk space.

- Make sure the "apktool.jar" is placed in this directory: Download the apktool 2.0.0 rc2 tar/zip delivery from the
  link http://code.google.com/p/android-apktool/downloads/list, and unzip apktool_2.0.0rc2.jar from the tar/zip file.
  Place it in this directory, and rename it to "apktool.jar".

#---------------------------------------------------------#
# 3.Steps                                                 #
#---------------------------------------------------------#

1 Connect DUT to PC

2 cd vendor/qcom/proprietary/qrdplus/globalization/multi-language/tool

3 python DUTStringsAnalysisTool.py

* For saving logs of running info, execute "python DUTStringsAnalysisTool.py 2>&1 | tee LogInfo.txt" for linux system,

  execute "python DUTStringsAnalysisTool.py > LogInfo.txt 2>&1" for windows system(but no output on the cmd window), to

  save info in LogInfo.txt.


#---------------------------------------------------------#
# 4.Execution time                                        #
#---------------------------------------------------------#

For Nexus 4 google ref phone, 104 apks, it takes about 6 minutes, for Huawei ref phone, 161 apks, it takes about 5 minutes,

mostly depends on how many apks on your DUT, and for some apks those have lots of strings, of course it will take more time to parse.

#---------------------------------------------------------#
# 5.Output hierarchy                                      #
#---------------------------------------------------------#

Under the current directory, output folder with date/time is generated, hierarchy is as below,

|-- APKsFromDUT                         #All the system apks pulled from DUT are put into this folder.
|   |-- A.apk
|   |-- B.apk
|-- DecompiledAPKs                      #Decompiled contents for each apk are put into this folder.
|   |-- A
|   |   |-- res
|   |       |-- values
|   |       |   |-- ids.xml
|   |       |   |-- public.xml
|   |       |   |-- strings.xml
|   |       |-- values-ar
|   |           |-- strings.xml
|   |-- B
|-- ExportedStrings                     #All strings for each apk are exported into this folder.
|   |-- A
|   |   |-- values-ar.txt               #For A.apk, all Arabic strings are put into this text file.
|   |   |-- values.txt                  #For A.apk, all English strings are put into this text file.
|   |-- B
|   |-- values-ar.txt
|   |-- values.txt
|-- Summary                             #Summary result can be found from this folder
    |-- count_A.txt                     #For A.apk, how many strings exists in different locale.
    |-- count_B.txt
    |-- SUMMARY_A.csv                   #User can open it with excel on window pc,user can check which strings are translated or not be translated.
    |-- SUMMARY_B.csv
    |-- summary.csv                     #For all apks, how many strings exists in different locale.
