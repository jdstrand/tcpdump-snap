# tcpdump-snap
Snap package for tcpdump.

To install:

    $ sudo snap install tcpdump

If want to save pcap files to $HOME or /media, optionally:

    $ sudo snap connect tcpdump:home
    $ sudo snap connect tcpdump:removable-media

Use:

    $ sudo tcpdump

