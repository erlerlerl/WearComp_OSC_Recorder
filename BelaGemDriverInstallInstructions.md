# Bela: Enable the Existing RTL8821CU Driver

This procedure applies to the **first Bela**, where the correct Realtek driver is already included in the kernel but was disabled by blacklisting or was simply not loaded.

## 1. Check the kernel and USB device

Check the kernel:

```bash
uname -a
```

The relevant Bela was running:

```text
6.12.49-arm64-ti-evl
```

Check the USB device:

```bash
lsusb
```

The Realtek adapter should appear as:

```text
0bda:c820 Realtek Semiconductor Corp.
```

---

## 2. Check whether the correct driver is available

Search for the relevant kernel modules:

```bash
find /lib/modules/$(uname -r) -iname '*8821cu*' -o -iname '*rtw88*'
```

Also check the specific module:

```bash
modinfo rtw88_8821cu
```

If the driver is included in the kernel, `modinfo` should return information about the module.

---

## 3. Check whether the driver is blacklisted

Search for relevant blacklist entries:

```bash
grep -RniE 'blacklist.*(8821|rtw88)|blacklist.*rtl' /etc/modprobe.d/
```

Additionally:

```bash
grep -Rni 'rtw88_8821cu' /etc/modprobe.d/
```

If you find a line such as:

```text
blacklist rtw88_8821cu
```

it prevents the driver from being loaded automatically.

---

## 4. Identify the blacklist file

Do not delete the entire file.

First list the modprobe configuration files:

```bash
ls -l /etc/modprobe.d/
```

Then inspect the relevant file:

```bash
cat /etc/modprobe.d/<FILE>
```

The line:

```text
blacklist rtw88_8821cu
```

should be removed or commented out.

For example:

```text
# blacklist rtw88_8821cu
```

---

## 5. Update the initramfs

After changing the modprobe configuration:

```bash
sudo update-initramfs -u
```

---

## 6. Load the driver manually

You can test the driver without rebooting:

```bash
sudo modprobe rtw88_8821cu
```

Then check:

```bash
lsmod | grep rtw88
```

You should see the relevant `rtw88` modules.

---

## 7. Check whether a Wi-Fi interface appeared

Run:

```bash
ip link
```

and:

```bash
iw dev
```

You should now see a wireless interface such as:

```text
wlan0
```

or another `wl...` interface name.

---

## 8. Check the kernel log

If no Wi-Fi interface appears:

```bash
dmesg | grep -iE 'rtw88|8821|firmware|wlan'
```

Pay particular attention to messages concerning:

* `rtw88_8821cu`
* firmware
* USB
* probe/bind
* initialization errors

---

## 9. Verify after reboot

If the driver works when loaded manually:

```bash
sudo reboot
```

After reboot:

```bash
lsmod | grep rtw88
```

and:

```bash
iw dev
```

The important test is whether the driver is now **loaded automatically** and the Wi-Fi interface is available.

---

# Important: First Bela vs. mini01

This procedure is for the Bela that already has the kernel driver:

```text
rtw88_8821cu
```

It is **not** for `mini01`.

`mini01` uses:

```text
4.14.108-ti-xenomai-r143
```

and requires the separately compiled:

```text
8821cu
```

driver because of the older kernel.

Do not mix the two procedures:

| Bela       | Kernel                     | Procedure                                                   |
| ---------- | -------------------------- | ----------------------------------------------------------- |
| First Bela | `6.12.49-arm64-ti-evl`     | Enable the existing `rtw88_8821cu` driver / check blacklist |
| `mini01`   | `4.14.108-ti-xenomai-r143` | Build and install the external `8821cu` driver              |
Document Title

