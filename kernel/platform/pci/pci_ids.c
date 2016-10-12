#include <pci/pci.h>

/* Taken from http://pciids.sourceforge.net/v2.2/pci.ids */

static const pci_device_id pci_vendor_1013[] = {
  { PCI_DEVICE_ID_CIRRUS_5446, "GD 5446" },
  { 0, 0 }
};

static const pci_device_id pci_vendor_1022[] = {
  { PCI_DEVICE_ID_AMD_LANCE, "79c970 [PCnet32 LANCE]" },
  { 0, 0 }
};

static const pci_device_id pci_vendor_11ab[] = {
  { PCI_DEVICE_ID_MARVELL_GT64120, "GT-64120/64120A/64121A System Controller" },
  { 0, 0 }
};

static const pci_device_id pci_vendor_8086[] = {
  { PCI_DEVICE_ID_INTEL_82371AB_0,  "82371AB/EB/MB PIIX4 ISA" },
  { PCI_DEVICE_ID_INTEL_82371AB,    "82371AB/EB/MB PIIX4 IDE" },
  { PCI_DEVICE_ID_INTEL_82371AB_2,  "82371AB/EB/MB PIIX4 USB" },
  { PCI_DEVICE_ID_INTEL_82371AB_3,  "82371AB/EB/MB PIIX4 ACPI" },
  { PCI_DEVICE_ID_INTEL_82540EM,    "82540EM e1000 Network Adapter"},
  { 0, 0 }
};

static const pci_device_id pci_vendor_1af4[] = {
    { PCI_DEVICE_ID_VIRTIO_NETWORK_DEVICE,  "VirtIO Network Device"         },
    { PCI_DEVICE_ID_VIRTIO_BLOCK_DEVICE,    "VirtIO Block Device"           },
    { PCI_DEVICE_ID_VIRTIO_CONSOLE,         "VirtIO Console"                },
    { PCI_DEVICE_ID_VIRTIO_MEMORY_BALLOON,  "VirtIO Memory Balloon"         },
    { PCI_DEVICE_ID_VIRTIO_SCSI,            "VirtIO SCSI"                   },
    { PCI_DEVICE_ID_VIRTIO_FILESYSTEM,      "VirtIO Filesystem"             },
    { PCI_DEVICE_ID_VIRTIO_GPU,             "VirtIO GPU"                    },
    { PCI_DEVICE_ID_VIRTIO_INPUT,           "VirtIO Input"                  },
    { PCI_DEVICE_ID_VIRTIO_SHARED_MEMORY,   "VirtIO Inter-VM shared memory" },
    { 0, 0 }
};

const pci_vendor_id pci_vendor_list[] = {
  { PCI_VENDOR_ID_CIRRUS,           "Cirrus Logic", pci_vendor_1013 },
  { PCI_VENDOR_ID_AMD,              "Advanced Micro Devices, Inc.", pci_vendor_1022 },
  { PCI_VENDOR_ID_MARVELL,          "Marvell Technology Group Ltd.", pci_vendor_11ab },
  { PCI_VENDOR_ID_INTEL,            "Intel Corporation", pci_vendor_8086 },
  { PCI_VENDOR_ID_REDHAT_QUMRANET,  "Red Hat, Inc", pci_vendor_1af4 },
  { 0, 0, 0 }
};

const char *pci_class_code[] = {
  "",
  "Mass Storage Controller",
  "Network Controller",
  "Display Controller",
  "Multimedia Controller",
  "Memory Controller",
  "Bridge Device",
  "Simple Communication Controllers",
  "Base System Peripherals",
  "Input Devices",
  "Docking Stations",
  "Processors",
  "Serial Bus Controllers",
  "Wireless Controllers",
  "Intelligent I/O Controllers",
  "Satellite Communication Controllers",
  "Encryption/Decryption Controllers",
  "Data Acquisition and Signal Processing Controllers"
};


