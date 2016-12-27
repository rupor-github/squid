# An alternative for the CAM software for the NZXT Kraken X41/X61 

### Squid
"Standard" Windows Service to control and monitor NZXT liquid cooler (Kraken X41/X61)

### Disclaimer

[NZXT](https://www.nzxt.com/) was **NOT** involved in this project, do **NOT** contact them if your device is damaged while using this software.

I will **NOT** accept any responsibility for any damage done to your hardware.

### Rationale
   I have a desktop built with MSI motherboard, Corsair power supply and NZXT Kraken cooler. Each of these comes with an application
   to control my hardware. MSI has two apps!. Each was build with .NET framework and eats half gigabyte (or more) of RAM drawing
   pretty pictures of computer internals, most of the time doing nothing useful - this is not related to actual work. Out of all
   of them NZXT CAM is most difficult to avoid - if you want to control noise you must have it running. So here is a replacement
   which attempts to get out of the way as much as possible. It does not have user interface (use PerfMon - it provides counters
   for all variables that matter), logs proper events (use EventVwr), knows how to work quietly even when user is not logged
   on and uses 6MB of memory. It would not ask for connection to your Google or Facebook accounts and would not try to upload 
   you data to Azure.

### Details
   To understand what to do I looked at [leviathan](https://github.com/jaksi/leviathan) and analyzed several consecutive CAM 3.0 
   versions. In my code I tried to stick to what CAM does with Kraken as much as possible. Presently this service assumes that 
   you have single CPU (with multiple cores) and single NZXT Kraken X41/X61 cooler (it checks for USB Vendor/Product ID "2433:b200").
   I am not interested in flashing LEDs on pump (my case is closed and sits under the desk) - but this is realtivly simple to add.

### Prerequisites
   To get CPU temperatures CAM is using [CPUID](http://www.cpuid.com/) SDK - so naturally Squid is doing the same. Since you paid
   for NZXT Kraken and NZXT purchased cpuid sdk I am assuming you could extract platform specific version (1.1.4.2) of cpuidskd dll from
   CAMV2.Hardware.dll assembly (part of CAM installation). Use excellent [JetBrains dotPeek](https://www.jetbrains.com/decompiler/) for that,
   its free. Just look under Resources for CAMV2.Hardware.Detect.cpuidsdk.dll or CAMV2.Hardware.Detect.cpuidsdk64.dll (depending on
   Windows you are running). Rename resulting dll to cpuidsdk.dll or cpuidsdk64.dll and place it into the same directory as SquidSrv.exe.

   Kraken presents itself as Asetek VI USB Device (which it is) and CAM communicates with it via old version of Silicon Labs Direct
   Access Drivers. Drivers are part of CAM installation (subdirectory KrakenDriver) and SDK is freely available form Silicon Labs:
   [Direct Access Driver Software](http://www.silabs.com/Support%20Documents/Software/USBXpress_Install.exe). Note: you need legacy
   SDK version 3.51 (yes, it works on Windows 10) to get proper SiUSBXp.dll.

   It could be much simpler if you install CAM first, make sure everything works, copy what is necessary - before trying to run
   Squid. As a bonus it will take care of installing proper Kraken USB divers.

### Important
   Since Kraken device is being opened in exclusive mode you cannot run CAM and Squid simultaneously!

### Installation
   Copy Squid binaries into directory of your choice (I am using "C:\Program Files\Squid"). Add cpuidsdk dll and SiUSBXp.dll (mind
   proper platform - 32 vs 64 bits). Step into this directory. Using elevated command prompt issue following commands (note, that
   lodctr needs FULL path):

   `SquidSrv.exe -install`

   `lodctr "/M:C:\Program Files\Squid\SquidSrv.man`

### Operation
   This is standard Windows service which supports normal controls - use "Services" management control as you would with any other
   service (start, stop, pause, continue).

   Service could be customized if necessary using following registry keys:
   + `HKEY_LOCAL_MACHINE\SYSTEM\CurrentControlSet\Services\Squid\Parameters\General`
     + Hysteresis (REG_DWORD) 0/1 - same as hysteresis in CAM
     + Profile (REG_SZ) - name of currently used temperature profile
   + `HKEY_LOCAL_MACHINE\SYSTEM\CurrentControlSet\Services\Squid\Parameters\Profiles`
     + Performance (REG_BINARY) - 20 byte array with CAM "performance" temperature profile
     + Silent (REG_BINARY) - 20 byte array with CAM "silent" temperature profile
     + Fixed (REG_BINARY) - 20 byte array temperature profile I am using

   Each profile array keeps pump (and fan) speed values in percents, covering 0 to 100 degrees Celcius in 5 degree steps.

When service is running you can observe operational values using Windows PerfMon:

![perfmon view](https://github.com/rupor-github/squid/raw/master/images/perfmon.jpg)

### Word of advice
   I experimented with multiple ways of connecting NZXT Kraken. Quietest and by far the most maintainable and flexible way is to ignore
   NZXT installation manual and connect pump header to any SYSFAN motherboard header (rather than recommended CPUFAN header) while
   connecting Kraken fans directly to motherboard CPUFAN headers (rather than recommended Kraken connectors from pump). Just do not
   forget to set SYSFAN control in BIOS to 100%.


### Building
   Use Visual Studio 2015 Community edition to open and build solution. Do not forget to get UsbExpress header and libraries from [here](http://www.silabs.com/Support%20Documents/Software/USBXpress_Install.exe).
   Just copy content of "SiLabs\MCU\USBXpress\USBXpress_API\Host" directory into usbexpress subdirectory of this project.

### Licenses
   + My code is under MIT license
   + I used freely available [CNTService v1.84](http://www.naughter.com/serv.html) and [PerfCtrv2Mgr v1.02](http://www.naughter.com/download/perfctrv2mgr.zip) sources from [PJ Naughter](http://www.naughter.com/index.html)
   which are under his license.

Enjoy!
