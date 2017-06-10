#!/system/bin/sh

# Local variable
SrcPath=/data/data
DstPath="$SECONDARY_STORAGE/backup/App"
CfgPath="$SECONDARY_STORAGE/backup/backupAppData.txt"
BusyBoxPath="/system/bin/busybox"
TarTool="$BusyBoxPath tar"

BackupTagFile="$SECONDARY_STORAGE/backup/App/backuptag"

if [ ! -f "$BackupTagFile" ]; then
    DstPath="/storage/sdcard0/backup/App"
    CfgPath="/storage/sdcard0/backup/backupAppData.txt"
    echo $DstPath > /storage/sdcard0/backup/App/backup
else
    echo $DstPath > /storage/sdcard0/backup/App/backup
fi

# Environment Setting
export PATH=/sbin:/system/sbin:/system/bin:/system/xbin:$PATH

# Set Property
setprop "persist.sys.shflag" 0
mkdir -p $DstPath
while read line
do
    echo "$line"
    cd "$SrcPath"
    $TarTool -cpf "$DstPath/$line.tar" "$line"
    cd -
done < "$CfgPath"

rm "$CfgPath"
setprop "persist.sys.shflag" 1
