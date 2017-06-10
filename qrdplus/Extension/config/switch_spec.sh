#!/system/bin/sh
# Copyright (c) 2013, Qualcomm Technologies, Inc. All Rights Reserved.
#
# Qualcomm Technologies Proprietary and Confidential.
#
#
# version 1.1.1
#
export PATH=/system/bin:$PATH

strBakForReplace=".bakforspec"
strExcludeFiles="exclude.list"
strExcludeFolder="exclude"
strForLink=".link"
currentSpec=""
SourceFolder=""
DestFolder=""
BasePath=""
LocalFlag=""
mode="running"

createFolder()
{
  local dirPath=$1
  if [ -d "$dirPath" ]
  then
    echo "Exist $dirPath"
  else
    createFolder "${dirPath%/*}"
    echo "mkdir and chmod $dirPath"
    mkdir "$dirPath"
    chmod 755 "$dirPath"
  fi
}

installFunc()
{
  local srcPath=$1
  local dstPath=$2
  local dstDir="${dstPath%/*}"
  createFolder $dstDir
  echo "installFunc $srcPath $dstPath $dstDir"
  if [ "${dstPath%$strForLink}" != "$dstPath" ]
  then
    dstPath="${dstPath%$strForLink}"
  fi
  if [ "${srcPath%$strForLink}" != "$srcPath" ]
  then
    if [ "${dstPath#${BasePath}/system/}" != "${dstPath}" ]
    then
      if [ -f "${srcPath%$strForLink}" ] || [ -h "${srcPath%$strForLink}" ]
      then
        if [ -f "$dstPath" ]
        then
          rm "$dstPath"
        fi
        mv "${srcPath%$strForLink}" $dstPath
        chmod 644 "$dstPath"
      fi
    else
      cp -p "${srcPath%$strForLink}" $dstPath
      chmod 644 "$dstPath"
    fi
  elif [ -h "$srcPath$strForLink" ]
  then
    installFunc "$srcPath$strForLink" $dstPath
  else
    if [ -f "$dstPath" ]
    then
      if [ -f "$dstPath$strBakForReplace$currentSpec" ]
      then
        rm "$dstPath$strBakForReplace$currentSpec"
      fi
      mv $dstPath $dstPath$strBakForReplace$currentSpec
    fi
    ln -s ${dstPath#$BasePath} "$srcPath$strForLink"
    installFunc "$srcPath$strForLink" $dstPath
  fi
}

uninstallFunc()
{
  local srcPath=$1
  local dstPath=$2
  echo "uninstallFunc $srcPath $dstPath"
  if [ "${dstPath%$strForLink}" != "$dstPath" ]
  then
    dstPath="${dstPath%$strForLink}"
  fi
  if [ "${srcPath%$strForLink}" != "$srcPath" ]
  then
    if [ "${dstPath#${BasePath}/system/}" != "${dstPath}" ]
    then
      if [ -f "$dstPath" ] || [ -h "$dstPath" ]
      then
        if [ -f "${srcPath%$strForLink}" ] || [ -h "${srcPath%$strForLink}" ]
        then
          rm $dstPath
        else
          if [ -f "${srcPath%$strForLink}" ]
          then
            rm "${srcPath%$strForLink}"
          fi
          mv $dstPath "${srcPath%$strForLink}"
          chmod 644 "${srcPath%$strForLink}"
        fi
      fi
    else
      rm $dstPath
    fi
    if [ -f "$dstPath$strBakForReplace$currentSpec" ]
    then
      if [ -f "$dstPath" ]
      then
        rm $dstPath
      fi
      mv $dstPath$strBakForReplace$currentSpec $dstPath
    fi
    rm $srcPath
  elif [ -h "$srcPath$strForLink" ]
  then
    uninstallFunc "$srcPath$strForLink" $dstPath
  else
    echo "Finish install"
  fi
}

installFolderFunc()
{
  local srcPath=$1
  local dstPath=$2
  for item in `ls -a $srcPath`
  do
    echo "find item=$item"
    if [ "$item" = "." ]
    then
      echo "current folder"
    else
      if [ "$item" = ".." ]
      then
        echo "upfolder"
      elif [ "$item" = ".preloadspec" ] || [ "$item" = "$strExcludeFiles" ]
      then
        echo "specflag"
      else
        if [ -f "$srcPath/$item" ]
        then
          installFunc "$srcPath/${item}" "$dstPath/${item}"
        elif [ -h "$srcPath/$item" ]
        then
          installFunc "$srcPath/${item}" "$dstPath/${item}"
        else
          if [ -d "$srcPath/$item" ]
          then
            installFolderFunc "$srcPath/${item}" "$dstPath/${item}"
          fi
        fi
      fi
    fi
  done
}

uninstallFolderFunc()
{
  local srcPath=$1
  local dstPath=$2
  for item in `ls -a $srcPath`
  do
    echo "uitem=$item"
    if [ "$item" = "." ]
    then
      echo "current folder"
    else
      if [ "$item" = ".." ]
      then
        echo "upfolder"
      elif [ "$item" = ".preloadspec" ] || [ "$item" = "$strExcludeFiles" ]
      then
        echo "specflag"
      else
        if [ -f "$srcPath/$item" ]
        then
          uninstallFunc "$srcPath/${item}" "$dstPath/${item}"
        elif [ -h "$srcPath/$item" ]
        then
          uninstallFunc "$srcPath/${item}" "$dstPath/${item}"
        else
          if [ -d "$srcPath/$item" ]
          then
            uninstallFolderFunc "$srcPath/${item}" "$dstPath/${item}"
          fi
        fi
      fi
    fi
  done
}

excludeFilesFunc()
{
  local srcPath=$1
  if [ -f "$srcPath" ]
  then
    echo "exclude the files in current carrier"
    while read line
    do
      if [ -f "$DestFolder/$line" ]
      then
        local dstPath="$SourceFolder/$strExcludeFolder/$line"
        local dstDir="${dstPath%/*}"
        createFolder $dstDir
        if [ "${line#system/}" != "${line}" ]
        then
          mv $DestFolder/$line $dstPath
        else
          cp -p $DestFolder/$line $dstPath
        fi
      fi
    done < "$srcPath"
  fi
}

includeFilesFunc()
{
  local srcPath=$1
  if [ -f "$srcPath" ]
  then
    echo "restore the files excluded in previous carrier"
    while read line
    do
      if [ -f "$SourceFolder/$strExcludeFolder/$line" ]
      then
        local dstPath="$DestFolder/$line"
        if [ "${line#system/}" != "${line}" ]
        then
          mv "$SourceFolder/$strExcludeFolder/$line" $dstPath
        else
          cp -p "$SourceFolder/$strExcludeFolder/$line" $dstPath
        fi
      fi
    done < "$srcPath"
  fi
}

getCurrentCarrier()
{
  local specPath=$1
  currentSpec=""
  if [ -f "$specPath" ]
  then
    . $specPath
    while read line
    do
      currentSpec=${line#*=}
    done < $specPath
  fi
}

makeFlagFolder()
{
  if [ -d "$DestFolder/data/switch_spec" ]
  then
    echo "no need to create flag"
  else
    mkdir "$DestFolder/data/switch_spec"
    chmod 770 "$DestFolder/data/switch_spec"
    if [ "$mode" = "running" ]
    then
      chown system:system "$DestFolder/data/switch_spec"
    fi
  fi
}

changeDirMode()
{
  local strCurPath=$1
  chmod 755 $strCurPath
  for item in `ls -a $strCurPath/`
  do
    if [ "$item" = "." ] || [ "$item" = ".." ]
    then
      echo ".."
    elif [ -f "$strCurPath/$item" ]
    then
      chmod 644 "$strCurPath/$item"
    elif [ -d "$strCurPath/$item" ]
    then
      changeDirMode "$strCurPath/$item"
    else
      echo "who is $strCurPath/$item"
    fi
  done
}

recoveryDataPartition()
{
  getCurrentCarrier "$DestFolder/system/vendor/speccfg/spec"
  installFolderFunc "$DestFolder/system/vendor/Default/data" "$DestFolder/data"
  if [ "$currentSpec" = "" ] || [  "$currentSpec" = "Default" ]
  then
    echo "Not find spec or default spec"
  else
    # Recovery the data partition for each carrier
    local specPath=$1
    x=0
    while read line
    do
      if [ "$x" -ge "1" ]
      then
        installFolderFunc "$DestFolder/system/vendor/${line#*=}/data" "$DestFolder/data"
      fi
      let "x+=1"
    done < $specPath
  fi
}

getActionSpecList()
{
  local app=$1
  local specList
  local SwitchFlag="$DestFolder/system/vendor/speccfg/spec.new"
  if [ -f "$app/cache/action" ]
  then
    cp -rf "$app/cache/action" "$SwitchFlag"
  fi
  if [ -f "$SwitchFlag" ]
  then
    strNewSpec=""
    newPackCount=0
    . "$SwitchFlag"
    if [ "$newPackCount" -ge "1" ]
    then
      local x=0
      while read line
      do
        if [ "$x" -ge "1" ]
        then
          specItem="${line#*=}"
          specList[$x-1]=$specItem
        fi
        let "x+=1"
      done < $SwitchFlag
    else
      specList[0]=$strNewSpec
    fi
  fi
  if [ -f "$SwitchFlag" ]
  then
    rm -rf "$SwitchFlag"
  fi
  echo ${specList[*]}
}

uninstallOldSpecList()
{
  local specPath=$1
  local specList
  if [ -f "$specPath" ]
  then
    local x=0
    while read line
    do
      if [ "$x" -ge "1" ]
      then
        specList[$x-1]="${line#*=}"
      fi
      let "x+=1"
    done < "$specPath"
  fi

  x="${#specList[@]}"
  while [ "$x" -gt "0" ]
  do
    let "x-=1"
    if [ "$x" -ge "1" ]
    then
      currentSpec=${specList[$x-1]}
    else
      currentSpec="Default"
    fi
    if [ "${specList[$x]}" != "Default" ]
    then
      uninstallFolderFunc "$SourceFolder/${specList[$x]}" "$DestFolder"
      includeFilesFunc "$SourceFolder/${specList[$x]}/$strExcludeFiles"
    fi
  done
  rm -rf $SourceFolder/$strExcludeFolder/*

  # Reinstall Default pack
  mv -f $DestFolder/system/build.prop.bakforspecDefault $DestFolder/system/build.prop
  uninstallFolderFunc "$SourceFolder/Default" "$DestFolder"
  installFolderFunc "$SourceFolder/Default" "$DestFolder"
  echo "packCount=1" > $specPath
  echo "strSpec1=Default" >> $specPath
}

overrideRoProperty()
{
  local srcprop=$1
  local dstprop=$2
  local tempfile="temp.prop"

  echo "Override ro.* property from $srcprop to $dstprop ..."

  while IFS=$'\n' read -r srcline
  do
    if [ "${srcline:0:1}" != "#" ] && [ "${srcline#*=}" != "${srcline}" ]
    then
      local flag=0
      while IFS=$'\n' read -r dstline
      do
        if [ "${srcline%%.*}" == "ro" ] && [ "${srcline%%[ =]*}" == "${dstline%%[ =]*}" ]
        then
          echo "Override $srcline ..."
          echo -E $srcline >> $tempfile
        else
          echo -E $dstline >> $tempfile
        fi
      done < $dstprop
      mv -f $tempfile $dstprop
    fi
  done < $srcprop

  chmod 644 $dstprop
}

installNewSpecList()
{
  local specList
  specList=(`echo "$@"`)
  local data="${specList[0]}"

  # Check if the list is ready
  local x=1
  local y=0
  local newList
  if [ "${#specList[@]}" -ge "1" ]
  then
    while [ "$x" -lt "${#specList[@]}" ]
    do
      if [ "${specList[$x]}" != "" ]
      then
        # Copy spec folder from $data/cache/system/vendor to /system/vendor
        if [ -d "$data/cache/system/vendor/${specList[$x]}" ]
        then
          if [ -d "$SourceFolder/${specList[$x]}" ]
          then
            rm -rf "$SourceFolder/${specList[$x]}"
          fi
          cp -rf "$data/cache/system/vendor/${specList[$x]}" "$SourceFolder/${specList[$x]}"
        fi
        if [ -d "$SourceFolder/${specList[$x]}" ]
        then
          newList[$y]=${specList[$x]}
          let "y+=1"
        fi
      fi
      let "x+=1"
    done
  fi

  if [ "$mode" = "running" ]
  then
    wipe data
  fi
  installFolderFunc "$SourceFolder/Default/data" "$DestFolder/data"

  if [ "${#newList[@]}" -ge "1" ]
  then
    if [ "${newList[0]}" != "Default" ]
    then
      # Backup build.prop for Default
      cp -f $DestFolder/system/build.prop $DestFolder/system/build.prop.bakforspecDefault
      x=0
      echo "packCount=${#newList[@]}" > $LocalFlag
      while [ "$x" -lt "${#newList[@]}" ]
      do
        excludeFilesFunc "$SourceFolder/${newList[$x]}/$strExcludeFiles"
        changeDirMode "$SourceFolder/${newList[$x]}"
        installFolderFunc "$SourceFolder/${newList[$x]}" "$DestFolder"
        overrideRoProperty "$DestFolder/system/vendor/vendor.prop" "$DestFolder/system/build.prop"
        let "x+=1"
        currentSpec="${newList[$x-1]}"
        echo "strSpec$x=$currentSpec" >> $LocalFlag
      done
    fi
  fi
}

cleanOldSpecs()
{
  local specList
  specList=(`echo "$@"`)
  local data="${specList[0]}"

  for item in `ls -a $SourceFolder`
  do
    if [ "$item" = "Default" ]
    then
      echo "Default carrier, no need remove"
    elif [ "$item" = ".." ] || [ "$item" = "." ]
    then
      echo "Current path"
    elif [ -f "$SourceFolder/$item/.preloadspec" ]
    then
      echo "find $item"
      local x=1
      local flag=0
      while [ "$x" -lt "${#specList[@]}" ]
      do
        if [ "$item" = "${specList[$x]}" ] && [ ! -d "$data/$item" ]
        then
          flag=1
          break
        fi
        let "x+=1"
      done

      if [ "$flag" -eq "0" ]
      then
        rm -rf "$SourceFolder/$item"
      fi
    fi
  done
}

######Main function start######

if [ "$#" -eq "0" ]
then
  if [ -d "$DestFolder/data/switch_spec" ]
  then
    echo "check ok"
  else
    recoveryDataPartition "$DestFolder/system/vendor/speccfg/spec"
  fi
else
  SourceFolder="$1"
  DestFolder="$2"
  BasePath="$3"
  LocalFlag="$4"
  echo "SourceFolder=$SourceFolder DestFolder=$DestFolder BasePath=$BasePath LocalFlag=$LocalFlag"
  RmFlag="0"
  SwitchApp="$DestFolder/data/data/com.qualcomm.qti.carrierconfigure"
  SwitchData="$DestFolder/data/data/com.qualcomm.qti.loadcarrier"

  if [ "$DestFolder" != "" ]
  then
    mode="compiling"
  else
    mode="running"
  fi

  if [ -d "$SourceFolder/$strExcludeFolder" ]
  then
    echo "no need to create excludefolder"
  else
    mkdir "$SourceFolder/$strExcludeFolder"
    chmod 770 "$SourceFolder/$strExcludeFolder"
  fi
  if [ "$#" -gt "4" ]
  then
    newSpecList="$5"
    echo "switchToSpec=${newSpecList[0]}"
    if [ "$#" -gt "5" ]
    then
      RmFlag="$6"
    fi
  else
    newSpecList=(`getActionSpecList $SwitchApp`)
    if [ -f "$SwitchApp/cache/rmflag" ]
    then
      RmFlag="1"
    fi
  fi

  getCurrentCarrier "$LocalFlag"

  if [ "${#currentSpec}" -eq "0" ]
  then
    echo "No find carrier, but need to install Default"
    installFolderFunc "$SourceFolder/Default" "$DestFolder"
    currentSpec="Default"
  fi

  uninstallOldSpecList "$LocalFlag"

  if [ "$RmFlag" -eq "1" ]
  then
    cleanOldSpecs "$SwitchData" "${newSpecList[*]}"
  fi

  if [ "${#newSpecList[@]}" -ge "1" ]
  then
    installNewSpecList "$SwitchData" "${newSpecList[*]}"
  fi

  chmod 644 "$LocalFlag"
  chmod 755 "$DestFolder/system/vendor/speccfg"
fi

makeFlagFolder

######Main function end######
