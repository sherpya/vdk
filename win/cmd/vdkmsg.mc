;/*
;   vdkmsg.h
;
;   Virtual Disk Driver control routines error message
;   Copyright (C) 2003 Ken Kato
;*/
;
;#ifndef _VDKMSG_H_
;#define _VDKMSG_H_
;

MessageIdTypedef=DWORD
LanguageNames=(English=0x409:msg0409)
LanguageNames=(Japanese=0x411:msg0411)

;
;//
;//     Parameter Error Message
;//
;
MessageId=
SymbolicName=MSG_MUST_BE_ADMIN
Language=English
You must have administrative privileges to use vdk.
.
Language=Japanese
VDK を使用するには Administrator 権限が必要です。
.


MessageId=
SymbolicName=MSG_UNKNOWN_COMMAND
Language=English
Unknown command '%1!s!' is used.
Try "VDK.EXE HELP" for available commands.
.
Language=Japanese
不明なコマンド '%1!s!' が使用されました。
"VDK.EXE HELP" で利用可能なコマンドを確認してください。
.


MessageId=
SymbolicName=MSG_UNKNOWN_OPTION
Language=English
Unknown option '%1!s!' is used.
Try "VDK.EXE HELP %2!s!" for available options.
.
Language=Japanese
不明なオプション '%1!s!' が使用されました。
"VDK.EXE HELP %2!s!" で利用可能なオプションを確認してください。
.


MessageId=
SymbolicName=MSG_DUPLICATE_ARGS
Language=English
Paramter %1!s! is specified more than once.
.
Language=Japanese
パラメータ %1!s! が二度以上指定されています。
.


MessageId=
SymbolicName=MSG_OPENMODE_OPTION
Language=English
More than one of /RW, /WB and /UNDO are used.
.
Language=Japanese
/RW、/WB、/UNDO のいずれかが重複しています。
.


MessageId=
SymbolicName=MSG_INVALID_DISK
Language=English
Invalid target disk number '%1!s!' is specified.
.
Language=Japanese
不正なディスク番号 '%1!s! が指定されました。
.


MessageId=
SymbolicName=MSG_INVALID_PART
Language=English
Invalid target partition number '%1!s!' is specified.
.
Language=Japanese
不正なパーティション番号 '%1!s!' が指定されました。
.


MessageId=
SymbolicName=MSG_INVALID_DISKNUM
Language=English
Invalid disk device number '%1!s!' is specified.
The value must be between 1 and 22.
.
Language=Japanese
不正なディスクデバイス数 '%1!s!' が指定されました。
デバイス数は 1 から 22 までの数を指定してください。
.


;
;//
;//     Input Prompt Message
;//
;
MessageId=
SymbolicName=MSG_PROMPT_RETRY
Language=English
Try again (y/n) ? %0
.
Language=Japanese
再試行しますか (y/n) ? %0
.


MessageId=
SymbolicName=MSG_PROMPT_CONTINUE
Language=English
Continue (y/n) ? %0
.
Language=Japanese
続行しますか (y/n) ? %0
.


MessageId=
SymbolicName=MSG_PROMPT_ABORTIGNORE
Language=English
A) abort / I) ignore ? %0
.
Language=Japanese
A) 中止 / I) 無視 ? %0
.


MessageId=
SymbolicName=MSG_PROMPT_YESNO
Language=English
Y) yes / N) no ? %0
.
Language=Japanese
Y) はい / N) いいえ ? %0
.


MessageId=
SymbolicName=MSG_PROMPT_PATH
Language=English
Input correct path : %0
.
Language=Japanese
正しいパスを入力してください : %0
.


;
;//
;//     Operation Result Message
;//
;
MessageId=
SymbolicName=MSG_INSTALL_OK
Language=English
Installed the Virtual Disk Driver.
.
Language=Japanese
Virtual Disk ドライバをインストールしました。
.


MessageId=
SymbolicName=MSG_INSTALL_NG
Language=English
Failed to install the Virtual Disk Driver.
.
Language=Japanese
Virtual Disk ドライバをインストールできません。
.


MessageId=
SymbolicName=MSG_REMOVE_OK
Language=English
Uninstalled the Virtual Disk Driver.
.
Language=Japanese
Virtual Disk ドライバをアンインストールしました。
.


MessageId=
SymbolicName=MSG_REMOVE_NG
Language=English
Failed to uninstall the Virtual Disk driver.
.
Language=Japanese
Virtual Disk ドライバをアンインストールできません。
.


MessageId=
SymbolicName=MSG_START_OK
Language=English
Started the Virtual Disk Driver.
.
Language=Japanese
Virtual Disk ドライバを開始しました。
.


MessageId=
SymbolicName=MSG_START_NG
Language=English
Failed to start the Virtual Disk Driver.
.
Language=Japanese
Virtual Disk ドライバを開始できません。
.


MessageId=
SymbolicName=MSG_STOP_OK
Language=English
Stopped the Virtual Disk Driver.
.
Language=Japanese
Virtual Disk ドライバを停止しました。
.


MessageId=
SymbolicName=MSG_STOP_NG
Language=English
Failed to stop the Virtual Disk Driver.
.
Language=Japanese
Virtual Disk ドライバを停止できません。
.


MessageId=
SymbolicName=MSG_DISKNUM_OK
Language=English
The number of virtual disk device is set to %1!lu!.
It will take effect the next time the driver is started.
.
Language=Japanese
仮想ディスクデバイス数を %1!lu! に設定しました。
次回ドライバを開始したときから有効になります。
.


MessageId=
SymbolicName=MSG_DISKNUM_NG
Language=English
Failed to set the number of virtual disk device.
.
Language=Japanese
仮想ディスクデバイス数を設定できません。
.


MessageId=
SymbolicName=MSG_CREATEDISK_OK
Language=English
Created the virtual disk device %1!lu!.
.
Language=Japanese
仮想ディスクデバイス %1!lu! を作成しました。
.


MessageId=
SymbolicName=MSG_CREATEDISK_NG
Language=English
Failed to create the virtual disk device %1!lu!.
.
Language=Japanese
仮想ディスクデバイス %1!lu! を作成できません。
.


MessageId=
SymbolicName=MSG_DELETEDISK_OK
Language=English
Deleted the virtual disk device %1!lu!.
.
Language=Japanese
仮想ディスクデバイス %1!lu! を削除しました。
.


MessageId=
SymbolicName=MSG_DELETEDISK_NG
Language=English
Failed to delete the virtual disk device %1!lu!.
.
Language=Japanese
仮想ディスクデバイス %1!lu! を削除できません。
.


MessageId=
SymbolicName=MSG_OPENIMAGE_NG
Language=English
Failed to open the virtual disk image.
.
Language=Japanese
仮想ディスクイメージをオープンできません。
.


MessageId=
SymbolicName=MSG_CLOSING_DISK
Language=English
Closing the image on the virtual disk %1!lu!...%0
.
Language=Japanese
イメージをクローズしています：ディスク %1!lu!...%0
.


MessageId=
SymbolicName=MSG_CLOSE_OK
Language=English
The image is closed.
.
Language=Japanese
クローズしました。
.


MessageId=
SymbolicName=MSG_DEVICE_EMPTY
Language=English
The drive is empty.
.
Language=Japanese
イメージはありません。
.


MessageId=
SymbolicName=MSG_DEVICE_BUSY
Language=English
The device is busy.
.
Language=Japanese
現在使用不可です。
.


MessageId=
SymbolicName=MSG_LINK_OK
Language=English
Assigned the drive letter '%1!c!' to disk %2!lu! partition %3!lu!.
.
Language=Japanese
ドライブ文字 '%1!c!' をディスク %2!lu! パーティション %3!lu! に割り当てました。
.


MessageId=
SymbolicName=MSG_LINK_NG
Language=English
Failed to assign the drive letter '%1!c!' to disk %2!lu! partition %3!lu!.
.
Language=Japanese
ドライブ文字 '%1!c!' をディスク %2!lu! パーティション %3!lu! に割り当てることができません。
.


MessageId=
SymbolicName=MSG_UNLINK_OK
Language=English
Removed the drive letter '%1!c!'.
.
Language=Japanese
ドライブ文字 '%1!c!' を削除しました。
.


MessageId=
SymbolicName=MSG_UNLINK_NG
Language=English
Failed to remove the drive letter '%1!c!'.
.
Language=Japanese
ドライブ文字 '%1!c!' を削除できません。
.


MessageId=
SymbolicName=MSG_GET_IMAGE_NG
Language=English
Failed to get image information from disk '%1!lu!'.
.
Language=Japanese
ディスク %1!lu! のイメージ情報を取得できません。
.


MessageId=
SymbolicName=MSG_GET_STAT_NG
Language=English
Failed to get the driver status.
.
Language=Japanese
ドライバの状態を取得できません。
.


MessageId=
SymbolicName=MSG_GET_CONFIG_NG
Language=English
Failed to get the driver configuration.
.
Language=Japanese
ドライバの設定を取得できません。
.


MessageId=
SymbolicName=MSG_GET_LINK_NG
Language=English
Failed to get the drive letter of disk %1!lu! partition %2!lu!.
.
Language=Japanese
ディスク %1!lu! パーティション %2!lu! のドライブ文字を取得できません。
.


MessageId=
SymbolicName=MSG_RESOLVE_LINK_NG
Language=English
Failed to get the target disk of drive letter %1!c!.
.
Language=Japanese
ドライブ文字 %1!lu! の参照先ディスクを取得できません。
.


MessageId=
SymbolicName=MSG_DISMOUNT_NG
Language=English
Failed to dismount a partition.
.
Language=Japanese
パーティションをディスマウントできません。
.


MessageId=
SymbolicName=MSG_CLOSE_FORCED
Language=English
The operation is forced to proceed.
.
Language=Japanese
イメージは強制的にクローズされます。
.


MessageId=
SymbolicName=MSG_CREATEREDO_NG
Language=English
Failed to create a REDO log.
.
Language=Japanese
REDO ログを作成できません。
.


;
;//
;//     Additional information about operation failure
;//
;
MessageId=
SymbolicName=MSG_DRIVE_IN_USE
Language=English
Some programs may be using the virtual drive.
.
Language=Japanese
仮想ドライブが使用中の可能性があります。
.


MessageId=
SymbolicName=MSG_REMOVE_PENDING
Language=English
The Virtual Disk Driver will be uninstalled on the next system start up.
You may need to restart the system before reinstalling the driver.
.
Language=Japanese
Virtual Disk ドライバは次回のシステム起動時にアンインストールされます。
再インストールする前にシステムを再起動する必要があるかもしれません。
.


MessageId=
SymbolicName=MSG_STOP_PENDING
Language=English
Stop operation has succeeded, but something
is preventing the driver from actually stopping.
.
Language=Japanese
停止処理は成功しましたが、何らかの理由によりドライバの停止が保留されています。
.


MessageId=
SymbolicName=MSG_NO_MORE_DISK
Language=English
Cannot create any more disk device.
.
Language=Japanese
これ以上ディスクデバイスを作成することはできません。
.


MessageId=
SymbolicName=MSG_NO_LESS_DISK
Language=English
Cannot delete the disk device 0.
.
Language=Japanese
ディスクデバイス 0 を削除することはできません。
.


MessageId=
SymbolicName=MSG_GET_PART_NG
Language=English
Failed to read partition table.
.
Language=Japanese
パーティションテーブルを読み取れません。
.


MessageId=
SymbolicName=MSG_FORCING_WARN
Language=English
Although you can force to close the image, it may lead to loss of
data or an unexpected behavior of the operating system.
.
Language=Japanese
イメージを強制的にクローズすることはできますが、データが失われたり
オペレーティングシステムが予期しない動作をしたりする可能性があります。
.


MessageId=
SymbolicName=MSG_RESTART_WARN
Language=English
You may need to reboot the system before restarting the driver.
.
Language=Japanese
ドライバを再起動する前にシステムを再起動する必要があるかもしれません。
.


MessageId=
SymbolicName=MSG_ALREADY_INSTALLED
Language=English
The Virtual Disk Driver is already installed.
.
Language=Japanese
Virtual Disk ドライバはすでにインストールされています。
.


MessageId=
SymbolicName=MSG_NOT_INSTALLED
Language=English
The Virtual Disk Driver is not installed.
.
Language=Japanese
Virtual Disk ドライバはインストールされていません。
.


MessageId=
SymbolicName=MSG_ALREADY_RUNNING
Language=English
The Virtual Disk Driver is already running.
.
Language=Japanese
Virtual Disk ドライバはすでに開始されています。
.


MessageId=
SymbolicName=MSG_NOT_RUNNING
Language=English
The Virtual Disk Driver is not running.
.
Language=Japanese
Virtual Disk ドライバは開始されていません。
.


MessageId=
SymbolicName=MSG_ALREADY_OPENED
Language=English
Another Virtual Disk image is already opened.
.
Language=Japanese
すでに別の仮想ディスクイメージがオープンされています。
.


MessageId=
SymbolicName=MSG_DRIVE_FULL
Language=English
No drive letter is available for the Virtual Disk drive.
.
Language=Japanese
未使用のドライブ文字がありません。
.


;
;//
;//     driver status information message
;//
;
MessageId=
SymbolicName=MSG_CURRENT_STATUS
Language=English
Driver Status   : %0
.
Language=Japanese
ドライバの状態  : %0
.



MessageId=
SymbolicName=MSG_STATUS_NOT_INST
Language=English
NOT INSTALLED
.
Language=Japanese
インストールされていません
.


MessageId=
SymbolicName=MSG_STATUS_STOPPED
Language=English
STOPPED
.
Language=Japanese
停止
.


MessageId=
SymbolicName=MSG_STATUS_START_P
Language=English
START_PENDING
.
Language=Japanese
開始処理中
.


MessageId=
SymbolicName=MSG_STATUS_STOP_P
Language=English
STOP_PENDING
.
Language=Japanese
停止処理中
.


MessageId=
SymbolicName=MSG_STATUS_RUNNING
Language=English
RUNNING
.
Language=Japanese
実行中
.


MessageId=
SymbolicName=MSG_STATUS_CONT_P
Language=English
CONTINUE_PENDING
.
Language=Japanese
再開処理中
.


MessageId=
SymbolicName=MSG_STATUS_PAUSE_P
Language=English
PAUSE_PENDING
.
Language=Japanese
一時停止処理中
.


MessageId=
SymbolicName=MSG_STATUS_PAUSED
Language=English
PAUSED
.
Language=Japanese
一時停止
.


MessageId=
SymbolicName=MSG_UNKNOWN_ULONG
Language=English
Unknown (0x%1!08x!)
.
Language=Japanese
不明 (0x%1!08x!)
.


;
;//
;//     driver configuration message
;//
;
MessageId=
SymbolicName=MSG_DRIVER_LOCATION
Language=English
Driver File     : %1!s!
.
Language=Japanese
ドライバファイル: %1!s!
.


MessageId=
SymbolicName=MSG_DRIVER_VERSION
Language=English
Driver Version  : %1!u!.%2!u!%3!s!
.
Language=Japanese
バージョン      : %1!u!.%2!u!%3!s!
.


MessageId=
SymbolicName=MSG_START_TYPE
Language=English
Start Type      : %0
.
Language=Japanese
開始方法        : %0
.


MessageId=
SymbolicName=MSG_START_AUTO
Language=English
AUTO
.
Language=Japanese
自動
.


MessageId=
SymbolicName=MSG_START_BOOT
Language=English
BOOT
.
Language=Japanese
ブート
.


MessageId=
SymbolicName=MSG_START_DEMAND
Language=English
DEMAND
.
Language=Japanese
手動
.


MessageId=
SymbolicName=MSG_START_DISABLED
Language=English
DISABLED
.
Language=Japanese
無効
.


MessageId=
SymbolicName=MSG_START_SYSTEM
Language=English
SYSTEM
.
Language=Japanese
システム
.


MessageId=
SymbolicName=MSG_DISK_DEVICE
Language=English
Number of Disks : %1!lu!
.
Language=Japanese
ﾃﾞｨｽｸﾃﾞﾊﾞｲｽ数   : %1!lu!
.


MessageId=
SymbolicName=MSG_ATTACHED_PART
Language=English
Attached Parts  : %1!lu!
.
Language=Japanese
使用ﾊﾟｰﾃｨｼｮﾝ数  : %1!lu!
.


MessageId=
SymbolicName=MSG_ORPHANED_PART
Language=English
Orphaned Parts  : %1!lu!
.
Language=Japanese
未使用ﾊﾟｰﾃｨｼｮﾝ  : %1!lu!
.


MessageId=
SymbolicName=MSG_REFERENCE_COUNT
Language=English
Reference Count : %1!lu!
.
Language=Japanese
リファレンス数  : %1!lu!
.


MessageId=
SymbolicName=MSG_VIRTUAL_DISK
Language=English
Virtual Disk %1!lu!
.
Language=Japanese
仮想ディスク %1!lu!
.


;
;//
;//     Image file information message
;//
;
MessageId=
SymbolicName=MSG_DISKIMAGE_NAME
Language=English
Image Name      : %1!s!
.
Language=Japanese
イメージ名称    : %1!s!
.


MessageId=
SymbolicName=MSG_IMAGE_NONE
Language=English
Image File      : none
.
Language=Japanese
イメージファイル: なし
.


MessageId=
SymbolicName=MSG_ACCESS_TYPE
Language=English
Access Type     : %0
.
Language=Japanese
アクセス種別    : %0
.


MessageId=
SymbolicName=MSG_ACCESS_RO
Language=English
Read-Only
.
Language=Japanese
読み取り専用
.


MessageId=
SymbolicName=MSG_ACCESS_RW
Language=English
Writable
.
Language=Japanese
書き込み可能
.


MessageId=
SymbolicName=MSG_ACCESS_WB
Language=English
Write-Blocked
.
Language=Japanese
書き込みブロック
.


MessageId=
SymbolicName=MSG_DISK_CAPACITY
Language=English
Disk Capacity   : %1!lu! sectors (%2!lu! MB)
.
Language=Japanese
ディスク容量    : %1!lu! セクタ (%2!lu! MB)
.


MessageId=
SymbolicName=MSG_DISK_GEOMETRY
Language=English
Geometry        : (C) %1!lu! * (H) %2!lu! * (S) %3!lu!
.
Language=Japanese
ジオメトリ      : (C) %1!lu! * (H) %2!lu! * (S) %3!lu!
.


MessageId=
SymbolicName=MSG_DISK_FILES
Language=English
Number Of Files : %1!lu!
.
Language=Japanese
総ファイル数    : %1!lu!
.


MessageId=
SymbolicName=MSG_FILE_HEADER
Language=English

  Type     Size    Path
.
Language=Japanese

  種別     サイズ  パス
.


MessageId=
SymbolicName=MSG_PARTITION_HEADER
Language=English
Partitions      :
      #   Start Sector    Length in sectors    Type
     --   ------------  ---------------------  ----
.
Language=Japanese
パーティション  :
      #    開始セクタ          サイズ          種別
     --   ------------  ---------------------  ----
.


MessageId=
SymbolicName=MSG_PARTITION_NONE
Language=English
Partitions      : none
.
Language=Japanese
パーティション  : なし
.


;
;//
;//     VDisk callback message
;//
;
MessageId=
SymbolicName=MSG_CB_FILE_OPEN
Language=English
Failed to open file '%1!s!'.
%2!s!
.
Language=Japanese
ファイル '%1!s!' を開けません。
%2!s!
.


MessageId=
SymbolicName=MSG_CB_FILE_TYPE
Language=English
Failed to decide type of '%1!s!'.
Open as a simple sector image file.
.
Language=Japanese
'%1!s!' ファイル種別が判別できませんでした。
単純なセクタイメージファイルとして開きます。
.


MessageId=
SymbolicName=MSG_CB_EMPTY_IMAGE
Language=English
No data file entry in '%1!s!'.
.
Language=Japanese
'%1!s!' データファイルが含まれていません。
.


MessageId=
SymbolicName=MSG_CB_SIZE_BOUNDARY
Language=English
'%1!s!' File size (%2!s! bytes)
is not a multiple of sector size.
The surplus area (%3!lu! bytes) will not be used.
.
Language=Japanese
'%1!s!' ファイルサイズ（%2!s! バイト）が
セクタサイズの倍数ではありません。
半端な領域 (%3!lu! バイト) は使用されません。
.


MessageId=
SymbolicName=MSG_CB_SIGNATURE
Language=English
'%1!s!' Invalid signature.
> 0x%2!08lx!
.
Language=Japanese
'%1!s!' 不正なシグネチャです。
> 0x%2!08lx!
.


MessageId=
SymbolicName=MSG_CB_CONTROLLER
Language=English
'%1!s!' Unknown controller type.
> %2!s!
.
Language=Japanese
'%1!s!' 未知のコントローラ種別です。
> %2!s!
.


MessageId=
SymbolicName=MSG_PROMPT_CONTROLLER
Language=English
I) ide or S) scsi or C) cancel ? %0
.
Language=Japanese
I) ide  S) scsi  C) cancel ? %0
.


MessageId=
SymbolicName=MSG_CB_HARDWAREVER
Language=English
'%1!s!' Unknown virtual hardware version.
> %2!s!
.
Language=Japanese
'%1!s!' 未知の仮想ハードウェアバージョンです。
> %2!s!
.


MessageId=
SymbolicName=MSG_CB_DESC_BADENTRY
Language=English
'%1!s!' Invalid description entry.
> %2!s!
.
Language=Japanese
'%1!s!' 不正なエントリが含まれています。
> %2!s!
.


MessageId=
SymbolicName=MSG_CB_DESC_OFFSET
Language=English
'%1!s!' Invalid extent offset.
> %2!s!
.
Language=Japanese
'%1!s!' 不正なデータファイルオフセットです。
> %2!s!
.


MessageId=
SymbolicName=MSG_CB_DESC_CAPACITY
Language=English
'%1!s!' Invalid extent capacity.
> '%2!s!'
.
Language=Japanese
'%1!s!' 不正なデータファイル容量です。
> '%2!s!'
.


MessageId=
SymbolicName=MSG_CB_DESC_GEOMETRY
Language=English
'%1!s!' Invalid geometry entry.
> '%2!s!'
.
Language=Japanese
'%1!s!' 不正なジオメトリパラメータです。
> '%2!s!'
.


MessageId=
SymbolicName=MSG_CB_DESC_FILETYPE
Language=English
'%1!s!' Unknown extent type.
> '%2!s!'
.
Language=Japanese
'%1!s!' 未知のデータファイル種別です。
> '%2!s!'
.


MessageId=
SymbolicName=MSG_CB_DESC_TIMESTAMP
Language=English
'%1!s!' Invalid timestamp entry.
> '%2!s!'
.
Language=Japanese
'%1!s!' 不正なタイムスタンプです。
> '%2!s!'
.


MessageId=
SymbolicName=MSG_CB_DESC_DISKTYPE
Language=English
'%1!s!' Unknown virtual disk type.
> '%2!s!'
.
Language=Japanese
'%1!s!' 未知の仮想ディスク種別です。
> '%2!s!'
.


MessageId=
SymbolicName=MSG_CB_EXT_OFFSET
Language=English
'%1!s!' Described offset %2!lu! does not correspond to the actual position %3!lu!.  The actual posision is used.
.
Language=Japanese
'%1!s!' 定義されたオフセット %2!lu! が実際のデータファイル位置 %3!lu! と一致しません。実際の位置を使用します。
.


MessageId=
SymbolicName=MSG_CB_EXT_FILESIZE
Language=English
'%1!s!' Described capacity %2!lu! does not match the actual file size %3!lu!.  The actual size is used.
.
Language=Japanese
'%1!s!' 定義された容量 %2!lu! が実際のファイルサイズ %3!lu! と一致しません。実際のサイズを使用します。
.


MessageId=
SymbolicName=MSG_CB_EXT_CAPACITY
Language=English
'%1!s!' Described disk capacity %2!lu! does not match the actuall total of all extents %3!lu!.
.
Language=Japanese
'%1!s!' 定義されたディスク容量 %2!lu! がデータファイルの実際の合計 %3!lu! と一致しません。
.


MessageId=
SymbolicName=MSG_CB_COWD_ORDINAL
Language=English
'%1!s!' Extent ordinal stored in the header %2!lu! does not match the actual order %3!lu!.  The actual order is used.
.
Language=Japanese
'%1!s!' ヘッダに格納されたファイル順序 %2!lu! がファイル名の順序 %3!lu! と一致しません。実際の順序が優先されます。
.


MessageId=
SymbolicName=MSG_CB_CONF_FILEVER
Language=English
COWdisk version mismatch.
    %1!s! -> %2!lu!
    %3!s! -> %4!lu!
.
Language=Japanese
COWdisk バージョンが一致しません。
    %1!s! -> %2!lu!
    %3!s! -> %4!lu!
.


MessageId=
SymbolicName=MSG_CB_CONF_FLAGS
Language=English
General flags mismatch.
    %1!s! -> 0x%2!08lx!
    %3!s! -> 0x%4!08lx!
.
Language=Japanese
全般フラグが一致しません。
    %1!s! -> 0x%2!08lx!
    %3!s! -> 0x%4!08lx!
.


MessageId=
SymbolicName=MSG_CB_CONF_PARENTTS
Language=English
Parent timestamp mismatch.
    %1!s! -> 0x%2!08lx!
    %3!s! -> 0x%4!08lx!
.
Language=Japanese
親タイムスタンプが一致しません。
    %1!s! -> 0x%2!08lx!
    %3!s! -> 0x%4!08lx!
.


MessageId=
SymbolicName=MSG_CB_CONF_TIMESTAMP
Language=English
Timestamp mismatch.
    %1!s! -> 0x%2!08lx!
    %3!s! -> 0x%4!08lx!
.
Language=Japanese
タイムスタンプが一致しません。
    %1!s! -> 0x%2!08lx!
    %3!s! -> 0x%4!08lx!
.


MessageId=
SymbolicName=MSG_CB_CONF_CONTROLLER
Language=English
Controller type mismatch.
    %1!s! -> %2!s!
    %3!s! -> %4!s!
.
Language=Japanese
コントローラ種別が一致しません。
    %1!s! -> %2!s!
    %3!s! -> %4!s!
.


MessageId=
SymbolicName=MSG_CB_CONF_EXTENTS
Language=English
Number of extents mismatch.
    %1!s! -> %2!lu!
    %3!s! -> %4!lu!
.
Language=Japanese
分割ファイル数が一致しません。
    %1!s! -> %2!lu!
    %3!s! -> %4!lu!
.


MessageId=
SymbolicName=MSG_CB_CONF_CYLINDERS
Language=English
Geometry (cylinders) mismatch.
    %1!s! -> %2!lu!
    %3!s! -> %4!lu!
.
Language=Japanese
ジオメトリ（シリンダ）が一致しません。
    %1!s! -> %2!lu!
    %3!s! -> %4!lu!
.


MessageId=
SymbolicName=MSG_CB_CONF_TRACKS
Language=English
Geometry (tracks) mismatch.
    %1!s! -> %2!lu!
    %3!s! -> %4!lu!
.
Language=Japanese
ジオメトリ（トラック）が一致しません。
    %1!s! -> %2!lu!
    %3!s! -> %4!lu!
.


MessageId=
SymbolicName=MSG_CB_CONF_SECTORS
Language=English
Geometry (sectors) mismatch.
    %1!s! -> %2!lu!
    %3!s! -> %4!lu!
.
Language=Japanese
ジオメトリ（セクタ）が一致しません。
    %1!s! -> %2!lu!
    %3!s! -> %4!lu!
.


MessageId=
SymbolicName=MSG_CB_CONF_CAPACITY
Language=English
Disk capacity mismatch.
    %1!s! -> %2!lu!
    %3!s! -> %4!lu!
.
Language=Japanese
ディスク容量が一致しません。
    %1!s! -> %2!lu!
    %3!s! -> %4!lu!
.


MessageId=
SymbolicName=MSG_CB_CONF_HARDWARE
Language=English
Virtual hardware version mismatch.
    %1!s! -> %2!lu!
    %3!s! -> %4!lu!
.
Language=Japanese
仮想ハードウェアバージョンが一致しません。
    %1!s! -> %2!lu!
    %3!s! -> %4!lu!
.


MessageId=
SymbolicName=MSG_CB_CONF_TOOLSFLAG
Language=English
Tools flags mismatch.
    %1!s! -> 0x%2!08lx!
    %3!s! -> 0x%4!08lx!
.
Language=Japanese
Tools フラグが一致しません。
    %1!s! -> 0x%2!08lx!
    %3!s! -> 0x%4!08lx!
.


MessageId=
SymbolicName=MSG_CB_CONF_SEQNUM
Language=English
Sequence number mismatch.
    %1!s! -> %2!lu!
    %3!s! -> %4!lu!
.
Language=Japanese
シーケンスナンバーが一致しません。
    %1!s! -> %2!lu!
    %3!s! -> %4!lu!
.


MessageId=
SymbolicName=MSG_CB_CONF_PARENTPATH
Language=English
Parent path mismatch.
    %1!s! -> %2!s!
    %3!s! -> %4!s!
.
Language=Japanese
親ディスクパスが一致しません。
    %1!s! -> %2!s!
    %3!s! -> %4!s!
.


MessageId=
SymbolicName=MSG_CB_COWD_CAPACITY
Language=English
'%1!s!' Disk capacity stored in the header %2!lu! does not match the total of all files %3!lu!.
.
Language=Japanese
'%1!s!' ヘッダに格納されたディスク容量 %2!lu! が全ファイル容量の合計 %3!lu! と一致しません。
.


MessageId=
SymbolicName=MSG_CB_COWD_FILEVER
Language=English
'%1!s!' Unknown COWDisk version.
> %2!lu!
.
Language=Japanese
'%1!s!' 未知の COWDisk バージョンです。
> %2!lu!
.


MessageId=
SymbolicName=MSG_CB_COWD_GEOMETRY
Language=English
'%1!s!' Invalid geometry values:
> (C) %2!lu! / (H) %3!lu! / (S) %4!lu! -> %5!lu!
.
Language=Japanese
'%1!s!' 不正なジオメトリパラメータです:
> (C) %2!lu! / (H) %3!lu! / (S) %4!lu! -> %5!lu!
.


MessageId=
SymbolicName=MSG_CB_COWD_PARENT
Language=English
'%1!s!' Parent path is too long.
> %2!s!
.
Language=Japanese
'%1!s!' 親ディスクパスが長すぎます。
> '%2!s!'
.


MessageId=
SymbolicName=MSG_CB_COWD_MAPSIZE
Language=English
'%1!s!' Invalid primary map size %2!lu!. Correct value is %3!lu!.
.
Language=Japanese
'%1!s!' 一次マップのサイズが不正です %2!lu!。正しい値は %3!lu! です。
.


MessageId=
SymbolicName=MSG_CB_COWD_ENDOFFILE
Language=English
'%1!s!' File size stored in the header %2!lu! does not match the actual file size %3!lu!.  The actual size is used.
.
Language=Japanese
'%1!s!' ヘッダに格納されたファイルサイズ %2!lu! が実際のサイズ %3!lu! と一致しません。実際のサイズが優先されます。
.


MessageId=
SymbolicName=MSG_CB_COWD_TIMESTAMP
Language=English
'%1!s!' Timestamps in the header does not match (0x%2!08lx! <> 0x%3!08lx!).
.
Language=Japanese
'%1!s!' ヘッダ内のタイムスタンプが一致しません (0x%2!08lx! <> 0x%3!08lx!)。
.


MessageId=
SymbolicName=MSG_CB_VMDK_NODESC
Language=English
'%1!s!' does not contain a virtual disk description.
.
Language=Japanese
'%1!s!' 仮想ディスク定義が見つかりません。
.


MessageId=
SymbolicName=MSG_CB_VMDK_FILEVER
Language=English
'%1!s!' Unknown VMDK version.
> %2!lu!
.
Language=Japanese
'%1!s!' 未知の VMDK バージョンです。
> %2!lu!
.


MessageId=
SymbolicName=MSG_CB_VMDK_FILECAP
Language=English
'%1!s!' Invalid file capacity.
> %2!s!
.
Language=Japanese
'%1!s!' 不正なファイル容量です。
> %2!s!
.


MessageId=
SymbolicName=MSG_CB_VMDK_GRANULARITY
Language=English
'%1!s!' Invalid granularity.
> %2!s!
.
Language=Japanese
'%1!s!' 不正なグレインサイズです。
> %2!s!
.


MessageId=
SymbolicName=MSG_CB_VMDK_DESCOFFSET
Language=English
'%1!s!' Invalid descriptor offset.
> %2!s!
.
Language=Japanese
'%1!s!' 不正な定義情報オフセットです。
> %2!s!
.


MessageId=
SymbolicName=MSG_CB_VMDK_DESCSIZE
Language=English
'%1!s!' Invalid descriptor size.
> %2!s!
.
Language=Japanese
'%1!s!' 不正な定義情報サイズです。
> %2!s!
.


MessageId=
SymbolicName=MSG_CB_VMDK_GTESPERGT
Language=English
'%1!s!' Invalid grain table size.
> %2!lu!
.
Language=Japanese
'%1!s!' 不正なグレインテーブルサイズです。
> %2!lu!
.


MessageId=
SymbolicName=MSG_CB_VMDK_GDOFFSET
Language=English
'%1!s!' Invalid grain directory offset.
> %2!s!
.
Language=Japanese
'%1!s!' 不正なグレインディレクトリオフセットです。
> %2!s!
.

MessageId=
SymbolicName=MSG_CB_VMDK_GRAINOFFSET
Language=English
'%1!s!' Invalid grain offset.
> %2!s!
.
Language=Japanese
'%1!s!' 不正なグレインオフセットです。
> %2!s!
.


MessageId=
SymbolicName=MSG_CB_VMDK_CHECKBYTES
Language=English
'%1!s!' Invalid check data.  The file may have been transfered in text mode.
.
Language=Japanese
'%1!s!' 不正なチェックデータです。ファイルがテキストモードで転送された可能性があります。
.


MessageId=
SymbolicName=MSG_CB_VMDK_SIZEMISMATCH
Language=English
'%1!s!' Capacity in the header %2!lu! does not match the capacity in the descriptor %3!lu!.  The value in the header is used.
.
Language=Japanese
'%1!s!' ヘッダ内のサイズ %2!lu! が定義されたサイズ %3!lu! と一致しません。ヘッダ内の値が優先されます。
.


MessageId=
SymbolicName=MSG_CB_PARENT_CAPACITY
Language=English
Disk capacity doesn't match the parent.
    %1!s! -> %2!lu!
    %3!s! -> %4!lu!
.
Language=Japanese
ディスク容量が親ディスクと一致しません。
    %1!s! -> %2!lu!
    %3!s! -> %4!lu!
.


MessageId=
SymbolicName=MSG_CB_PARENT_CONTROLLER
Language=English
Controller type doesn't match the parent.
    %1!s! -> %2!s!
    %3!s! -> %4!s!
.
Language=Japanese
コントローラ種別が親ディスクと一致しません。
    %1!s! -> %2!s!
    %3!s! -> %4!s!
.


MessageId=
SymbolicName=MSG_CB_PARENT_TIMESTAMP
Language=English
Timestamp doesn't match the parent.
    %1!s! -> 0x%2!08lx!
    %3!s! -> 0x%4!08lx!
.
Language=Japanese
タイムスタンプが親ディスクと一致しません。
    %1!s! -> 0x%2!08lx!
    %3!s! -> 0x%4!08lx!
.


;
;//
;//     Help Message
;//
;
MessageId=
SymbolicName=MSG_HELP_USAGE
Language=English
Usage:  VDK.EXE command [options...]
Try 'VDK.EXE HELP' for more information.
.
Language=Japanese
構文：  VDK.EXE コマンド [オプション...]
詳しくは 'VDK.EXE HELP' を使用してください。
.


MessageId=
SymbolicName=MSG_USAGE_INSTALL
Language=English
SYNTAX:   VDK.EXE INSTALL [driver] [/AUTO]
Try 'VDK.EXE HELP INSTALL' for more information.
.
Language=Japanese
構文：    VDK.EXE INSTALL [ドライバ] [/AUTO]
詳しくは 'VDK.EXE HELP INSTALL' を使用してください。
.


MessageId=
SymbolicName=MSG_USAGE_REMOVE
Language=English
SYNTAX:   VDK.EXE REMOVE
Try 'VDK.EXE HELP REMOVE' for more information.
.
Language=Japanese
構文：    VDK.EXE REMOVE
詳しくは 'VDK.EXE HELP REMOVE' を使用してください。
.


MessageId=
SymbolicName=MSG_USAGE_START
Language=English
SYNTAX:   VDK.EXE START
Try 'VDK.EXE HELP START' for more information.
.
Language=Japanese
構文：    VDK.EXE START
詳しくは 'VDK.EXE HELP START' を使用してください。
.


MessageId=
SymbolicName=MSG_USAGE_STOP
Language=English
SYNTAX:   VDK.EXE STOP
Try 'VDK.EXE HELP STOP' for more information.
.
Language=Japanese
構文：    VDK.EXE STOP
詳しくは 'VDK.EXE HELP STOP' を使用してください。
.


MessageId=
SymbolicName=MSG_USAGE_DRIVER
Language=English
SYNTAX:   VDK.EXE DRIVER
Try 'VDK.EXE HELP DRIVER' for more information.
.
Language=Japanese
構文：    VDK.EXE DRIVER
詳しくは 'VDK.EXE HELP DRIVER' を使用してください。
.


MessageId=
SymbolicName=MSG_USAGE_DISK
Language=English
SYNTAX:   VDK.EXE DISK number
Try 'VDK.EXE HELP DISK' for more information.
.
Language=Japanese
構文：    VDK.EXE DISK デバイス数
詳しくは 'VDK.EXE HELP DISK' を使用してください。
.


MessageId=
SymbolicName=MSG_USAGE_CREATE
Language=English
SYNTAX:   VDK.EXE CREATE
Try 'VDK.EXE HELP CREATE' for more information.
.
Language=Japanese
構文：    VDK.EXE CREATE
詳しくは 'VDK.EXE HELP CREATE' を使用してください。
.


MessageId=
SymbolicName=MSG_USAGE_DELETE
Language=English
SYNTAX:   VDK.EXE DELETE
Try 'VDK.EXE HELP DELETE' for more information.
.
Language=Japanese
構文：    VDK.EXE DELETE
詳しくは 'VDK.EXE HELP DELETE' を使用してください。
.


MessageId=
SymbolicName=MSG_USAGE_VIEW
Language=English
SYNTAX:   VDK.EXE VIEW path [/SEARCH:path]
Try 'VDK.EXE HELP VIEW' for more information.
.
Language=Japanese
構文：    VDK.EXE VIEW イメージ [/SEARCH:path]
詳しくは 'VDK.EXE HELP VIEW' を使用してください。
.


MessageId=
SymbolicName=MSG_USAGE_OPEN
Language=English
SYNTAX:   VDK.EXE OPEN disk# image [/RW | /WB | /UNDO | /UNDO:path]
              [/SEARCH:path] [/P:part#] [/L:drive]
Try 'VDK.EXE HELP OPEN' for more information.
.
Language=Japanese
構文：    VDK.EXE OPEN ディスク番号 イメージ [/RW | /WB | /UNDO | /UNDO:パス]
              [/SEARCH:パス] [/P:パーティション番号] [/L:ドライブ文字]
詳しくは 'VDK.EXE HELP OPEN' を使用してください。
.


MessageId=
SymbolicName=MSG_USAGE_CLOSE
Language=English
SYNTAX:   VDK.EXE CLOSE disk# [/Q | /F]
          VDK.EXE CLOSE drive [/Q | /F]
Try 'VDK.EXE HELP CLOSE' for more information.
.
Language=Japanese
構文：    VDK.EXE CLOSE ディスク番号 [/Q | /F]
          VDK.EXE CLOSE ドライブ文字 [/Q | /F]
詳しくは 'VDK.EXE HELP CLOSE' を使用してください。
.


MessageId=
SymbolicName=MSG_USAGE_LINK
Language=English
SYNTAX:   VDK.EXE LINK disk# part# [drive]
Try 'VDK.EXE HELP LINK' for more information.
.
Language=Japanese
構文：    VDK.EXE LINK ディスク番号 パーティション番号 [ドライブ文字]
詳しくは 'VDK.EXE HELP LINK' を使用してください。
.


MessageId=
SymbolicName=MSG_USAGE_ULINK
Language=English
SYNTAX:   VDK.EXE ULINK disk# part#
          VDK.EXE ULINK drive
Try 'VDK.EXE HELP ULINK' for more information.
.
Language=Japanese
構文：    VDK.EXE ULINK ディスク番号 パーティション番号
          VDK.EXE ULINK ドライブ文字
詳しくは 'VDK.EXE HELP ULINK' を使用してください。
.


MessageId=
SymbolicName=MSG_USAGE_IMAGE
Language=English
SYNTAX:   VDK.EXE IMAGE [disk#]
          VDK.EXE IMAGE [drive]
Try 'VDK.EXE HELP IMAGE' for more information.
.
Language=Japanese
構文：    VDK.EXE IMAGE [ディスク番号]
          VDK.EXE IMAGE [ドライブ文字]
詳しくは 'VDK.EXE HELP IMAGE' を使用してください。
.


MessageId=
SymbolicName=MSG_USAGE_HELP
Language=English
SYNTAX:   VDK.EXE HELP [command]
Try 'VDK.EXE HELP HELP' for more information.
.
Language=Japanese
構文：    VDK.EXE HELP [コマンド]
詳しくは 'VDK.EXE HELP HELP' を使用してください。
.


MessageId=
SymbolicName=MSG_HELP_GENERAL
Language=English
Usage:
  VDK.EXE command [options...]

Commands:
  INSTALL   Install the Virtual Disk Driver.
  REMOVE    Uninstall the Virtual Disk Driver.
  START     Start the Virtual Disk Driver.
  STOP      Stop the Virtual Disk Driver.
  DRIVER    Print Virtual Disk Driver status.
  DISK      Set the initial number of virtual disk devices.
  CREATE    Create a new virtual disk device.
  DELETE    Delete a virtual disk device.
  VIEW      Print disk image information.
  OPEN      Open a disk image as a virtual drive.
  CLOSE     Close a disk image.
  LINK      Assign a drive letter to a partition.
  ULINK     Remove a drive letter from a partition.
  IMAGE     Print opened image file information.
  HELP      Print command help.

All commands and options are case insensitive.
Try "VDK.EXE HELP command" for detailed help for each command.
.
Language=Japanese
構文：
  VDK.EXE コマンド [オプション...]

コマンド：
  INSTALL   Virtual Disk ドライバをインストールします。
  REMOVE    Virtual Disk ドライバをアンインストールします。
  START     Virtual Disk ドライバを開始します。
  STOP      Virtual Disk ドライバを停止します。
  DRIVER    Virtual Disk ドライバ状態を表示します。
  DISK      初期ディスクデバイス数を設定します。
  CREATE    仮想ディスクデバイスを作成します。
  DELETE    仮想ディスクデバイスを削除します。
  VIEW      イメージファイル情報を表示します。
  OPEN      仮想ディスクイメージをオープンします。
  CLOSE     仮想ディスクイメージをクローズします。
  LINK      仮想パーティションにドライブ文字を割り当てます。
  ULINK     仮想パーティションのドライブ文字を削除します。
  IMAGE     オープン中のイメージファイル情報を表示します。
  HELP      コマンドヘルプを表示します。

コマンドおよびオプションの大文字、小文字は区別されません。
'VDK.EXE HELP <コマンド>' で各コマンドの詳細なヘルプが表示されます。
.


MessageId=
SymbolicName=MSG_HELP_INSTALL
Language=English
Install the Virtual Disk Driver.

SYNTAX:
  VDK.EXE INSTALL [driver] [/AUTO]

OPTIONS:
  driver    Specifies the path to the Virtual Disk Driver file (VDK.SYS).
            Default is VDK.SYS in the same directory as VDK.EXE.
            (Note: *NOT* current working directory.)

  /AUTO     Configures the driver to start at the system startup.
            (Note: this option does not start the driver after installation
            is completed.)
            By default the driver has to be started manually.

Device drivers cannot be started from network drives.
Make sure to place VDK.SYS on a local drive.
.
Language=Japanese
Virtual Disk ドライバをインストールします。

構文：
  VDK.EXE INSTALL [ドライバ] [/AUTO]

オプション：
  ドライバ  Virtual Disk ドライバ（VDK.SYS）のパスを指定します。
            省略時は VDK.EXE と同じディレクトリにある VDK.SYS です。
            （注意：カレントディレクトリではありません。）

  /AUTO     システム起動時に Virtual Disk ドライバを開始するよう設定します。
            （注意：インストール直後に自動で開始されるわけではありません。）
            省略した場合は必要に応じて手動で開始しなければなりません。

デバイスドライバをネットワークドライブから起動することはできません。
VDK.SYS ファイルは必ずローカルドライブに置いてください。
.


MessageId=
SymbolicName=MSG_HELP_REMOVE
Language=English
Uninstall the Virtual Disk Driver.

SYNTAX:
  VDK.EXE REMOVE

OPTIONS:
  NONE

This command removes the Virtual Disk Driver entries from the system
registry, but does not delete the driver file from the drive.
If the driver is running, this command closes all image files and
stops the driver before removing it from the system.
.
Language=Japanese
Virtual Disk ドライバをアンインストールします。

構文：
  VDK.EXE REMOVE

オプション：
  なし

システムレジストリから Virtual Disk ドライバのエントリを削除しますが、
ドライバファイル自体は削除しません。
必要に応じてイメージファイルのクローズとドライバの停止を行います。
.


MessageId=
SymbolicName=MSG_HELP_START
Language=English
Start the Virtual Disk Driver.

SYNTAX:
  VDK.EXE START

OPTIONS:
  NONE

If the driver is not already installed, this command attempts to install it
with the default options.
.
Language=Japanese
Virtual Disk ドライバを開始します。

構文：
  VDK.EXE START

オプション：
  なし

ドライバがインストールされていない場合、デフォルトオプションでのインストール
を試みます。
.


MessageId=
SymbolicName=MSG_HELP_STOP
Language=English
Stop the Virtual Disk Driver.

SYNTAX:
  VDK.EXE STOP

OPTIONS:
  NONE

This command closes all image files before stopping the driver.
The driver cannot be stopped if virtual drives are used by any other programs.
.
Language=Japanese
Virtual Disk ドライバを停止します。

構文：
  VDK.EXE STOP

オプション：
  なし

オープン中のイメージファイルを全てクローズしてからドライバを停止します。
仮想ドライブが他のプログラムから使用されている場合はドライバを停止できません。
.


MessageId=
SymbolicName=MSG_HELP_DRIVER
Language=English
Print Virtual Disk Driver status.

SYNTAX:
  VDK.EXE DRIVER

OPTIONS:
  NONE

This commands prints the following information:
    Driver file path
    Driver file version
    Driver start type (AUTO/MANUAL)
    Current running state
    Number of disk devices
.
Language=Japanese
Virtual Disk ドライバ状態を表示します。

構文：
  VDK.EXE DRIVER

オプション：
  なし

以下の情報を出力します：
    ドライバファイルのパス
    ドライババージョン
    ドライバ開始方法
    現在の実行状態
    仮想ディスクデバイス数
.


MessageId=
SymbolicName=MSG_HELP_DISK
Language=English
Set the initial number of virtual disks.

SYNTAX:
  VDK.EXE DISK number

OPTIONS:
  number    Number of virtual disks between 1 and 22.
            The default value is 4.

This command sets the number of virtual disk devices created when the driver
is started.
If the driver is already running, the value takes effect the next time the
driver is started.
.
Language=Japanese
初期ディスクデバイス数を設定します。

構文：
  VDK.EXE DISK デバイス数

オプション：
  デバイス数  1 から 22 までの仮想ディスクデバイス数を指定します。
              デフォルトは 4。

ドライバ開始時に作成するディスクデバイス数を設定します。
ドライバがすでに稼動中の場合、設定値は次回のドライバ開始時から有効になります。
.


MessageId=
SymbolicName=MSG_HELP_CREATE
Language=English
Create a new Virtual Disk device.

SYNTAX:
  VDK.EXE CREATE

OPTIONS:
  NONE

This commands creates a new virtual disk device.
The created disk has the highest disk number.
.
Language=Japanese
仮想ディスクデバイスを追加します。

構文：
  VDK.EXE CREATE

オプション：
  なし

新たな仮想ディスクデバイスを作成します。
.


MessageId=
SymbolicName=MSG_HELP_DELETE
Language=English
Delete a Virtual Disk device.

SYNTAX:
  VDK.EXE DELETE

OPTIONS:
  NONE

This command deletes a virtual disk device with the highest disk number.
A disk cannot be deleted if an image is opened, or any process is using it.
Disk #0 cannot be deleted (there must exist at least one disk device).
.
Language=Japanese
仮想ディスクデバイスを削除します。

構文：
  VDK.EXE DELETE

オプション：
  なし

一番最後のディスク番号の仮想ディスクデバイスを削除します。
イメージがオープンされている場合、あるいは他のプロセスがデバイスを使用している
場合には削除することができません。
ディスク 0 を削除することはできません。
.


MessageId=
SymbolicName=MSG_HELP_VIEW
Language=English
Print disk image information.

SYNTAX:
  VDK.EXE VIEW image [/SEARCH:path]

OPTIONS:
  image     Path to the image to print the information.

  /SEARCH:path
            Specifies a path to search the image file (and related files).
            The specified path has the highest priority in the searching
            order, and searched even before the explicit paths in the
            command line, descriptor files and virtual disk files.

This command prints the following information for the specified image:

    Virtual disk capacity (in 512 byte sectors)
    Number of files composing the image
    Type, capacity and path of each component file
    Size and type of each partition in the image
.
Language=Japanese
イメージファイル情報を表示します。

構文：
  VDK.EXE VIEW イメージ [/SEARCH:パス]

オプション：
  イメージ  仮想ディスクイメージのパスを指定します。

  /SEARCH:パス
            イメージファイルや関連ファイルを検索するパスを指定します。
            ここで指定したパスは最優先で扱われ、コマンドラインや定義
            ファイル、仮想ディスクファイル内に明示的に記述されている
            パスよりも先に検索されます。

指定された仮想ディスクイメージの以下の情報を表示します：

    仮想ディスクサイズ
    仮想ディスクを構成するファイルの数
    各ファイルの種別、容量、パス
    仮想ディスク上のパーティションのサイズおよびパーティションタイプ
.


MessageId=
SymbolicName=MSG_HELP_OPEN
Language=English
Open a disk image as a virtual drive.

SYNTAX:
  VDK.EXE OPEN disk# image [/RW | /WB | /UNDO | /UNDO:path]
        [/SEARCH:path] [/P:part#] [/L:drive]

OPTIONS:
  disk#     Specifies the virtual disk number.
            This must be the first parameter.
            '*' means the first available disk, and if all existing disk is
            busy a new virtual disk is created.

  image     Specifies the path to the disk image file.
            This must be the second parameter.

  /RW       Open the image in Read-Write mode.

  /WB       Open the image in Write-Block mode.

  /UNDO | /UNDO:path
            Creates a REDO log for the image and open in Read-Write mode.
            If you specify a path, the REDO log is created in the specified
            directory.

  /P:part#  Specifies a partition number to set a drive letter.
            Drive letters can be set/removed later with LINK/ULINK commands.
            By default, drive letters are assigned to all mountable partitions.

  /L:drive  Specifies drive letters to assign to partitions.
            By default, the first available drive letter is used.

  /SEARCH:path
            Specifies a path to search the image file (and related files).
            The specified path has the highest priority in the searching
            order, and searched even before the explicit paths in the
            command line, descriptor files and virtual disk files.

Only one of /RW, /WB and /UNDO can be used at a time.
When none of these is specified, the image is opened in Read-Only mode.
.
Language=Japanese
仮想ディスクイメージをオープンします。

構文：
  VDK.EXE OPEN ディスク番号 イメージ [/RW | /WB | /UNDO | /UNDO:パス]
        [/SEARCH:パス] [/P:パーティション番号] [/L:ドライブ文字]

オプション：
  ディスク番号
            目的の仮想ディスク番号を指定します。
            最初に指定しなければなりません。
            '*' を指定した場合、最初の空きディスクが使用されます。
            空きがない場合は新たなディスクを作成して使用します。

  イメージ  オープンするディスクイメージのパスを指定します。
            二番目に指定しなければなりません。

  /RW       イメージを書き込み可能モードでオープンします。

  /WB       イメージを書き込みブロックモードでオープンします。

  /UNDO | /UNDO:パス
            イメージの REDO ログを作成して書き込み可能モードでオープンします。
            パスを指定した場合、REDO ログは指定ディレクトリに作成されます。

  /P:パーティション番号
            ドライブ文字を割り当てるパーティション番号を指定します。
            後で LINK/ULINK コマンドでドライブ文字を割り当てることもできます。
            省略した場合、マウント可能なパーティション全てに割り当てられます。

  /L:ドライブ文字
            仮想パーティションに割り当てるドライブ文字を指定します。
            省略した場合最初の利用可能なドライブ文字を自動選択します。

  /SEARCH:パス
            イメージファイルや関連ファイルを検索するパスを指定します。
            ここで指定したパスは最優先で扱われ、コマンドラインや定義
            ファイル、仮想ディスクファイル内に明示的に記述されている
            パスよりも先に検索されます。

/RW、/WB、/UNDO はいずれか一つだけを指定することができます。何も指定しない場合
イメージは読み取り専用でオープンされ、仮想ディスクは書き込み不可となります。
.


MessageId=
SymbolicName=MSG_HELP_CLOSE
Language=English
Close a disk image.

SYNTAX:
  VDK.EXE CLOSE disk# [/Q | /F]
  VDK.EXE CLOSE drive [/Q | /F]

OPTIONS:
  disk#     Specifies the target virtual disk number.
            '*' means all existing virtual disks.

  drive     Specifies a drive letter of a partition on the target disk.

  /Q        Suppresses prompting and fails the operation when an error
            has occured.

  /F        Suppresses prompting and forces the image to close even if
            an error has occured.

Make sure that other programs are not using any partitions on the drive
before closing the image.  Generally, the image should not be closed
while the virtual drive is used by any processes.
Although you can force to close the image by answering to do so when
asked or by using the /F option, you should be aware that to forcibly
closing an image may lead to loss of data or unexpected behavior of the
operating system.
.
Language=Japanese
仮想ディスクイメージをクローズします。

構文：
  VDK.EXE CLOSE ディスク番号 [/Q | /F]
  VDK.EXE CLOSE ドライブ文字 [/Q | /F]

オプション：
  ディスク番号  目的の仮想ディスク番号を指定します。
                '*' を指定した場合は全てのディスクのイメージをクローズします。

  ドライブ文字  目的の仮想ディスク上のパーティション（複数ある場合はいずれか
                一つ）のドライブ文字を指定します。

  /Q            エラー発生時にプロンプトを出さずに処理を中断します。

  /F            エラー発生時にプロンプトを出さずに処理を強制続行します。

他のプログラムが仮想ドライブを使用していないことを確認してください。
通常は仮想ドライブが使用中の場合イメージをクローズしないでください。
プロンプト、あるいは /F オプションを使用して強制的にクローズすることも
できますが、その場合データが失われたり、オペレーティングシステムが予期
しない動作をしたりする可能性があります。
.


MessageId=
SymbolicName=MSG_HELP_LINK
Language=English
Assign a drive letter to a partition.

SYNTAX:
  VDK.EXE LINK disk# part# [drive]

OPTIONS:
  disk#     Specifies the target virtual disk number.

  part#     Specifies the target partition number.

  drive     Specifies a drive letter to assign.
            By default, the first available drive letter is used.

You cannot assign more than one drive letters to one partition.
You cannot assign a drive letter already used for another volume.
Use IMAGE command to see partition numbers and the current drive
letter assignment.
.
Language=Japanese
仮想ディスクのパーティションにドライブ文字を割り当てます。

構文：
  VDK.EXE LINK ディスク番号 パーティション番号 [ドライブ文字]

オプション：
  ディスク番号
            目的の仮想ディスク番号を指定します。

  パーティション番号
            目的のパーティション番号を指定します。

  ドライブ文字
            割り当てるドライブ文字を指定します。
            省略した場合、最初の使用可能なドライブ文字が割り当てられます。

一つのパーティションに複数のドライブ文字を割り当てることはできません。
すでに使用されているドライブ文字を割り当てることもできません。
パーティション番号および現在のドライブ文字の割り当ては IMAGE コマンドで
確認することができます。
.




MessageId=
SymbolicName=MSG_HELP_ULINK
Language=English
Remove a drive letter from a partition.

SYNTAX:
  VDK.EXE ULINK disk# part#
  VDK.EXE ULINK drive

OPTIONS:
  disk#     Specifies the target virtual disk number.

  part#     Specifies the target partition number.

  drive     Specifies the drive letter to remove.

This command can remove drive letters of virtual disk drives only.
Use IMAGE command to see partition numbers and the current drive
letter assignment.
.
Language=Japanese
仮想ディスク上のパーティションからドライブ文字を削除します。

構文：
  VDK.EXE ULINK ディスク番号 パーティション番号
  VDK.EXE ULINK ドライブ文字

オプション：
  ディスク番号
            目的の仮想ディスク番号を指定します。

  パーティション番号
            目的のパーティション番号を指定します。

  ドライブ文字
            削除するドライブ文字を指定します。

このコマンドで削除できるのは VDK 仮想ディスクのドライブ文字のみです。
パーティション番号および現在のドライブ文字の割り当ては IMAGE コマンドで
確認することができます。
.


MessageId=
SymbolicName=MSG_HELP_IMAGE
Language=English
Print current virtual disk image information.

SYNTAX:
  VDK.EXE IMAGE [disk#]
  VDK.EXE IMAGE [drive]

OPTIONS:
  disk#     Specifies the target virtual disk number.

  drive     Specifies a drive letter of a partition on the target disk.

This command prints the following information for the target disk:

    Virtual disk access mode
    Virtual disk capacity (in 512 byte sectors)
    Number of files composing the virtual disk
    Type, capacity and path of each component file
    Drive letter, size and type of each partition on the virtual disk

When target is not specified, information for all drives are printed.
.
Language=Japanese
オープン中のディスクイメージ情報を表示します。

構文：
  VDK.EXE IMAGE [ディスク番号]
  VDK.EXE IMAGE [ドライブ文字]

OPTIONS:
  ディスク番号  対象の仮想ディスク番号を指定します。

  ドライブ文字  対象の仮想ディスク上のパーティション（複数ある場合はいずれか
                一つ）のドライブ文字を指定します。

対象の仮想ディスクの以下の情報を表示します：

    仮想ディスクアクセス種別
    仮想ディスクサイズ
    仮想ディスクを構成するファイルの数
    各ファイルの種別、容量、パス
    仮想ディスク上の各パーティションドライブ文字、サイズ、タイプ

対象が指定されなかった場合、全ての仮想ディスクの情報が表示されます。
.


MessageId=
SymbolicName=MSG_HELP_HELP
Language=English
Print VDK.EXE command help.

SYNTAX:
  VDK.EXE HELP [command]

OPTIONS:
  command   Specifies a VDK.EXE command to display help.
            Following commands can be specified:

                INSTALL REMOVE  START   STOP    DRIVER
                DISK    CREATE  DELETE  VIEW    OPEN
                CLOSE   LINK    ULINK   IMAGE   HELP

            If not specified, the general help is printed.
.
Language=Japanese
VDK.EXE のヘルプを表示します。

構文：
  VDK.EXE HELP [コマンド]

オプション：
  コマンド  ヘルプを表示する VDK コマンドを指定します。
            以下のコマンドが使用できます

                INSTALL REMOVE  START   STOP    DRIVER
                DISK    CREATE  DELETE  VIEW    OPEN
                CLOSE   LINK    ULINK   IMAGE   HELP

            省略した場合は概要ヘルプが表示されます。
.


;
;#endif // _VDKMSG_H_
