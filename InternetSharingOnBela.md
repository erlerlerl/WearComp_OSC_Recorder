# Sharing a Mac's Internet Connection with a Bela via Ethernet/USB

This guide describes how to give a Bela internet access through a Mac without using macOS Internet Sharing.

The setup is:

```text
                    Internet
                       │
                    Wi-Fi
                      en0
                  192.168.1.17
                       │
                       │  macOS routing + NAT
                       │
                 ┌─────┴─────┐
                 │    Mac    │
                 └─────┬─────┘
                       │
                     en15
                  192.168.7.1
                       │
                    USB/Ethernet
                       │
                     Bela
                  192.168.7.2
```

The Bela uses `192.168.7.1` as its gateway.

---

## 1. Configure the Mac's Bela interface

Identify the Mac interface connected to the Bela:

```bash
ifconfig en15
```

Configure it with:

```bash
sudo ifconfig en15 192.168.7.1 netmask 255.255.255.0 up
```

Verify:

```bash
ifconfig en15
```

You should see:

```text
inet 192.168.7.1 netmask 0xffffff00
status: active
```

Do **not** enable macOS Internet Sharing. Internet Sharing can take control of the interface and replace the manually configured address with a bridge such as `bridge100`.

---

## 2. Configure the Bela

On the Bela, configure the interface connected to the Mac as:

```text
IP address: 192.168.7.2
Netmask:    255.255.255.0
Gateway:    192.168.7.1
```

For example, in `/etc/network/interfaces`:

```text
auto usb1
iface usb1 inet static
    address 192.168.7.2
    netmask 255.255.255.0
    network 192.168.7.0
    gateway 192.168.7.1
```

Check the routing table:

```bash
ip route
```

You should have:

```text
default via 192.168.7.1 dev usb1
192.168.7.0/24 dev usb1 ...
```

---

## 3. Test the local connection

From the Bela:

```bash
ping -c 3 192.168.7.1
```

This must work before troubleshooting internet access.

---

## 4. Enable IP forwarding on macOS

Check:

```bash
sysctl net.inet.ip.forwarding
```

If it says:

```text
net.inet.ip.forwarding: 0
```

enable forwarding:

```bash
sudo sysctl -w net.inet.ip.forwarding=1
```

Verify:

```bash
sysctl net.inet.ip.forwarding
```

It should say:

```text
net.inet.ip.forwarding: 1
```

---

## 5. Create the PF NAT rule

Create a small PF configuration:

```bash
sudo sh -c 'cat > /tmp/bela-nat.conf <<EOF
nat on en0 from 192.168.7.0/24 to any -> (en0)
EOF'
```

Here:

* `en0` = Mac's internet/Wi-Fi interface
* `192.168.7.0/24` = Bela network

Load the NAT rule into an anchor:

```bash
sudo pfctl -a bela-nat -Nf /tmp/bela-nat.conf
```

Verify:

```bash
sudo pfctl -a bela-nat -sn
```

Expected:

```text
nat on en0 inet from 192.168.7.0/24 to any -> (en0) round-robin
```

---

## 6. Important: PF must actually be enabled

Check:

```bash
sudo pfctl -s info
```

If it says:

```text
Status: Disabled
```

enable PF:

```bash
sudo pfctl -e
```

Then verify:

```bash
sudo pfctl -s info
```

You need:

```text
Status: Enabled
```

---

## 7. Attach the NAT anchor to the main PF ruleset

Simply creating the `bela-nat` anchor is not enough. It must be referenced by `/etc/pf.conf`.

Edit:

```bash
sudo nano /etc/pf.conf
```

Find:

```text
nat-anchor "com.apple/*"
```

and add immediately below it:

```text
nat-anchor "bela-nat"
```

So that section becomes:

```text
scrub-anchor "com.apple/*"
nat-anchor "com.apple/*"
nat-anchor "bela-nat"
rdr-anchor "com.apple/*"
dummynet-anchor "com.apple/*"
anchor "com.apple/*"
```

Check:

```bash
grep -n 'nat-anchor' /etc/pf.conf
```

You should see something like:

```text
23:nat-anchor "com.apple/*"
24:nat-anchor "bela-nat"
```

---

## 8. Reload PF

Reload the main PF configuration:

```bash
sudo pfctl -f /etc/pf.conf
```

A warning similar to this may appear:

```text
Use of -f option, could result in flushing of rules...
```

This is a standard `pfctl` warning. Verify that the NAT rule is still present:

```bash
sudo pfctl -a bela-nat -sn
```

It should show:

```text
nat on en0 inet from 192.168.7.0/24 to any -> (en0) round-robin
```

---

## 9. Test internet access from the Bela

First test an IP address:

```bash
ping -c 3 8.8.8.8
```

Then test DNS:

```bash
ping -c 3 deb.debian.org
```

If both work, the Bela has internet access.

You can now run:

```bash
sudo apt-get update
```

and install/build packages normally.

---

# Troubleshooting

## Bela can ping 192.168.7.1 but not 8.8.8.8

On the Mac check:

```bash
sysctl net.inet.ip.forwarding
```

It must be:

```text
net.inet.ip.forwarding: 1
```

Then:

```bash
sudo pfctl -s info
```

PF must say:

```text
Status: Enabled
```

Then:

```bash
sudo pfctl -a bela-nat -sn
```

The NAT rule must be present.

---

## Use tcpdump to see where packets stop

On the Mac:

```bash
sudo tcpdump -ni en15 'host 8.8.8.8'
```

and in another terminal:

```bash
sudo tcpdump -ni en0 'host 8.8.8.8'
```

Then on the Bela:

```bash
ping -c 3 8.8.8.8
```

A successful NAT setup should eventually show traffic leaving `en0` with the Mac's Wi-Fi address rather than the Bela's `192.168.7.2` address.

For example, the outgoing packet should look conceptually like:

```text
192.168.1.17 > 8.8.8.8
```

rather than:

```text
192.168.7.2 > 8.8.8.8
```

---

# Important: macOS Internet Sharing

Do **not** enable:

**System Settings → General → Sharing → Internet Sharing**

for this setup.

Internet Sharing may:

* remove `192.168.7.1` from `en15`
* create `bridge100`
* assign a different subnet such as `192.168.2.1`
* take control of the forwarding/NAT configuration

The manual setup above deliberately keeps `en15` as a normal routed interface with:

```text
Mac en15: 192.168.7.1
Bela:      192.168.7.2
```

---

# Minimal working recipe

Once the configuration is understood, the essential commands are:

### Mac

```bash
sudo ifconfig en15 192.168.7.1 netmask 255.255.255.0 up
sudo sysctl -w net.inet.ip.forwarding=1
sudo pfctl -e
sudo pfctl -a bela-nat -Nf /tmp/bela-nat.conf
sudo pfctl -f /etc/pf.conf
```

with `/etc/pf.conf` containing:

```text
nat-anchor "bela-nat"
```

and `/tmp/bela-nat.conf` containing:

```text
nat on en0 from 192.168.7.0/24 to any -> (en0)
```

### Bela

```text
IP:      192.168.7.2
Netmask: 255.255.255.0
Gateway: 192.168.7.1
```

Then:

```bash
ping -c 3 8.8.8.8
sudo apt-get update
```

---

## Persistence after reboot

The configuration we established manually has two important pieces that may need to be made persistent:

1. macOS IP forwarding:

   ```text
   net.inet.ip.forwarding = 1
   ```

2. PF must be enabled and the `bela-nat` rule must be loaded.

The Bela's `192.168.7.2` address and gateway can be made persistent through `/etc/network/interfaces`, as described above.

For a permanent Mac setup, it is better to create a small launch mechanism for the forwarding/PF configuration rather than relying on manually running the commands after every reboot.
Document Title

