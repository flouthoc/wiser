# wiser

 A very minimal type-2 hypervisor built using Linux Kernel Virtual Machine for Linux.
 
 Following project is under-development expect unfinished components.
 
 ![Image](../main/assets/wiser.png?raw=true) 

## Usage

User needs to download and build kernel images from https://www.kernel.org/.


```bash 
Usage: wiser [OPTION...] --image path-to-kernel-image
wiser - Extremely tiny type-2 hypervisor for linux. Will boot your
unikernel/linux someday.

  -c, --vcpu                 Number of cpu for your vm
  -i, --image=IMAGE          linux kernel bzImage
  -r, --memory               Ram size for your vm
  -v, --verbose              Produce verbose output
  -?, --help                 Give this help list
      --usage                Give a short usage message
  -V, --version              Print program version

Mandatory or optional arguments to long options are also mandatory or optional
for any corresponding short options.

Report bugs to https://github.com/flouthoc/wiser/issues.
```

* #### image
Path to linux kernel bzImage. The bzImage file is in a specific format. It contains concatenated ```bootsect.o + setup.o + misc.o + piggy.o```.

## Roadmap
* Allow users to load initramfs.

## References
* https://github.com/kvmtool/kvmtool
* https://gitlab.prognosticlab.org/debashis/palacios
* https://wiki.osdev.org/Main_Page
* https://c9x.me/x86/
