#!/usr/bin/python
"""
Copyright (c) 2015 Qualcomm Technologies, Inc.  All Rights Reserved.
Qualcomm Technologies Proprietary and Confidential.
"""

import os
import time
import commands
import shutil

def get_files_by_suffix(dirpath, suffix):
    filelist = []
    for root, dirs, files in os.walk(dirpath):
        filelist.extend(map(lambda x:os.path.join(root, x), filter(lambda y:y.endswith(suffix), files)))
    return filelist

def pull_apks(srcpath, outpath):
    print "Begin to Pull apks from " + srcpath
    src = srcpath
    tmp = os.path.join(outpath, "tmpAPKsFromDUT")
    dst = os.path.join(outpath, "APKsFromDUT")
    os.makedirs(tmp)
    if not os.path.exists(dst):
        os.makedirs(dst)

    print "Pulling all files from " + src + " to " + tmp
    status = os.system("adb pull " + src + " " + tmp)
    if status != 0:
        print "Pull " + src + " failed. Try again, please."
        exit(1)
    print "Pull files done. Saving apks ..."

    apklist = get_files_by_suffix(tmp, ".apk")

    for apk in apklist:
        apkname = os.path.basename(apk)
        os.rename(apk, os.path.join(dst, apkname))

    print "Save apks done. Deleting tmp files ..."
    shutil.rmtree(tmp, True)

    print "Pull apks from " + srcpath + " done."

def decompile_apks(outpath):
    src = os.path.join(outpath, "APKsFromDUT")
    dst = os.path.join(outpath, "DecompiledAPKs")
    os.makedirs(dst)

    apklist = get_files_by_suffix(src, ".apk")
    nowpath = os.getcwd()
    for apk in apklist:
        apkname = os.path.basename(apk)
        os.chdir(os.path.join(nowpath, dst))
        jarpath = ".." + os.path.sep + ".." + os.path.sep + "apktool.jar"
        srcpath = ".." + os.path.sep + "APKsFromDUT" + os.path.sep + apkname
        status = os.system("java -jar " + jarpath + " d -f " + srcpath)
        os.chdir(nowpath)
        if status != 0:
            print "Decompile " + apk + " failed."
            failedinfofile = open(os.path.join(dst, "DecompileFailedAPKs.txt"), "a")
            failedinfofile.write(os.path.basename(apk) + os.linesep)
            failedinfofile.close()

def export_strings(srcpath, dstpath):
    print "Getting strings from " + srcpath + " ..."
    src = open(srcpath, "r")
    dst = open(dstpath, "a")
    filepath = srcpath[srcpath.find(os.path.sep)+1:]
    filepath = filepath[filepath.find(os.path.sep)+1:]
    for line in src:
        if -1 != line.find("<string name="):
            nameindex = line.find("name=")
            name = line[nameindex+6:]
            nameindex = name.find("\"")
            name = "\"" + name[:nameindex] + "\""
            valueindex1 = line.find(">")
            valueindex2 = line.find("</string>")
            value = line[valueindex1+1:valueindex2]
            dst.write(filepath + "\t" + name + "\t" + value + os.linesep)
    src.close()
    dst.close()

def export_strings_from_apks(outpath):
    src = os.path.join(outpath, "DecompiledAPKs")
    dst = os.path.join(outpath, "ExportedStrings")
    os.makedirs(dst)
    folderlist = os.listdir(src)
    for folder in folderlist:
        print "Export strings from : " + folder
        srcdir = os.path.join(src, folder)
        dstdir = os.path.join(dst, folder)
        os.makedirs(dstdir)

        filelist = get_files_by_suffix(srcdir, "strings.xml")
        for f in filelist:
            filepathsplit = f.split(os.path.sep)
            export_strings(f, os.path.join(dstdir, filepathsplit[-2] + ".txt"))

def generate_total_result(outpath):
    workpath = os.path.join(outpath, "Summary")
    filelist = get_files_by_suffix(workpath, ".txt")
    filelist.sort()
    title = "apkname"
    summarydic = {}
    checkdic = {}
    filecount = 0
    for f in filelist:
        filecount += 1
        basename = os.path.basename(f)
        apkname = basename[basename.find("count_")+6:basename.rfind(".txt")]
        title = title + "," +apkname;

        srcfile = open(f, "r")
        for line in srcfile:
            linesplit = line.strip(os.linesep).split("\t")
            langname = linesplit[0]
            number = linesplit[1]
            if langname in summarydic:
                summarydic[langname] = summarydic[langname] + "," + number
                checkdic[langname] = True
            else:
                initstr = ""
                for i in range(filecount-1):
                    initstr = initstr + ",0"
                initstr = initstr + "," + number
                summarydic[langname] = initstr
                checkdic[langname] = True
        srcfile.close()

        for d,x in summarydic.items():
            if not checkdic[d]:
                summarydic[d] = x + ",0"
            checkdic[d] = False

    print "Write total summary file: summary.csv"
    summaryfile = open(os.path.join(workpath, "summary.csv"), "w")
    summaryfile.write(title + os.linesep)
    for d,x in summarydic.items():
        summaryfile.write(d + x + os.linesep)
    summaryfile.close()

def summarize_results(outpath):
    src = os.path.join(outpath, "ExportedStrings")
    dst = os.path.join(outpath, "Summary")
    os.makedirs(dst)

    folderlist = os.listdir(src)
    for folder in folderlist:
        print "Analisis apk: " + folder
        summaryfilename = "SUMMARY_" + folder + ".csv"
        countfilename = "count_" + folder + ".txt"
        countfile = open(os.path.join(dst, countfilename), "w")

        srcdir = os.path.join(src, folder)
        filelist = get_files_by_suffix(srcdir, ".txt")
        filelist.sort()

        title = "stringname"
        summarydic = {}
        filecount = 0
        for f in filelist:
            print "file: " + f
            filecount += 1
            basename = os.path.basename(f)
            basename = basename[:basename.rfind(".txt")]
            title = title + "," + basename;

            count = 0
            srcfile = open(f, "r")
            for line in srcfile:
                count += 1
                linesplit = line.split("\t")
                stringname = linesplit[1]
                if stringname in summarydic:
                    summarydic[stringname] = summarydic[stringname] + ",Y"
                else:
                    initstr = ""
                    for i in range(filecount-1):
                        initstr = initstr + ",N"
                    initstr = initstr + ",Y"
                    summarydic[stringname] = initstr
            srcfile.close()

            countfile.write(basename + "\t" + str(count) + os.linesep)

            for d,x in summarydic.items():
                if len(x) == (filecount-1)*2:
                    summarydic[d] = x + ",N"
                elif len(x) != filecount*2:
                    print "error " + f

        countfile.close()

        print "Write results: " + summaryfilename + " and " + countfilename
        summaryfile = open(os.path.join(dst, summaryfilename), "w")
        summaryfile.write(title + os.linesep)
        for d,x in summarydic.items():
            summaryfile.write(d + x + os.linesep)
        summaryfile.close()
    #generate a total summary file for apks(folders)
    generate_total_result(outpath)

    failedfilepath = reduce(os.path.join, [outpath, "DecompiledAPKs", "DecompileFailedAPKs.txt"])
    if os.path.exists(failedfilepath):
        os.rename(failedfilepath, os.path.join(dst, ".DecompileFailedAPKs.txt"))

if __name__ == '__main__':
    print "[O/7] Process start..."
    outfolder = "AnalysisResult-" + time.strftime('%Y%m%d-%H%M',time.localtime(time.time()))
    print "[1/7] Pulling apks from /system/app ..."
    pull_apks("/system/app", outfolder)
    print "[2/7] Pulling apks from /system/priv-app ..."
    pull_apks("/system/priv-app", outfolder)
    print "[3/7] Pulling apks from /system/framework ..."
    pull_apks("/system/framework", outfolder)
    print "[4/7] Decompiling apks ..."
    decompile_apks(outfolder)
    print "[5/7] Exporting strings from decompiled apks ..."
    export_strings_from_apks(outfolder)
    print "[6/7] Summarizing results ..."
    summarize_results(outfolder)
    print "[7/7] Process done."
