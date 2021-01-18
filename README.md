# tcpdump-snap
Snap package for tcpdump.

## Installation

To install:

    $ sudo snap install tcpdump

## Usage

    $ sudo tcpdump

tcpdump will drop privileges to the ```snap_daemon``` user and group by
default. To write out captures, the ```/var/snap/tcpdump/common/captures```
directory is created with ```snap_daemon``` ownership. Eg:

    $ sudo tcpdump -w /var/snap/tcpdump/common/captures/my.pcap

If you prefer, you may use ```-Z root``` to prevent dropping privileges and
then write out anywhere in the snap's writable areas.

If want to save pcap files to $HOME or /media, optionally:

    $ sudo snap connect tcpdump:home
    $ sudo snap connect tcpdump:removable-media

In these cases, you will need to create a directory that ```snap_daemon``` can
write to. Eg:

    $ mkdir -m 770 $HOME/captures
    $ sudo chown root:snap-daemon $HOME/captures
    $ sudo tcpdump -w $HOME/captures/my.pcap

## Integration with other snaps

This snap exports the ```/var/snap/tcpdump/common/captures``` as a writable
area via the content interface. Other snaps may specify in their
```snap.yaml```:

    plugs:
      captures:
        interface: content
        target: $SNAP_DATA/tcpdump-captures

Then when install that snap:

    $ sudo snap connect <yoursnap>:captures tcpdump:captures

See https://forum.snapcraft.io/t/the-content-interface for details.
