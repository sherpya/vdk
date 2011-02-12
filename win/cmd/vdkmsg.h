/*
   vdkmsg.h

   Virtual Disk Driver control routines error message
   Copyright (C) 2003 Ken Kato
*/

#ifndef _VDKMSG_H_
#define _VDKMSG_H_


//
//     Parameter Error Message
//

//
//  Values are 32 bit values laid out as follows:
//
//   3 3 2 2 2 2 2 2 2 2 2 2 1 1 1 1 1 1 1 1 1 1
//   1 0 9 8 7 6 5 4 3 2 1 0 9 8 7 6 5 4 3 2 1 0 9 8 7 6 5 4 3 2 1 0
//  +---+-+-+-----------------------+-------------------------------+
//  |Sev|C|R|     Facility          |               Code            |
//  +---+-+-+-----------------------+-------------------------------+
//
//  where
//
//      Sev - is the severity code
//
//          00 - Success
//          01 - Informational
//          10 - Warning
//          11 - Error
//
//      C - is the Customer code flag
//
//      R - is a reserved bit
//
//      Facility - is the facility code
//
//      Code - is the facility's status code
//
//
// Define the facility codes
//


//
// Define the severity codes
//


//
// MessageId: MSG_MUST_BE_ADMIN
//
// MessageText:
//
// You must have administrative privileges to use vdk.
//
#define MSG_MUST_BE_ADMIN                ((DWORD)0x00000001L)

//
// MessageId: MSG_UNKNOWN_COMMAND
//
// MessageText:
//
// Unknown command '%1!s!' is used.
// Try "VDK.EXE HELP" for available commands.
//
#define MSG_UNKNOWN_COMMAND              ((DWORD)0x00000002L)

//
// MessageId: MSG_UNKNOWN_OPTION
//
// MessageText:
//
// Unknown option '%1!s!' is used.
// Try "VDK.EXE HELP %2!s!" for available options.
//
#define MSG_UNKNOWN_OPTION               ((DWORD)0x00000003L)

//
// MessageId: MSG_DUPLICATE_ARGS
//
// MessageText:
//
// Paramter %1!s! is specified more than once.
//
#define MSG_DUPLICATE_ARGS               ((DWORD)0x00000004L)

//
// MessageId: MSG_OPENMODE_OPTION
//
// MessageText:
//
// More than one of /RW, /WB and /UNDO are used.
//
#define MSG_OPENMODE_OPTION              ((DWORD)0x00000005L)

//
// MessageId: MSG_INVALID_DISK
//
// MessageText:
//
// Invalid target disk number '%1!s!' is specified.
//
#define MSG_INVALID_DISK                 ((DWORD)0x00000006L)

//
// MessageId: MSG_INVALID_PART
//
// MessageText:
//
// Invalid target partition number '%1!s!' is specified.
//
#define MSG_INVALID_PART                 ((DWORD)0x00000007L)

//
// MessageId: MSG_INVALID_DISKNUM
//
// MessageText:
//
// Invalid disk device number '%1!s!' is specified.
// The value must be between 1 and 22.
//
#define MSG_INVALID_DISKNUM              ((DWORD)0x00000008L)


//
//     Input Prompt Message
//

//
// MessageId: MSG_PROMPT_RETRY
//
// MessageText:
//
// Try again (y/n) ? %0
//
#define MSG_PROMPT_RETRY                 ((DWORD)0x00000009L)

//
// MessageId: MSG_PROMPT_CONTINUE
//
// MessageText:
//
// Continue (y/n) ? %0
//
#define MSG_PROMPT_CONTINUE              ((DWORD)0x0000000AL)

//
// MessageId: MSG_PROMPT_ABORTIGNORE
//
// MessageText:
//
// A) abort / I) ignore ? %0
//
#define MSG_PROMPT_ABORTIGNORE           ((DWORD)0x0000000BL)

//
// MessageId: MSG_PROMPT_YESNO
//
// MessageText:
//
// Y) yes / N) no ? %0
//
#define MSG_PROMPT_YESNO                 ((DWORD)0x0000000CL)

//
// MessageId: MSG_PROMPT_PATH
//
// MessageText:
//
// Input correct path : %0
//
#define MSG_PROMPT_PATH                  ((DWORD)0x0000000DL)


//
//     Operation Result Message
//

//
// MessageId: MSG_INSTALL_OK
//
// MessageText:
//
// Installed the Virtual Disk Driver.
//
#define MSG_INSTALL_OK                   ((DWORD)0x0000000EL)

//
// MessageId: MSG_INSTALL_NG
//
// MessageText:
//
// Failed to install the Virtual Disk Driver.
//
#define MSG_INSTALL_NG                   ((DWORD)0x0000000FL)

//
// MessageId: MSG_REMOVE_OK
//
// MessageText:
//
// Uninstalled the Virtual Disk Driver.
//
#define MSG_REMOVE_OK                    ((DWORD)0x00000010L)

//
// MessageId: MSG_REMOVE_NG
//
// MessageText:
//
// Failed to uninstall the Virtual Disk driver.
//
#define MSG_REMOVE_NG                    ((DWORD)0x00000011L)

//
// MessageId: MSG_START_OK
//
// MessageText:
//
// Started the Virtual Disk Driver.
//
#define MSG_START_OK                     ((DWORD)0x00000012L)

//
// MessageId: MSG_START_NG
//
// MessageText:
//
// Failed to start the Virtual Disk Driver.
//
#define MSG_START_NG                     ((DWORD)0x00000013L)

//
// MessageId: MSG_STOP_OK
//
// MessageText:
//
// Stopped the Virtual Disk Driver.
//
#define MSG_STOP_OK                      ((DWORD)0x00000014L)

//
// MessageId: MSG_STOP_NG
//
// MessageText:
//
// Failed to stop the Virtual Disk Driver.
//
#define MSG_STOP_NG                      ((DWORD)0x00000015L)

//
// MessageId: MSG_DISKNUM_OK
//
// MessageText:
//
// The number of virtual disk device is set to %1!lu!.
// It will take effect the next time the driver is started.
//
#define MSG_DISKNUM_OK                   ((DWORD)0x00000016L)

//
// MessageId: MSG_DISKNUM_NG
//
// MessageText:
//
// Failed to set the number of virtual disk device.
//
#define MSG_DISKNUM_NG                   ((DWORD)0x00000017L)

//
// MessageId: MSG_CREATEDISK_OK
//
// MessageText:
//
// Created the virtual disk device %1!lu!.
//
#define MSG_CREATEDISK_OK                ((DWORD)0x00000018L)

//
// MessageId: MSG_CREATEDISK_NG
//
// MessageText:
//
// Failed to create the virtual disk device %1!lu!.
//
#define MSG_CREATEDISK_NG                ((DWORD)0x00000019L)

//
// MessageId: MSG_DELETEDISK_OK
//
// MessageText:
//
// Deleted the virtual disk device %1!lu!.
//
#define MSG_DELETEDISK_OK                ((DWORD)0x0000001AL)

//
// MessageId: MSG_DELETEDISK_NG
//
// MessageText:
//
// Failed to delete the virtual disk device %1!lu!.
//
#define MSG_DELETEDISK_NG                ((DWORD)0x0000001BL)

//
// MessageId: MSG_OPENIMAGE_NG
//
// MessageText:
//
// Failed to open the virtual disk image.
//
#define MSG_OPENIMAGE_NG                 ((DWORD)0x0000001CL)

//
// MessageId: MSG_CLOSING_DISK
//
// MessageText:
//
// Closing the image on the virtual disk %1!lu!...%0
//
#define MSG_CLOSING_DISK                 ((DWORD)0x0000001DL)

//
// MessageId: MSG_CLOSE_OK
//
// MessageText:
//
// The image is closed.
//
#define MSG_CLOSE_OK                     ((DWORD)0x0000001EL)

//
// MessageId: MSG_DEVICE_EMPTY
//
// MessageText:
//
// The drive is empty.
//
#define MSG_DEVICE_EMPTY                 ((DWORD)0x0000001FL)

//
// MessageId: MSG_DEVICE_BUSY
//
// MessageText:
//
// The device is busy.
//
#define MSG_DEVICE_BUSY                  ((DWORD)0x00000020L)

//
// MessageId: MSG_LINK_OK
//
// MessageText:
//
// Assigned the drive letter '%1!c!' to disk %2!lu! partition %3!lu!.
//
#define MSG_LINK_OK                      ((DWORD)0x00000021L)

//
// MessageId: MSG_LINK_NG
//
// MessageText:
//
// Failed to assign the drive letter '%1!c!' to disk %2!lu! partition %3!lu!.
//
#define MSG_LINK_NG                      ((DWORD)0x00000022L)

//
// MessageId: MSG_UNLINK_OK
//
// MessageText:
//
// Removed the drive letter '%1!c!'.
//
#define MSG_UNLINK_OK                    ((DWORD)0x00000023L)

//
// MessageId: MSG_UNLINK_NG
//
// MessageText:
//
// Failed to remove the drive letter '%1!c!'.
//
#define MSG_UNLINK_NG                    ((DWORD)0x00000024L)

//
// MessageId: MSG_GET_IMAGE_NG
//
// MessageText:
//
// Failed to get image information from disk '%1!lu!'.
//
#define MSG_GET_IMAGE_NG                 ((DWORD)0x00000025L)

//
// MessageId: MSG_GET_STAT_NG
//
// MessageText:
//
// Failed to get the driver status.
//
#define MSG_GET_STAT_NG                  ((DWORD)0x00000026L)

//
// MessageId: MSG_GET_CONFIG_NG
//
// MessageText:
//
// Failed to get the driver configuration.
//
#define MSG_GET_CONFIG_NG                ((DWORD)0x00000027L)

//
// MessageId: MSG_GET_LINK_NG
//
// MessageText:
//
// Failed to get the drive letter of disk %1!lu! partition %2!lu!.
//
#define MSG_GET_LINK_NG                  ((DWORD)0x00000028L)

//
// MessageId: MSG_RESOLVE_LINK_NG
//
// MessageText:
//
// Failed to get the target disk of drive letter %1!c!.
//
#define MSG_RESOLVE_LINK_NG              ((DWORD)0x00000029L)

//
// MessageId: MSG_DISMOUNT_NG
//
// MessageText:
//
// Failed to dismount a partition.
//
#define MSG_DISMOUNT_NG                  ((DWORD)0x0000002AL)

//
// MessageId: MSG_CLOSE_FORCED
//
// MessageText:
//
// The operation is forced to proceed.
//
#define MSG_CLOSE_FORCED                 ((DWORD)0x0000002BL)

//
// MessageId: MSG_CREATEREDO_NG
//
// MessageText:
//
// Failed to create a REDO log.
//
#define MSG_CREATEREDO_NG                ((DWORD)0x0000002CL)


//
//     Additional information about operation failure
//

//
// MessageId: MSG_DRIVE_IN_USE
//
// MessageText:
//
// Some programs may be using the virtual drive.
//
#define MSG_DRIVE_IN_USE                 ((DWORD)0x0000002DL)

//
// MessageId: MSG_REMOVE_PENDING
//
// MessageText:
//
// The Virtual Disk Driver will be uninstalled on the next system start up.
// You may need to restart the system before reinstalling the driver.
//
#define MSG_REMOVE_PENDING               ((DWORD)0x0000002EL)

//
// MessageId: MSG_STOP_PENDING
//
// MessageText:
//
// Stop operation has succeeded, but something
// is preventing the driver from actually stopping.
//
#define MSG_STOP_PENDING                 ((DWORD)0x0000002FL)

//
// MessageId: MSG_NO_MORE_DISK
//
// MessageText:
//
// Cannot create any more disk device.
//
#define MSG_NO_MORE_DISK                 ((DWORD)0x00000030L)

//
// MessageId: MSG_NO_LESS_DISK
//
// MessageText:
//
// Cannot delete the disk device 0.
//
#define MSG_NO_LESS_DISK                 ((DWORD)0x00000031L)

//
// MessageId: MSG_GET_PART_NG
//
// MessageText:
//
// Failed to read partition table.
//
#define MSG_GET_PART_NG                  ((DWORD)0x00000032L)

//
// MessageId: MSG_FORCING_WARN
//
// MessageText:
//
// Although you can force to close the image, it may lead to loss of
// data or an unexpected behavior of the operating system.
//
#define MSG_FORCING_WARN                 ((DWORD)0x00000033L)

//
// MessageId: MSG_RESTART_WARN
//
// MessageText:
//
// You may need to reboot the system before restarting the driver.
//
#define MSG_RESTART_WARN                 ((DWORD)0x00000034L)

//
// MessageId: MSG_ALREADY_INSTALLED
//
// MessageText:
//
// The Virtual Disk Driver is already installed.
//
#define MSG_ALREADY_INSTALLED            ((DWORD)0x00000035L)

//
// MessageId: MSG_NOT_INSTALLED
//
// MessageText:
//
// The Virtual Disk Driver is not installed.
//
#define MSG_NOT_INSTALLED                ((DWORD)0x00000036L)

//
// MessageId: MSG_ALREADY_RUNNING
//
// MessageText:
//
// The Virtual Disk Driver is already running.
//
#define MSG_ALREADY_RUNNING              ((DWORD)0x00000037L)

//
// MessageId: MSG_NOT_RUNNING
//
// MessageText:
//
// The Virtual Disk Driver is not running.
//
#define MSG_NOT_RUNNING                  ((DWORD)0x00000038L)

//
// MessageId: MSG_ALREADY_OPENED
//
// MessageText:
//
// Another Virtual Disk image is already opened.
//
#define MSG_ALREADY_OPENED               ((DWORD)0x00000039L)

//
// MessageId: MSG_DRIVE_FULL
//
// MessageText:
//
// No drive letter is available for the Virtual Disk drive.
//
#define MSG_DRIVE_FULL                   ((DWORD)0x0000003AL)


//
//     driver status information message
//

//
// MessageId: MSG_CURRENT_STATUS
//
// MessageText:
//
// Driver Status   : %0
//
#define MSG_CURRENT_STATUS               ((DWORD)0x0000003BL)

//
// MessageId: MSG_STATUS_NOT_INST
//
// MessageText:
//
// NOT INSTALLED
//
#define MSG_STATUS_NOT_INST              ((DWORD)0x0000003CL)

//
// MessageId: MSG_STATUS_STOPPED
//
// MessageText:
//
// STOPPED
//
#define MSG_STATUS_STOPPED               ((DWORD)0x0000003DL)

//
// MessageId: MSG_STATUS_START_P
//
// MessageText:
//
// START_PENDING
//
#define MSG_STATUS_START_P               ((DWORD)0x0000003EL)

//
// MessageId: MSG_STATUS_STOP_P
//
// MessageText:
//
// STOP_PENDING
//
#define MSG_STATUS_STOP_P                ((DWORD)0x0000003FL)

//
// MessageId: MSG_STATUS_RUNNING
//
// MessageText:
//
// RUNNING
//
#define MSG_STATUS_RUNNING               ((DWORD)0x00000040L)

//
// MessageId: MSG_STATUS_CONT_P
//
// MessageText:
//
// CONTINUE_PENDING
//
#define MSG_STATUS_CONT_P                ((DWORD)0x00000041L)

//
// MessageId: MSG_STATUS_PAUSE_P
//
// MessageText:
//
// PAUSE_PENDING
//
#define MSG_STATUS_PAUSE_P               ((DWORD)0x00000042L)

//
// MessageId: MSG_STATUS_PAUSED
//
// MessageText:
//
// PAUSED
//
#define MSG_STATUS_PAUSED                ((DWORD)0x00000043L)

//
// MessageId: MSG_UNKNOWN_ULONG
//
// MessageText:
//
// Unknown (0x%1!08x!)
//
#define MSG_UNKNOWN_ULONG                ((DWORD)0x00000044L)


//
//     driver configuration message
//

//
// MessageId: MSG_DRIVER_LOCATION
//
// MessageText:
//
// Driver File     : %1!s!
//
#define MSG_DRIVER_LOCATION              ((DWORD)0x00000045L)

//
// MessageId: MSG_DRIVER_VERSION
//
// MessageText:
//
// Driver Version  : %1!u!.%2!u!%3!s!
//
#define MSG_DRIVER_VERSION               ((DWORD)0x00000046L)

//
// MessageId: MSG_START_TYPE
//
// MessageText:
//
// Start Type      : %0
//
#define MSG_START_TYPE                   ((DWORD)0x00000047L)

//
// MessageId: MSG_START_AUTO
//
// MessageText:
//
// AUTO
//
#define MSG_START_AUTO                   ((DWORD)0x00000048L)

//
// MessageId: MSG_START_BOOT
//
// MessageText:
//
// BOOT
//
#define MSG_START_BOOT                   ((DWORD)0x00000049L)

//
// MessageId: MSG_START_DEMAND
//
// MessageText:
//
// DEMAND
//
#define MSG_START_DEMAND                 ((DWORD)0x0000004AL)

//
// MessageId: MSG_START_DISABLED
//
// MessageText:
//
// DISABLED
//
#define MSG_START_DISABLED               ((DWORD)0x0000004BL)

//
// MessageId: MSG_START_SYSTEM
//
// MessageText:
//
// SYSTEM
//
#define MSG_START_SYSTEM                 ((DWORD)0x0000004CL)

//
// MessageId: MSG_DISK_DEVICE
//
// MessageText:
//
// Number of Disks : %1!lu!
//
#define MSG_DISK_DEVICE                  ((DWORD)0x0000004DL)

//
// MessageId: MSG_ATTACHED_PART
//
// MessageText:
//
// Attached Parts  : %1!lu!
//
#define MSG_ATTACHED_PART                ((DWORD)0x0000004EL)

//
// MessageId: MSG_ORPHANED_PART
//
// MessageText:
//
// Orphaned Parts  : %1!lu!
//
#define MSG_ORPHANED_PART                ((DWORD)0x0000004FL)

//
// MessageId: MSG_REFERENCE_COUNT
//
// MessageText:
//
// Reference Count : %1!lu!
//
#define MSG_REFERENCE_COUNT              ((DWORD)0x00000050L)

//
// MessageId: MSG_VIRTUAL_DISK
//
// MessageText:
//
// Virtual Disk %1!lu!
//
#define MSG_VIRTUAL_DISK                 ((DWORD)0x00000051L)


//
//     Image file information message
//

//
// MessageId: MSG_DISKIMAGE_NAME
//
// MessageText:
//
// Image Name      : %1!s!
//
#define MSG_DISKIMAGE_NAME               ((DWORD)0x00000052L)

//
// MessageId: MSG_IMAGE_NONE
//
// MessageText:
//
// Image File      : none
//
#define MSG_IMAGE_NONE                   ((DWORD)0x00000053L)

//
// MessageId: MSG_ACCESS_TYPE
//
// MessageText:
//
// Access Type     : %0
//
#define MSG_ACCESS_TYPE                  ((DWORD)0x00000054L)

//
// MessageId: MSG_ACCESS_RO
//
// MessageText:
//
// Read-Only
//
#define MSG_ACCESS_RO                    ((DWORD)0x00000055L)

//
// MessageId: MSG_ACCESS_RW
//
// MessageText:
//
// Writable
//
#define MSG_ACCESS_RW                    ((DWORD)0x00000056L)

//
// MessageId: MSG_ACCESS_WB
//
// MessageText:
//
// Write-Blocked
//
#define MSG_ACCESS_WB                    ((DWORD)0x00000057L)

//
// MessageId: MSG_DISK_CAPACITY
//
// MessageText:
//
// Disk Capacity   : %1!lu! sectors (%2!lu! MB)
//
#define MSG_DISK_CAPACITY                ((DWORD)0x00000058L)

//
// MessageId: MSG_DISK_GEOMETRY
//
// MessageText:
//
// Geometry        : (C) %1!lu! * (H) %2!lu! * (S) %3!lu!
//
#define MSG_DISK_GEOMETRY                ((DWORD)0x00000059L)

//
// MessageId: MSG_DISK_FILES
//
// MessageText:
//
// Number Of Files : %1!lu!
//
#define MSG_DISK_FILES                   ((DWORD)0x0000005AL)

//
// MessageId: MSG_FILE_HEADER
//
// MessageText:
//
// 
//   Type     Size    Path
//
#define MSG_FILE_HEADER                  ((DWORD)0x0000005BL)

//
// MessageId: MSG_PARTITION_HEADER
//
// MessageText:
//
// Partitions      :
//       #   Start Sector    Length in sectors    Type
//      --   ------------  ---------------------  ----
//
#define MSG_PARTITION_HEADER             ((DWORD)0x0000005CL)

//
// MessageId: MSG_PARTITION_NONE
//
// MessageText:
//
// Partitions      : none
//
#define MSG_PARTITION_NONE               ((DWORD)0x0000005DL)


//
//     VDisk callback message
//

//
// MessageId: MSG_CB_FILE_OPEN
//
// MessageText:
//
// Failed to open file '%1!s!'.
// %2!s!
//
#define MSG_CB_FILE_OPEN                 ((DWORD)0x0000005EL)

//
// MessageId: MSG_CB_FILE_TYPE
//
// MessageText:
//
// Failed to decide type of '%1!s!'.
// Open as a simple sector image file.
//
#define MSG_CB_FILE_TYPE                 ((DWORD)0x0000005FL)

//
// MessageId: MSG_CB_EMPTY_IMAGE
//
// MessageText:
//
// No data file entry in '%1!s!'.
//
#define MSG_CB_EMPTY_IMAGE               ((DWORD)0x00000060L)

//
// MessageId: MSG_CB_SIZE_BOUNDARY
//
// MessageText:
//
// '%1!s!' File size (%2!s! bytes)
// is not a multiple of sector size.
// The surplus area (%3!lu! bytes) will not be used.
//
#define MSG_CB_SIZE_BOUNDARY             ((DWORD)0x00000061L)

//
// MessageId: MSG_CB_SIGNATURE
//
// MessageText:
//
// '%1!s!' Invalid signature.
// > 0x%2!08lx!
//
#define MSG_CB_SIGNATURE                 ((DWORD)0x00000062L)

//
// MessageId: MSG_CB_CONTROLLER
//
// MessageText:
//
// '%1!s!' Unknown controller type.
// > %2!s!
//
#define MSG_CB_CONTROLLER                ((DWORD)0x00000063L)

//
// MessageId: MSG_PROMPT_CONTROLLER
//
// MessageText:
//
// I) ide or S) scsi or C) cancel ? %0
//
#define MSG_PROMPT_CONTROLLER            ((DWORD)0x00000064L)

//
// MessageId: MSG_CB_HARDWAREVER
//
// MessageText:
//
// '%1!s!' Unknown virtual hardware version.
// > %2!s!
//
#define MSG_CB_HARDWAREVER               ((DWORD)0x00000065L)

//
// MessageId: MSG_CB_DESC_BADENTRY
//
// MessageText:
//
// '%1!s!' Invalid description entry.
// > %2!s!
//
#define MSG_CB_DESC_BADENTRY             ((DWORD)0x00000066L)

//
// MessageId: MSG_CB_DESC_OFFSET
//
// MessageText:
//
// '%1!s!' Invalid extent offset.
// > %2!s!
//
#define MSG_CB_DESC_OFFSET               ((DWORD)0x00000067L)

//
// MessageId: MSG_CB_DESC_CAPACITY
//
// MessageText:
//
// '%1!s!' Invalid extent capacity.
// > '%2!s!'
//
#define MSG_CB_DESC_CAPACITY             ((DWORD)0x00000068L)

//
// MessageId: MSG_CB_DESC_GEOMETRY
//
// MessageText:
//
// '%1!s!' Invalid geometry entry.
// > '%2!s!'
//
#define MSG_CB_DESC_GEOMETRY             ((DWORD)0x00000069L)

//
// MessageId: MSG_CB_DESC_FILETYPE
//
// MessageText:
//
// '%1!s!' Unknown extent type.
// > '%2!s!'
//
#define MSG_CB_DESC_FILETYPE             ((DWORD)0x0000006AL)

//
// MessageId: MSG_CB_DESC_TIMESTAMP
//
// MessageText:
//
// '%1!s!' Invalid timestamp entry.
// > '%2!s!'
//
#define MSG_CB_DESC_TIMESTAMP            ((DWORD)0x0000006BL)

//
// MessageId: MSG_CB_DESC_DISKTYPE
//
// MessageText:
//
// '%1!s!' Unknown virtual disk type.
// > '%2!s!'
//
#define MSG_CB_DESC_DISKTYPE             ((DWORD)0x0000006CL)

//
// MessageId: MSG_CB_EXT_OFFSET
//
// MessageText:
//
// '%1!s!' Described offset %2!lu! does not correspond to the actual position %3!lu!.  The actual posision is used.
//
#define MSG_CB_EXT_OFFSET                ((DWORD)0x0000006DL)

//
// MessageId: MSG_CB_EXT_FILESIZE
//
// MessageText:
//
// '%1!s!' Described capacity %2!lu! does not match the actual file size %3!lu!.  The actual size is used.
//
#define MSG_CB_EXT_FILESIZE              ((DWORD)0x0000006EL)

//
// MessageId: MSG_CB_EXT_CAPACITY
//
// MessageText:
//
// '%1!s!' Described disk capacity %2!lu! does not match the actuall total of all extents %3!lu!.
//
#define MSG_CB_EXT_CAPACITY              ((DWORD)0x0000006FL)

//
// MessageId: MSG_CB_COWD_ORDINAL
//
// MessageText:
//
// '%1!s!' Extent ordinal stored in the header %2!lu! does not match the actual order %3!lu!.  The actual order is used.
//
#define MSG_CB_COWD_ORDINAL              ((DWORD)0x00000070L)

//
// MessageId: MSG_CB_CONF_FILEVER
//
// MessageText:
//
// COWdisk version mismatch.
//     %1!s! -> %2!lu!
//     %3!s! -> %4!lu!
//
#define MSG_CB_CONF_FILEVER              ((DWORD)0x00000071L)

//
// MessageId: MSG_CB_CONF_FLAGS
//
// MessageText:
//
// General flags mismatch.
//     %1!s! -> 0x%2!08lx!
//     %3!s! -> 0x%4!08lx!
//
#define MSG_CB_CONF_FLAGS                ((DWORD)0x00000072L)

//
// MessageId: MSG_CB_CONF_PARENTTS
//
// MessageText:
//
// Parent timestamp mismatch.
//     %1!s! -> 0x%2!08lx!
//     %3!s! -> 0x%4!08lx!
//
#define MSG_CB_CONF_PARENTTS             ((DWORD)0x00000073L)

//
// MessageId: MSG_CB_CONF_TIMESTAMP
//
// MessageText:
//
// Timestamp mismatch.
//     %1!s! -> 0x%2!08lx!
//     %3!s! -> 0x%4!08lx!
//
#define MSG_CB_CONF_TIMESTAMP            ((DWORD)0x00000074L)

//
// MessageId: MSG_CB_CONF_CONTROLLER
//
// MessageText:
//
// Controller type mismatch.
//     %1!s! -> %2!s!
//     %3!s! -> %4!s!
//
#define MSG_CB_CONF_CONTROLLER           ((DWORD)0x00000075L)

//
// MessageId: MSG_CB_CONF_EXTENTS
//
// MessageText:
//
// Number of extents mismatch.
//     %1!s! -> %2!lu!
//     %3!s! -> %4!lu!
//
#define MSG_CB_CONF_EXTENTS              ((DWORD)0x00000076L)

//
// MessageId: MSG_CB_CONF_CYLINDERS
//
// MessageText:
//
// Geometry (cylinders) mismatch.
//     %1!s! -> %2!lu!
//     %3!s! -> %4!lu!
//
#define MSG_CB_CONF_CYLINDERS            ((DWORD)0x00000077L)

//
// MessageId: MSG_CB_CONF_TRACKS
//
// MessageText:
//
// Geometry (tracks) mismatch.
//     %1!s! -> %2!lu!
//     %3!s! -> %4!lu!
//
#define MSG_CB_CONF_TRACKS               ((DWORD)0x00000078L)

//
// MessageId: MSG_CB_CONF_SECTORS
//
// MessageText:
//
// Geometry (sectors) mismatch.
//     %1!s! -> %2!lu!
//     %3!s! -> %4!lu!
//
#define MSG_CB_CONF_SECTORS              ((DWORD)0x00000079L)

//
// MessageId: MSG_CB_CONF_CAPACITY
//
// MessageText:
//
// Disk capacity mismatch.
//     %1!s! -> %2!lu!
//     %3!s! -> %4!lu!
//
#define MSG_CB_CONF_CAPACITY             ((DWORD)0x0000007AL)

//
// MessageId: MSG_CB_CONF_HARDWARE
//
// MessageText:
//
// Virtual hardware version mismatch.
//     %1!s! -> %2!lu!
//     %3!s! -> %4!lu!
//
#define MSG_CB_CONF_HARDWARE             ((DWORD)0x0000007BL)

//
// MessageId: MSG_CB_CONF_TOOLSFLAG
//
// MessageText:
//
// Tools flags mismatch.
//     %1!s! -> 0x%2!08lx!
//     %3!s! -> 0x%4!08lx!
//
#define MSG_CB_CONF_TOOLSFLAG            ((DWORD)0x0000007CL)

//
// MessageId: MSG_CB_CONF_SEQNUM
//
// MessageText:
//
// Sequence number mismatch.
//     %1!s! -> %2!lu!
//     %3!s! -> %4!lu!
//
#define MSG_CB_CONF_SEQNUM               ((DWORD)0x0000007DL)

//
// MessageId: MSG_CB_CONF_PARENTPATH
//
// MessageText:
//
// Parent path mismatch.
//     %1!s! -> %2!s!
//     %3!s! -> %4!s!
//
#define MSG_CB_CONF_PARENTPATH           ((DWORD)0x0000007EL)

//
// MessageId: MSG_CB_COWD_CAPACITY
//
// MessageText:
//
// '%1!s!' Disk capacity stored in the header %2!lu! does not match the total of all files %3!lu!.
//
#define MSG_CB_COWD_CAPACITY             ((DWORD)0x0000007FL)

//
// MessageId: MSG_CB_COWD_FILEVER
//
// MessageText:
//
// '%1!s!' Unknown COWDisk version.
// > %2!lu!
//
#define MSG_CB_COWD_FILEVER              ((DWORD)0x00000080L)

//
// MessageId: MSG_CB_COWD_GEOMETRY
//
// MessageText:
//
// '%1!s!' Invalid geometry values:
// > (C) %2!lu! / (H) %3!lu! / (S) %4!lu! -> %5!lu!
//
#define MSG_CB_COWD_GEOMETRY             ((DWORD)0x00000081L)

//
// MessageId: MSG_CB_COWD_PARENT
//
// MessageText:
//
// '%1!s!' Parent path is too long.
// > %2!s!
//
#define MSG_CB_COWD_PARENT               ((DWORD)0x00000082L)

//
// MessageId: MSG_CB_COWD_MAPSIZE
//
// MessageText:
//
// '%1!s!' Invalid primary map size %2!lu!. Correct value is %3!lu!.
//
#define MSG_CB_COWD_MAPSIZE              ((DWORD)0x00000083L)

//
// MessageId: MSG_CB_COWD_ENDOFFILE
//
// MessageText:
//
// '%1!s!' File size stored in the header %2!lu! does not match the actual file size %3!lu!.  The actual size is used.
//
#define MSG_CB_COWD_ENDOFFILE            ((DWORD)0x00000084L)

//
// MessageId: MSG_CB_COWD_TIMESTAMP
//
// MessageText:
//
// '%1!s!' Timestamps in the header does not match (0x%2!08lx! <> 0x%3!08lx!).
//
#define MSG_CB_COWD_TIMESTAMP            ((DWORD)0x00000085L)

//
// MessageId: MSG_CB_VMDK_NODESC
//
// MessageText:
//
// '%1!s!' does not contain a virtual disk description.
//
#define MSG_CB_VMDK_NODESC               ((DWORD)0x00000086L)

//
// MessageId: MSG_CB_VMDK_FILEVER
//
// MessageText:
//
// '%1!s!' Unknown VMDK version.
// > %2!lu!
//
#define MSG_CB_VMDK_FILEVER              ((DWORD)0x00000087L)

//
// MessageId: MSG_CB_VMDK_FILECAP
//
// MessageText:
//
// '%1!s!' Invalid file capacity.
// > %2!s!
//
#define MSG_CB_VMDK_FILECAP              ((DWORD)0x00000088L)

//
// MessageId: MSG_CB_VMDK_GRANULARITY
//
// MessageText:
//
// '%1!s!' Invalid granularity.
// > %2!s!
//
#define MSG_CB_VMDK_GRANULARITY          ((DWORD)0x00000089L)

//
// MessageId: MSG_CB_VMDK_DESCOFFSET
//
// MessageText:
//
// '%1!s!' Invalid descriptor offset.
// > %2!s!
//
#define MSG_CB_VMDK_DESCOFFSET           ((DWORD)0x0000008AL)

//
// MessageId: MSG_CB_VMDK_DESCSIZE
//
// MessageText:
//
// '%1!s!' Invalid descriptor size.
// > %2!s!
//
#define MSG_CB_VMDK_DESCSIZE             ((DWORD)0x0000008BL)

//
// MessageId: MSG_CB_VMDK_GTESPERGT
//
// MessageText:
//
// '%1!s!' Invalid grain table size.
// > %2!lu!
//
#define MSG_CB_VMDK_GTESPERGT            ((DWORD)0x0000008CL)

//
// MessageId: MSG_CB_VMDK_GDOFFSET
//
// MessageText:
//
// '%1!s!' Invalid grain directory offset.
// > %2!s!
//
#define MSG_CB_VMDK_GDOFFSET             ((DWORD)0x0000008DL)

//
// MessageId: MSG_CB_VMDK_GRAINOFFSET
//
// MessageText:
//
// '%1!s!' Invalid grain offset.
// > %2!s!
//
#define MSG_CB_VMDK_GRAINOFFSET          ((DWORD)0x0000008EL)

//
// MessageId: MSG_CB_VMDK_CHECKBYTES
//
// MessageText:
//
// '%1!s!' Invalid check data.  The file may have been transfered in text mode.
//
#define MSG_CB_VMDK_CHECKBYTES           ((DWORD)0x0000008FL)

//
// MessageId: MSG_CB_VMDK_SIZEMISMATCH
//
// MessageText:
//
// '%1!s!' Capacity in the header %2!lu! does not match the capacity in the descriptor %3!lu!.  The value in the header is used.
//
#define MSG_CB_VMDK_SIZEMISMATCH         ((DWORD)0x00000090L)

//
// MessageId: MSG_CB_PARENT_CAPACITY
//
// MessageText:
//
// Disk capacity doesn't match the parent.
//     %1!s! -> %2!lu!
//     %3!s! -> %4!lu!
//
#define MSG_CB_PARENT_CAPACITY           ((DWORD)0x00000091L)

//
// MessageId: MSG_CB_PARENT_CONTROLLER
//
// MessageText:
//
// Controller type doesn't match the parent.
//     %1!s! -> %2!s!
//     %3!s! -> %4!s!
//
#define MSG_CB_PARENT_CONTROLLER         ((DWORD)0x00000092L)

//
// MessageId: MSG_CB_PARENT_TIMESTAMP
//
// MessageText:
//
// Timestamp doesn't match the parent.
//     %1!s! -> 0x%2!08lx!
//     %3!s! -> 0x%4!08lx!
//
#define MSG_CB_PARENT_TIMESTAMP          ((DWORD)0x00000093L)


//
//     Help Message
//

//
// MessageId: MSG_HELP_USAGE
//
// MessageText:
//
// Usage:  VDK.EXE command [options...]
// Try 'VDK.EXE HELP' for more information.
//
#define MSG_HELP_USAGE                   ((DWORD)0x00000094L)

//
// MessageId: MSG_USAGE_INSTALL
//
// MessageText:
//
// SYNTAX:   VDK.EXE INSTALL [driver] [/AUTO]
// Try 'VDK.EXE HELP INSTALL' for more information.
//
#define MSG_USAGE_INSTALL                ((DWORD)0x00000095L)

//
// MessageId: MSG_USAGE_REMOVE
//
// MessageText:
//
// SYNTAX:   VDK.EXE REMOVE
// Try 'VDK.EXE HELP REMOVE' for more information.
//
#define MSG_USAGE_REMOVE                 ((DWORD)0x00000096L)

//
// MessageId: MSG_USAGE_START
//
// MessageText:
//
// SYNTAX:   VDK.EXE START
// Try 'VDK.EXE HELP START' for more information.
//
#define MSG_USAGE_START                  ((DWORD)0x00000097L)

//
// MessageId: MSG_USAGE_STOP
//
// MessageText:
//
// SYNTAX:   VDK.EXE STOP
// Try 'VDK.EXE HELP STOP' for more information.
//
#define MSG_USAGE_STOP                   ((DWORD)0x00000098L)

//
// MessageId: MSG_USAGE_DRIVER
//
// MessageText:
//
// SYNTAX:   VDK.EXE DRIVER
// Try 'VDK.EXE HELP DRIVER' for more information.
//
#define MSG_USAGE_DRIVER                 ((DWORD)0x00000099L)

//
// MessageId: MSG_USAGE_DISK
//
// MessageText:
//
// SYNTAX:   VDK.EXE DISK number
// Try 'VDK.EXE HELP DISK' for more information.
//
#define MSG_USAGE_DISK                   ((DWORD)0x0000009AL)

//
// MessageId: MSG_USAGE_CREATE
//
// MessageText:
//
// SYNTAX:   VDK.EXE CREATE
// Try 'VDK.EXE HELP CREATE' for more information.
//
#define MSG_USAGE_CREATE                 ((DWORD)0x0000009BL)

//
// MessageId: MSG_USAGE_DELETE
//
// MessageText:
//
// SYNTAX:   VDK.EXE DELETE
// Try 'VDK.EXE HELP DELETE' for more information.
//
#define MSG_USAGE_DELETE                 ((DWORD)0x0000009CL)

//
// MessageId: MSG_USAGE_VIEW
//
// MessageText:
//
// SYNTAX:   VDK.EXE VIEW path [/SEARCH:path]
// Try 'VDK.EXE HELP VIEW' for more information.
//
#define MSG_USAGE_VIEW                   ((DWORD)0x0000009DL)

//
// MessageId: MSG_USAGE_OPEN
//
// MessageText:
//
// SYNTAX:   VDK.EXE OPEN disk# image [/RW | /WB | /UNDO | /UNDO:path]
//               [/SEARCH:path] [/P:part#] [/L:drive]
// Try 'VDK.EXE HELP OPEN' for more information.
//
#define MSG_USAGE_OPEN                   ((DWORD)0x0000009EL)

//
// MessageId: MSG_USAGE_CLOSE
//
// MessageText:
//
// SYNTAX:   VDK.EXE CLOSE disk# [/Q | /F]
//           VDK.EXE CLOSE drive [/Q | /F]
// Try 'VDK.EXE HELP CLOSE' for more information.
//
#define MSG_USAGE_CLOSE                  ((DWORD)0x0000009FL)

//
// MessageId: MSG_USAGE_LINK
//
// MessageText:
//
// SYNTAX:   VDK.EXE LINK disk# part# [drive]
// Try 'VDK.EXE HELP LINK' for more information.
//
#define MSG_USAGE_LINK                   ((DWORD)0x000000A0L)

//
// MessageId: MSG_USAGE_ULINK
//
// MessageText:
//
// SYNTAX:   VDK.EXE ULINK disk# part#
//           VDK.EXE ULINK drive
// Try 'VDK.EXE HELP ULINK' for more information.
//
#define MSG_USAGE_ULINK                  ((DWORD)0x000000A1L)

//
// MessageId: MSG_USAGE_IMAGE
//
// MessageText:
//
// SYNTAX:   VDK.EXE IMAGE [disk#]
//           VDK.EXE IMAGE [drive]
// Try 'VDK.EXE HELP IMAGE' for more information.
//
#define MSG_USAGE_IMAGE                  ((DWORD)0x000000A2L)

//
// MessageId: MSG_USAGE_HELP
//
// MessageText:
//
// SYNTAX:   VDK.EXE HELP [command]
// Try 'VDK.EXE HELP HELP' for more information.
//
#define MSG_USAGE_HELP                   ((DWORD)0x000000A3L)

//
// MessageId: MSG_HELP_GENERAL
//
// MessageText:
//
// Usage:
//   VDK.EXE command [options...]
// 
// Commands:
//   INSTALL   Install the Virtual Disk Driver.
//   REMOVE    Uninstall the Virtual Disk Driver.
//   START     Start the Virtual Disk Driver.
//   STOP      Stop the Virtual Disk Driver.
//   DRIVER    Print Virtual Disk Driver status.
//   DISK      Set the initial number of virtual disk devices.
//   CREATE    Create a new virtual disk device.
//   DELETE    Delete a virtual disk device.
//   VIEW      Print disk image information.
//   OPEN      Open a disk image as a virtual drive.
//   CLOSE     Close a disk image.
//   LINK      Assign a drive letter to a partition.
//   ULINK     Remove a drive letter from a partition.
//   IMAGE     Print opened image file information.
//   HELP      Print command help.
// 
// All commands and options are case insensitive.
// Try "VDK.EXE HELP command" for detailed help for each command.
//
#define MSG_HELP_GENERAL                 ((DWORD)0x000000A4L)

//
// MessageId: MSG_HELP_INSTALL
//
// MessageText:
//
// Install the Virtual Disk Driver.
// 
// SYNTAX:
//   VDK.EXE INSTALL [driver] [/AUTO]
// 
// OPTIONS:
//   driver    Specifies the path to the Virtual Disk Driver file (VDK.SYS).
//             Default is VDK.SYS in the same directory as VDK.EXE.
//             (Note: *NOT* current working directory.)
// 
//   /AUTO     Configures the driver to start at the system startup.
//             (Note: this option does not start the driver after installation
//             is completed.)
//             By default the driver has to be started manually.
// 
// Device drivers cannot be started from network drives.
// Make sure to place VDK.SYS on a local drive.
//
#define MSG_HELP_INSTALL                 ((DWORD)0x000000A5L)

//
// MessageId: MSG_HELP_REMOVE
//
// MessageText:
//
// Uninstall the Virtual Disk Driver.
// 
// SYNTAX:
//   VDK.EXE REMOVE
// 
// OPTIONS:
//   NONE
// 
// This command removes the Virtual Disk Driver entries from the system
// registry, but does not delete the driver file from the drive.
// If the driver is running, this command closes all image files and
// stops the driver before removing it from the system.
//
#define MSG_HELP_REMOVE                  ((DWORD)0x000000A6L)

//
// MessageId: MSG_HELP_START
//
// MessageText:
//
// Start the Virtual Disk Driver.
// 
// SYNTAX:
//   VDK.EXE START
// 
// OPTIONS:
//   NONE
// 
// If the driver is not already installed, this command attempts to install it
// with the default options.
//
#define MSG_HELP_START                   ((DWORD)0x000000A7L)

//
// MessageId: MSG_HELP_STOP
//
// MessageText:
//
// Stop the Virtual Disk Driver.
// 
// SYNTAX:
//   VDK.EXE STOP
// 
// OPTIONS:
//   NONE
// 
// This command closes all image files before stopping the driver.
// The driver cannot be stopped if virtual drives are used by any other programs.
//
#define MSG_HELP_STOP                    ((DWORD)0x000000A8L)

//
// MessageId: MSG_HELP_DRIVER
//
// MessageText:
//
// Print Virtual Disk Driver status.
// 
// SYNTAX:
//   VDK.EXE DRIVER
// 
// OPTIONS:
//   NONE
// 
// This commands prints the following information:
//     Driver file path
//     Driver file version
//     Driver start type (AUTO/MANUAL)
//     Current running state
//     Number of disk devices
//
#define MSG_HELP_DRIVER                  ((DWORD)0x000000A9L)

//
// MessageId: MSG_HELP_DISK
//
// MessageText:
//
// Set the initial number of virtual disks.
// 
// SYNTAX:
//   VDK.EXE DISK number
// 
// OPTIONS:
//   number    Number of virtual disks between 1 and 22.
//             The default value is 4.
// 
// This command sets the number of virtual disk devices created when the driver
// is started.
// If the driver is already running, the value takes effect the next time the
// driver is started.
//
#define MSG_HELP_DISK                    ((DWORD)0x000000AAL)

//
// MessageId: MSG_HELP_CREATE
//
// MessageText:
//
// Create a new Virtual Disk device.
// 
// SYNTAX:
//   VDK.EXE CREATE
// 
// OPTIONS:
//   NONE
// 
// This commands creates a new virtual disk device.
// The created disk has the highest disk number.
//
#define MSG_HELP_CREATE                  ((DWORD)0x000000ABL)

//
// MessageId: MSG_HELP_DELETE
//
// MessageText:
//
// Delete a Virtual Disk device.
// 
// SYNTAX:
//   VDK.EXE DELETE
// 
// OPTIONS:
//   NONE
// 
// This command deletes a virtual disk device with the highest disk number.
// A disk cannot be deleted if an image is opened, or any process is using it.
// Disk #0 cannot be deleted (there must exist at least one disk device).
//
#define MSG_HELP_DELETE                  ((DWORD)0x000000ACL)

//
// MessageId: MSG_HELP_VIEW
//
// MessageText:
//
// Print disk image information.
// 
// SYNTAX:
//   VDK.EXE VIEW image [/SEARCH:path]
// 
// OPTIONS:
//   image     Path to the image to print the information.
// 
//   /SEARCH:path
//             Specifies a path to search the image file (and related files).
//             The specified path has the highest priority in the searching
//             order, and searched even before the explicit paths in the
//             command line, descriptor files and virtual disk files.
// 
// This command prints the following information for the specified image:
// 
//     Virtual disk capacity (in 512 byte sectors)
//     Number of files composing the image
//     Type, capacity and path of each component file
//     Size and type of each partition in the image
//
#define MSG_HELP_VIEW                    ((DWORD)0x000000ADL)

//
// MessageId: MSG_HELP_OPEN
//
// MessageText:
//
// Open a disk image as a virtual drive.
// 
// SYNTAX:
//   VDK.EXE OPEN disk# image [/RW | /WB | /UNDO | /UNDO:path]
//         [/SEARCH:path] [/P:part#] [/L:drive]
// 
// OPTIONS:
//   disk#     Specifies the virtual disk number.
//             This must be the first parameter.
//             '*' means the first available disk, and if all existing disk is
//             busy a new virtual disk is created.
// 
//   image     Specifies the path to the disk image file.
//             This must be the second parameter.
// 
//   /RW       Open the image in Read-Write mode.
// 
//   /WB       Open the image in Write-Block mode.
// 
//   /UNDO | /UNDO:path
//             Creates a REDO log for the image and open in Read-Write mode.
//             If you specify a path, the REDO log is created in the specified
//             directory.
// 
//   /P:part#  Specifies a partition number to set a drive letter.
//             Drive letters can be set/removed later with LINK/ULINK commands.
//             By default, drive letters are assigned to all mountable partitions.
// 
//   /L:drive  Specifies drive letters to assign to partitions.
//             By default, the first available drive letter is used.
// 
//   /SEARCH:path
//             Specifies a path to search the image file (and related files).
//             The specified path has the highest priority in the searching
//             order, and searched even before the explicit paths in the
//             command line, descriptor files and virtual disk files.
// 
// Only one of /RW, /WB and /UNDO can be used at a time.
// When none of these is specified, the image is opened in Read-Only mode.
//
#define MSG_HELP_OPEN                    ((DWORD)0x000000AEL)

//
// MessageId: MSG_HELP_CLOSE
//
// MessageText:
//
// Close a disk image.
// 
// SYNTAX:
//   VDK.EXE CLOSE disk# [/Q | /F]
//   VDK.EXE CLOSE drive [/Q | /F]
// 
// OPTIONS:
//   disk#     Specifies the target virtual disk number.
//             '*' means all existing virtual disks.
// 
//   drive     Specifies a drive letter of a partition on the target disk.
// 
//   /Q        Suppresses prompting and fails the operation when an error
//             has occured.
// 
//   /F        Suppresses prompting and forces the image to close even if
//             an error has occured.
// 
// Make sure that other programs are not using any partitions on the drive
// before closing the image.  Generally, the image should not be closed
// while the virtual drive is used by any processes.
// Although you can force to close the image by answering to do so when
// asked or by using the /F option, you should be aware that to forcibly
// closing an image may lead to loss of data or unexpected behavior of the
// operating system.
//
#define MSG_HELP_CLOSE                   ((DWORD)0x000000AFL)

//
// MessageId: MSG_HELP_LINK
//
// MessageText:
//
// Assign a drive letter to a partition.
// 
// SYNTAX:
//   VDK.EXE LINK disk# part# [drive]
// 
// OPTIONS:
//   disk#     Specifies the target virtual disk number.
// 
//   part#     Specifies the target partition number.
// 
//   drive     Specifies a drive letter to assign.
//             By default, the first available drive letter is used.
// 
// You cannot assign more than one drive letters to one partition.
// You cannot assign a drive letter already used for another volume.
// Use IMAGE command to see partition numbers and the current drive
// letter assignment.
//
#define MSG_HELP_LINK                    ((DWORD)0x000000B0L)

//
// MessageId: MSG_HELP_ULINK
//
// MessageText:
//
// Remove a drive letter from a partition.
// 
// SYNTAX:
//   VDK.EXE ULINK disk# part#
//   VDK.EXE ULINK drive
// 
// OPTIONS:
//   disk#     Specifies the target virtual disk number.
// 
//   part#     Specifies the target partition number.
// 
//   drive     Specifies the drive letter to remove.
// 
// This command can remove drive letters of virtual disk drives only.
// Use IMAGE command to see partition numbers and the current drive
// letter assignment.
//
#define MSG_HELP_ULINK                   ((DWORD)0x000000B1L)

//
// MessageId: MSG_HELP_IMAGE
//
// MessageText:
//
// Print current virtual disk image information.
// 
// SYNTAX:
//   VDK.EXE IMAGE [disk#]
//   VDK.EXE IMAGE [drive]
// 
// OPTIONS:
//   disk#     Specifies the target virtual disk number.
// 
//   drive     Specifies a drive letter of a partition on the target disk.
// 
// This command prints the following information for the target disk:
// 
//     Virtual disk access mode
//     Virtual disk capacity (in 512 byte sectors)
//     Number of files composing the virtual disk
//     Type, capacity and path of each component file
//     Drive letter, size and type of each partition on the virtual disk
// 
// When target is not specified, information for all drives are printed.
//
#define MSG_HELP_IMAGE                   ((DWORD)0x000000B2L)

//
// MessageId: MSG_HELP_HELP
//
// MessageText:
//
// Print VDK.EXE command help.
// 
// SYNTAX:
//   VDK.EXE HELP [command]
// 
// OPTIONS:
//   command   Specifies a VDK.EXE command to display help.
//             Following commands can be specified:
// 
//                 INSTALL REMOVE  START   STOP    DRIVER
//                 DISK    CREATE  DELETE  VIEW    OPEN
//                 CLOSE   LINK    ULINK   IMAGE   HELP
// 
//             If not specified, the general help is printed.
//
#define MSG_HELP_HELP                    ((DWORD)0x000000B3L)


#endif // _VDKMSG_H_
