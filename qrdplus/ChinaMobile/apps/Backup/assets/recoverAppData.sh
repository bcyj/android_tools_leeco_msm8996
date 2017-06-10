#!/system/bin/sh

# Local variable
DstPath=/data/data
SrcPath=$SECONDARY_STORAGE/backup/App
CfgPath=$SECONDARY_STORAGE/backup/recoverAppData.txt
BusyBoxPath="/system/bin/busybox"
TarTool="$BusyBoxPath tar"

RestoreTagFile="$SECONDARY_STORAGE/backup/App/restoretag"

if [ ! -f "$RestoreTagFile" ]; then
    SrcPath="/storage/sdcard0/backup/App"
    CfgPath="/storage/sdcard0/backup/recoverAppData.txt"
    echo $SrcPath > /storage/sdcard0/backup/App/restore
else
    echo $SrcPath > /storage/sdcard0/backup/App/restore
fi

# Environment Setting
export PATH=/sbin:/system/sbin:/system/bin:/system/xbin:$PATH

# Set Property
setprop "persist.sys.shflag" 0

# Function Definition
strOwner=""
strGroup=""
getMode()
{
    strOwner=""
    strGroup=""
    local file=$1
    ls -dl $file | while read line
    do
        strOwner="${line#* }"
        strGroup="${strOwner#* }"
        index=0
        for item in $strOwner
        do
            if [ "$index" -eq "0" ]
            then
                index=1
                strOwner=$item
            else
                strGroup=$item
                break
            fi
        done
        echo "strOwner=$strOwner" > $file.opt
        echo "strGroup=$strGroup" >> $file.opt
    done
    source $file.opt
    rm $file.opt
}

checkfile()
{
    local file=$1
    local co=$2
    local cg=$3
    local do=$4
    local dg=$5
    getMode $file
    local toOwner=$strOwner
    local toGroup=$strGroup
    [[ "$strOwner" == "$co" ]] && toOwner="$do"
    [[ "$strGroup" == "$cg" ]] && toGroup="$dg"
    if [ -h $file ]
    then
        chown -h $toOwner:$toGroup $file
    else
        if [ -p $file ]
        then
            rm -f $file
        else
            chown $toOwner:$toGroup $file
        fi
    fi
    if [ -d "$file" ]
    then
        for f in $file/*
        do
            checkfile $f $co $cg $do $dg
        done
    fi
}

adjustMode()
{
    local src="$1"
    local dst="$2"
    getMode $src
    local curOwner="$strOwner"
    local curGroup="$strGroup"
    getMode $dst
    local dstOwner="$strOwner"
    local dstGroup="$strGroup"
    [[ "$dstOwner" == "" ]] && dstOwner=$curOwner
    [[ "$dstGroup" == "" ]] && dstGroup=$curGroup
    echo "s=$src d=$dst so=$curOwner sg=$curGroup do=$dstOwner dg=$dstGroup"
    checkfile $src $curOwner $curGroup $dstOwner $dstGroup
}

mkdir -p "$DstPath.recovery"
while read line
do
    local count=1
    while true
    do
        count=$(($count+1))
        if [ -d "$DstPath/$line" ]
        then
            break
        elif [ "$count" -gt "10" ]
        then
            break
        else
            echo "not find $line"
            sleep 1
        fi
    done
    $TarTool -xpf "$SrcPath/$line.tar" -C "$DstPath.recovery"
    adjustMode "$DstPath.recovery/$line" "$DstPath/$line"
    cp -Rpf "$DstPath.recovery/$line" "$DstPath/"
    rm -rf "$DstPath.recovery/$line"
done < "$CfgPath"
rm -rf "$DstPath.recovery"
rm "$CfgPath"
setprop "persist.sys.shflag" 1
