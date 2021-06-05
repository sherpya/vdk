
    Virtual Disk Driver for Windows NT platform.
    Copyright (C) 2003-2005 Ken Kato (chitchat-lj@infoseek.jp)
    http://chitchat.at.infoseek.co.jp/vmware/vdk.html
    Copyright (c) 2021 Gianluigi Tiesi <sherpya@netfarm.it>

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.
    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.
    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA

    The GNU General Public License is also available from:
    http://www.gnu.org/copyleft/gpl.html

    This program owes a great deal to Bo Brantén's 'ntifs' project and
    'filedisk' program (http://www.acc.umu.se/~bosse/).
    This program simply could not happen without his great works.


INTRODUCTION
============

    This is a Virtual Disk Driver (VDK) Version 3 for Windows NT / 2000 / XP.
    I have received some reports that it also works on Windows 2000 Server.

    With VDK you can mount a VMware virtual disk to your Windows hosts and
    use them as another disk drive attached to your system.

    VDK can open the following VMware virtual disk types for both read-only
    and read-write access:

    - VMware 2.x Plain Disks (*.pln).
    - VMware 2.x / 3.x Virtual Disks (*.dsk, *.vmdk).
    - VMware 2.x / 3.x REDO log files based on both Plain Disks and
      Virtual Disks.
    - VMware 2.x / 3.x differential / nested Virtual Disks (which actually
      are renamed REDO logs), based on both Plain Disks and Virtual Disks.
      There is no limitation on the nesting level.
    - VMware 4.x Flat virtual disks (both split and monolithic).
    - VMware 4.x Sparse virtual disks (both split and monolithic).
    - VMware 4.x REDO log files (which actually are the same thing as
      split-sparse virtual disks) and differential / nested virtual disks
      (ditto).

    VDK allows you to use these types of virtual disks with minor errors,
    such as timestamp mismatch, sequence number mismatch, etc.
    (VDK simply ignores those errors, it does not fix them.)


    VDK can also open the following types of virtual disks.

    - Raw sector image which you can create with regular disk dump tools
      such as Linux dd command.

    - Non-partitioned disk image, such as floppy image, removable disk image,
      and dd image of one partition.  (This is not actually a type of virtual
      disk, but rather how the virtual disk is formatted.)
      In this case, the whole image is treated as a single partition.


    VDK is also capable of creating REDO log files which VMware can use.
    This means that VDK can open virtual disks in undoable mode, then you
    can commit the changes using VMware, or discard the changes by simply
    deleting created REDO log files.


    VDK cannot use the following virtual disks for the time being, but I'm
    planning to implement them:

    - Any disk types with serious errors.


    VDK cannot use the following disk types, and I have no intention of
    implementing them:

    - Raw Disks for any version of VMware.
    - REDO log files for Raw Disks.


    In this text, the term "Virtual Disk" (with capitals) specifically means
    growable virtual disk created with VMware 2.x / 3.x, whereas "virtual
    disk" (in lower case) is used in more generic sense as opposed to
    "physical disk".  I know it's kind of confusing, but I just couldn't
    come up with more suitable terms.


WARNING & LIMITATIONS
=====================

    The VMware virtual disk format and structure on which VDK was
    developed is entirely the result of my *personal* investigation.
    Absolutely no official information is provided by VMware, Inc. or any
    other resources.
    Please note that there is ABSOLUTELY NO WARRANTY and be sure to use it
    AT YOUR OWN RISK.

    You must have administrative privileges to use the VDK.  This is
    becuase Windows does not allow non-administrative users to manipulate
    hard disk volumes.

    VDK is a *disk* driver, not a file system driver, therefore you can
    only access partitions which your Windows host can recognize.
    For example, you cannot access ext2 partitions on virtual drives unless
    you have a 3rd party ext2 file system driver installed on your Windows
    system (See NOTES below for more information).

    VDK does not communicate with Windows 2000/XP Mount Manager.
    Therefore:

    - Disk Management Console (partition manipulation and mount point
      configuration) does not recognize the VDK virtual drives.
      Most of third party utilities, such as PartitionMagic, should work.

    - Disk Defragment utility does not recognize the VDK virtual drives.
      Third party defrag utility, such as Diskeeper, works file.

    - Graphical mode of Check Disk may not work.  Sometimes it works and
      sometimes it doesn't for me, though I couldn't figure out the pattern
      so far.  (command line chkdsk.exe works fine).

    VDK does not handle GPT partitioned disk (I don't really know what it
    is.  All I know is that it's a new type of partition table.)

    Also, I suspect that you cannot use Windows NT Fault Tolerant Volumes
    (mirror, stripe, volume set, etc) or Windows 2000/XP Dynamic Volumes on
    virtual drives.  I simply don't know because I've never used them.


TO DO
=====

    - Allow non-administrative users to use the program.

    - Better compatibility with Win2K/XP
      (PnP support, Mount Manager support, etc).

    - GUI front end program.

    - Virtual disk error correction.

    - Virtual disk type conversion.


FILES
=====

    vdk.sys       Virtual Disk kernel-mode Driver.

    vdk.exe       Virtual Disk Driver control program (console).

    README.TXT    Explanation of the Virtual Disk Driver (this file).

    COPYING.TXT   GNU GENERAL PUBLIC LICENSE Version 2.


INSTALL
=======

    Place VDK.SYS and VDK.EXE into a directory on a local drive.
    Then use VDK.EXE to install the driver (VDK.SYS) onto the system.
    Read the following sections for more information.

    If you have previous version of VDK.SYS installed on your system, you
    don't usually need to uninstall the previous driver from the system.
    You can just stop the old driver, replace the driver file, and restart
    the driver.

    !!! NOTE !!!
    Device drivers cannot be started from a network drive.  Make sure that
    at least the driver file (VDK.SYS) is placed on a local drive.


UNINSTALL
=========

    Use VDK.EXE to remove the driver entry from the system registry.
    Then delete VDK.SYS and VDK.EXE from the local drive.


VDK.EXE REFERENCE
=================
    
    VDK.EXE is a command line front end program to control the Virtual
    Disk Driver (VDK.SYS).
    
    SYNTAX:
      VDK.EXE command [command specific options...]

    COMMANDS:
      INSTALL   Install the Virtual Disk Driver.
      REMOVE    Uninstall the Virtual Disk Driver.
      START     Start the Virtual Disk Driver.
      STOP      Stop the Virtual Disk Driver.
      DRIVER    Print Virtual Disk Driver status.
      DISK      Set the initial number of Virtual Disk devices.
      CREATE    Create a new Virtual Disk device.
      DELETE    Delete a Virtual Disk device.
      VIEW      Print disk image files information.
      OPEN      Open disk image files as a virtual drive.
      CLOSE     Close disk image files.
      LINK      Assign a drive letter to a partition.
      ULINK     Remove a drive letter from a partition.
      IMAGE     Print opened image file information.
      HELP      Print command help.

    All commands and options are case insensitive.


VDK INSTALL
===========

    Install the Virtual Disk Driver.

    SYNTAX:
      VDK.EXE INSTALL [driver] [/AUTO]

    OPTIONS:
      driver    Specifies the path to the Virtual Disk Driver file (VDK.SYS).
                Default is VDK.SYS in the same directory as VDK.EXE.
                (Note: *NOT* current working directory.)

      /AUTO     Configures the driver to start at the system startup.
                (Note: this option does not start the driver after
                installation is completed.)
                By default the driver has to be started manually.

    Device drivers cannot be started from network drives.
    Make sure to place VDK.SYS on a local drive.


VDK REMOVE
==========

    Uninstall the Virtual Disk Driver.

    SYNTAX:
      VDK.EXE REMOVE

    OPTIONS:
      NONE

    This command removes the Virtual Disk Driver entries from the system
    registry, but does not delete the driver file from the drive.
    If the driver is running, this command closes all image files and
    stops the driver before removing it from the system.


VDK START
=========

    Start the Virtual Disk Driver.

    SYNTAX:
      VDK.EXE START

    OPTIONS:
      NONE

    If the driver is not already installed, this command attempts to install
    it with the default options.


VDK STOP
========

    Stop the Virtual Disk Driver.

    SYNTAX:
      VDK.EXE STOP

    OPTIONS:
      NONE

    This command closes all image files before stopping the driver.
    The driver cannot be stopped if virtual drives are used by any other
    programs.


VDK DRIVER
==========

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


VDK DISK
========

    Set the initial number of virtual disks.

    SYNTAX:
      VDK.EXE DISK number

    OPTIONS:
      number    Number of virtual disks between 1 and 22.
                The default value is 4.

    This command sets the number of virtual disk devices created when the
    driver is started.
    If the driver is already running, this value takes effect the next time
    the driver is started.


VDK CREATE
==========

    Create a new virtual disk device.

    SYNTAX:
      VDK.EXE CREATE

    OPTIONS:
      NONE

    This commands creates a new virtual disk device.
    The created disk has the highest disk number.


VDK DELETE
==========

    Delete a virtual disk device.

    SYNTAX:
      VDK.EXE DELETE

    OPTIONS:
      NONE

    This command deletes a virtual disk device with the highest disk number.
    A disk cannot be deleted if an image is opened, or any process is using it.
    Disk #0 cannot be deleted (there must exist at least one disk device).


VDK VIEW
========

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


VDK OPEN
========

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
                See IMAGE TYPES section below for more information

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
    See ACCESS MODES section below for more information about these options.


VDK CLOSE
=========

    Close a disk image.

    SYNTAX:
      VDK.EXE CLOSE disk#
      VDK.EXE CLOSE drive

    OPTIONS:
      disk#     Specifies the target virtual disk number.
                '*' means all existing virtual disks.

      drive     Specifies one of drive letters of partitions on the target
                disk.

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


VDK LINK
========

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

    With usual (partitioned) virtual disks, the first partition on the disk
    is partition 1, the second is partition 2, and so on.
    Unlike Linux, partition numbers for primary partitions and logical
    partitions are not distinguished -- logical partition number starts just
    after the last primary partition and can be any number from 1.

    Partition 0 represents the whole disk (like /dev/hda, /dev/hdb,
    /dev/sda, etc. in Linux) and you should not assign a drive letter to it.
    You can assign a drive letter, but file system drivers cannot access the
    partition, because it is not formatted in any file system.  It is just
    like you don't usually mount /dev/hda in Linux.

    Non-partitioned virtual disks (such as floppy image and removable disk
    image) only have partition 0, and a drive letter is to be assigned to
    this partition.  Again, it is like mounting /dev/fd0 in Linux.


VDK ULINK
=========

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

    Closing a virtual disk automatically unlinks all drive letters assigned
    to partitions on the virtual disk.


VDK IMAGE
=========

    Print current virtual disk image information.

    SYNTAX:
      VDK.EXE IMAGE [disk#]
      VDK.EXE IMAGE [drive]

    OPTIONS:
      disk#     Specifies the target virtual disk number.

      drive     Specifies one of drive letters of partitions on the target
                disk.

    This command prints the following information for the target disk:

        Virtual disk access mode
        Virtual disk capacity (in 512 byte sectors)
        Number of files composing the virtual disk
        Type, capacity and path of each component file
        Drive letter, size and type of each partition on the virtual disk

    When target is not specified, information for all drives are printed.


VDK HELP
========

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


IMAGE TYPES
===========

    You can use virtual disks on both local and network drives.
    VIEW and OPEN commands accept the following path expressions:

      relative paths    [<dir>\]<filename>
      absolute paths    x:\<dir>\<filename>
      UNC paths         \\<server>\<share>\<dir>\<filename>

    VIEW/OPEN commands can handle the following types of virtual disks:

    - VMware 2.x Plain Disk
        Consists of one descriptor file (*.pln) and one or more raw data
        files (*.dat).
        Use the path to the descriptor file for VIEW/OPEN commands.

    - VMware 2.x Virtual Disk / REDO log / differential disk (COWD version 1)
        Consists of only one virtual disk file (*.dsk or *.REDO).
        Use this path for VIEW/OPEN commands.

    - VMware 3.x Virtual Disk / REDO log / differential disk (COWD version 3)
        Consists of one or more virtual disk files (*.vmdk or *.REDO)
        Use any one of the composing files for VIEW/OPEN commands.

    - VMware 4.x Monolithic Flat Virtual Disk
        Consists of a descriptor file (*.vmdk) and a data file (*-flat.vmdk).
        Use the path to the descriptor file for VIEW/OPEN commands.

    - VMware 4.x Split Flat Virtual Disk
        Consists of one descriptor file (*.vmdk) and one or more raw data
        files (*-fxxx.vmdk).
        Use the path to the descriptor file for VIEW/OPEN commands.

    - VMware 4.x Monolithic Sparse Virtual Disk (VMDK version 1)
        Consists of only one virtual disk file (*.vmdk).
        Use this path for VIEW/OPEN commands.

    - VMware 4.x Split Sparse Virtual Disk / REDO log / differential disk
        (VMDK version 1)
        Consists of a descriptor file (*.vmdk or *.REDO_*) and one or more
        virtual disk files (*-sxxx.vmdk or *-sxxx.REDO_*).
        Use the path to the descriptor file for VIEW/OPEN commands.

    - Single raw sector image file
        Consists of only one raw sector image file.  The file size must
        be an integer multiple of 512.
        Use this path for VIEW/OPEN commands.

    - A set of raw sector image files
        Consists of a descriptor file and one or more raw sector image files.
        Use the path to the descriptor file for VIEW/OPEN commands.

        The descriptor file in this case is a plain text file which simply
        lists composing image files:

        e.g.1
        image01.dat
        image02.dat
        image03.dat
        ...

        e.g.2
        c:\data\volume.001
        c:\data\volume.002
        c:\data\volume.003
        ...

        e.g.3
        disk.img
        c:\work\part2.dsk
        ..\image\data


ACCESS MODES
============

    When an image is opened for read-only access (without any access mode
    options) the virtual drive is treated as a write protected volume.

    Even with /RW or /UNDO option, underlying files for REDO log,
    differential/nested Virtual Disks are always opened for read-only access.

    Use /RW option with extreme care because this option causes VDK to
    modify the virtual disk files.

    When you use /WB (write-block) option, you should avoid using the drive
    for a long period of time or writing large amount of data to the drive.
    The difference between data on the drive, which is unmodified, and data
    in the OS's disk cache, which the OS believes to be written successfully
    to the drive, may lead to an unpredictable behavior.


NOTES
=====

    Encrypted/compressed virtual disk files

        NTFS has native support for encrypting/compressing files on the
        volume.  However VDK does not allow to open encrypted or compressed
        virtual disk files.  This is because the Windows cache manager may
        deadlock if a virtual disk file is encrypted/compressed.


    Resolving relative paths

        If relative paths are used in descriptor files or REDO log,
                differential/nested Virtual Disks, VMware resolves them from the
        *.vmx file's location.  However VDK.EXE tries to resolve them from
        the current file's location, and VDK.EXE may not be able to find
        the target files.  In such cases, VDK.EXE will prompt you for the
        correct path.


    REDO logs created with /UNDO option

        If you use /UNDO option for opening *.pln, *.dsk or *.vmdk files,
        VMware will recognize the REDO log created by VDK.EXE, and you can
        commit or discard the REDO log using VMware.

        However VMware will not look for the REDO log for *.REDO files, so
        if you use /UNDO option for opening *.REDO files, you have to
        manually delete the created REDO log files afterward.


    Opening REDO logs

        VDK.EXE does not automatically open REDO log for the target disk as
        VMware does (unless /UNDO option is specified).  If you want to
        open a REDO log, you have to explicitly specify the path to the REDO
        log file as shown in the examples below:

        VDK.EXE OPEN VDISK.VMDK

            Opens VDISK.VMDK, even if VDISK.VMDK.REDO exists.
            VDISK.VMDK.REDO is not used at all.

        VDK.EXE OPEN VDISK.VMDK.REDO

            Opens VDISK.VMDK.REDO, which must exist.
            Also uses VDISK.VMDK for read-only access.

        VDK.EXE OPEN VDISK.VMDK /UNDO

            Creates VDISK.VMDK.REDO, which must NOT exist, then opens it
            for read-write access.
            Also uses VDISK.VMDK for read-only access.
            VMware can use VDISK.VMDK.REDO and will automatically recognize
            it when you use VDISK.VMDK as a virtual disk.

        VDK.EXE OPEN VDISK.VMDK.REDO /UNDO

            Creates VDISK.VMDK.REDO.REDO, which must NOT exist,
            then opens it for read-write access.
            Also uses VDISK.VMDK.REDO and VDISK.VMDK for read-only access.
            VMware can use VDISK.VMDK.REDO.REDO but will not automatically
            recognize it when you use VDISK.VMDK or VDISK.VMDK.REDO.
            You must explicitly specify VDISK.VMDK.REDO.REDO for VMware to
            use it.

    Using REDO logs, differential/nested Virtual Disks

        When changes are made to Virtual Disks, VDK does not update
        timestamp or sequence number in the Virtual Disk header as VMware
        does.

        If you made any changes to parent/underlying disks of REDO log or
        differential/nested Virtual Disks, VMware wouldn't know that they
        are consequently invalidated.

        Make sure not to make any changes to parent/underlying disks.
        Do not use /RW option to open them, and use /UNDO or /WB option
        instead if you need read-write access to the virtual drive.


    Partition types on virtual disks

        After successfully opened the virtual disk, VDK.EXE assigns drive
        letters to partitions of the following types on the virtual drive:
        
        - FAT16, NTFS partitions on Windows NT
        - FAT16, FAT32, NTFS partitions on Windows 2000/XP

        If you have 3rd party file system drivers installed on your system
        and want to access those partitions, you have to manually assign
        drive letters with VDK.EXE LINK command.

        The following file system driver works on VDK without any known
        problems:

        - FAT32 for Windows NT
            FAT32 file system driver for Windows NT 4.0
            available from Sysinternals.
            http://www.sysinternals.com/

        - Paragon Ext2FS Anywhere Demo version
            ext2/ext3 file system driver for Windows 95/98/ME/NT/2000/XP
            (free trial version with restricted features)
            available from Paragon Software Group
            http://www.paragon-gmbh.com/

            Thanks to Dave Parsons for mentioning this product to me.
            I suppose that the product version also works, but I cannot
            confirm it because I cannot afford it for the time being ;-)

        The following file system drivers work on VDK with some issues:

        - Ext2Fsd
            ext2 file system driver for Windows 2000/XP
            written by Matt Wu.
            http://sys.xiloo.com/

            * This driver only partially supports dismounting of the
            volumes.  Once you have mounted an ext2 partition, this driver
            keeps hold of the partition object and prevents the VDK driver
            from unloading (means that you cannot stop the VDK driver without
            rebooting your system).

        - Ext2 File System Driver for Windows NT 4.0
            ext2 file system driver for Windows NT 4.0
            written by Manoj Paul Joseph
            http://freesourcecodes.tripod.com/ext2.htm

            * This driver does not support dismounting of the volumes.
            As a consequence, once you have mounted an ext2 partition, this
            driver keeps hold of the partition object and prevents the VDK
            driver from unloading.

        - EXT2IFS
            ext2 file system driver for Windows NT/2000/XP
            written by John Newbigin
            http://uranus.it.swin.edu.au/~jn/linux/ext2ifs.htm

            * This driver does not support dismounting of the volumes.
            As a consequence, once you have mounted an ext2 partition, this
            driver keeps hold of the partition object and prevents the VDK
            driver from unloading.

            ** The GUI front end program included in EXT2IFS assumes
            that all ext2 partitions exist only on fixed disks (very
            reasonable assumption, I must say), so its drive letter
            assignment method does not work well with VDK.
            However if you assign drive letters to ext2 partitions on the
            virtual drive with VDK.EXE, the EXT2IFS driver recognizes the
            partition and you can access it.

        You can also use other utitity programs to access such partitions:

        - explore2fs
            A program to access ext2 volume from Windows.
            http://uranus.it.swin.edu.au/~jn/linux/explore2fs.htm

            Thanks to João Alves for mentioning this program to me.


    Read-only virtual drives and Windows

        By default VDK.EXE opens virtual disks read-only for some safety.
        However stupid Windows does not seem to cope well with the idea of
        read-only hard disk drives:
        
        - Windows NT and Windows 2000 simply cannot access NTFS partitions
          on read-only volumes.  (See the next section)

        - Windows may occasionally complain that it cannot write to the
          disk and warn you about loss of data.  You can safely ignore it.
          How can you lose data on read-only volumes anyway?

          When VDK opens a virtual disk for read-only access, it properly
          sets a flag to declare the drive as read-only, answers that the
          volume is write protected whenever asked by the system, and
          declines every write request with polite "no, you can't write
          to this volume because it's write protected" status code.
          Still, Windows tries to write.  If Windows does not believe what
          the drivers say and try to write anyway, why does it bother to
          ask in the first place?

        - Windows NT Disk Administrator causes the whole system to freeze
          if you are using a read-only virtual drive.

        - and, I suspect, many more...


    Using NTFS partitions on virtual drives

        Windows NT and Windows 2000 cannot mount NTFS partitions if the
        volume is read-only, so you have to open virtual disks in writable
        mode if you want to use NTFS partitions on it.
        (Windows XP can mount read-only NTFS partitions).

        However you must be very careful when you use NTFS partitions
        created with Windows NT from Windows 2000/XP.

        Windows NT and Windows 2000/XP actually use different versions of
        NTFS (Windows NT uses NTFS 4, Windows 2000/XP uses NTFS 5).
        When Windows 2000/XP finds NTFS 4 partitions it automatically
        upgrades the file system to NTFS 5 WITHOUT EVER ASKING IF THE USER
        WANTS IT.  Actually Windows 2000/XP cannot mount NTFS 4 partitions
        without converting it to NTFS 5.

        On the other hand, Windows NT cannot access NTFS 5 partition at all
        without the Service Pack 4 or later applied to the system, or even
        with proper Service Packs some disk management programs cannot work
        on NTFS 5 partition (chkdsk, certain defragment application, etc).
        And it is impossible to revert the upgrading process.

        Outrageous, isn't it?

        When you want to access NTFS 4 partitions from Windows 2000/XP, do
        not EVER use /RW option.  Use /UNDO or /WB option instead.


HISTORY
=======
    05-06-2021  Version 3.3 (VDK.SYS 3.3, VDK.EXE 3.3)
                - Modernization, cmake and fixes

    06-04-2005  Version 3.2 (VDK.SYS 3.1, VDK.EXE 3.2)
                - Accept virtual disks created with VMware 5.x
                - Accept virtual disks with "lsilogic" virtual controller.
                - Added /SEARCH option to VIEW and OPEN commands.
                - Added /UNDO:path option to OPEN command.

    10-11-2003  Version 3.1 (VDK.SYS 3.1, VDK.EXE 3.1)
                - Fixed a bug that the driver could cause the system to stall
                  during the system shutdown.
                - Fixed a bug that vdk.exe could not read extended partition
                  tables stored beyond 4GB from the beginning of the disk.
                - Added /Q (quiet) and /F (force) options to CLOSE command.
                - Corrected description of required privileges in the help
                  and the doc.

    04-11-2003  Version 3.0 Release (VDK.SYS 3.0, VDK.EXE 3.0)
                Added support for multiple virtual drives, write-block
                mode, sector image file set, view information command.

    30-07-2003  Version 2.0 Release (VDK.SYS 2.0, VDK.EXE 2.0)
                Added support for VMware 4.0 virtual disks, simple sector
                image files, non-partitioned disk image files.

    27-05-2003  VDK.SYS 1.01
                Fixed a major bug where it couldn't handle virtual disks
                larger than 4GB properly.

    20-05-2003  Initial Release (VDK.SYS 1.0, VDK.EXE 1.0)

