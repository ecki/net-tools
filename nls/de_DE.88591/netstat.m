$set 4  #netstat

$ #_header Original Message:(Destination     Gateway         Genmask         Flags Metric Ref Use    Iface\n)
# Ziel            Gateway         Maske           Optn  Metrik Ref Ben    Iface\n

$ #_args Original Message:(%s -> %d args)
# %s -> %d Argumente

$ #_netstat Original Message:(netstat: unsupported address family %d !\n)
# netstat: Adressfamilie %d nicht unterstützt !\n

$ #_UNKN Original Message:(UNKNOWN)
# UNBEKANNT

$ #_off Original Message:(off (0.00/%ld))
# aus (0.00/%ld)

$ #_on Original Message:(on (%2.2f/%ld))
# an (%2.2f/%ld)

$ #_unkn Original Message:(unkn-%d (%2.2f/%ld))
# unbek-%d (%2.2f/%ld)

$ #_off2 Original Message:(off (0.00/%ld) %c)
# aus (0.00/%ld) %c

$ #_on2 Original Message:(on (%2.2f/%ld) %c)
# an (%2.2f/%ld) %c

$ #_unkn2 Original Message:(unkn-%d (%2.2f/%ld) %c)
# unbek-%d (%2.2f/%ld) %c

$ #_off3 Original Message:(off (0.00/%ld) %c)
# aus (0.00/%ld) %c

$ #_on3 Original Message:(on (%2.2f/%ld) %c)
# an (%2.2f/%ld) %c

$ #_unkn3 Original Message:(unkn-%d (%2.2f/%ld) %c)
# unbek-%d (%2.2f/%ld) %c

$ #_unix Original Message:(Active UNIX domain sockets\n)
# Aktive UNIX Domain-Sockets\n

$ #_header_unix Original Message:(Proto RefCnt Flags      Type            State           Path\n)
# Proto RefAnz Optn       Typ             Zustand         Pfad\n

$ #_noflags Original Message:([NO FLAGS])
# [KEINE OPTIONEN]

$ #_interface Original Message:(Kernel Interface table\n)
# Kernel Schnittstellentabelle\n

$ #_header_iface Original Message:(Iface   MTU Met  RX-OK RX-ERR RX-DRP RX-OVR  TX-OK TX-ERR TX-DRP TX-OVR Flags\n)
# Iface   MTU Met  RX-OK RX-ERR RX-DRP RX-OVR  TX-OK TX-ERR TX-DRP TX-OVR Flags\n

$ #_unkn_iface Original Message:(%s: unknown interface.\n)
# %s: unbekannte Schnittstelle.\n


$ #_usage1  Original Message:(usage: netstat [-veenc] [<Af>] -r            netstat {-V|--version|-h|--help}\n)
# usage: netstat [-veenc] [<Af>] -r            netstat {-V|--version|-h|--help}\n

$ #_usage2  Original Message:(       netstat [-vncao] [<Socket>]\n)
#        netstat [-vncao] [<Socket>]\n

$ #_usage3  Original Message:(       netstat { [-veenac] -i | [-vnc] -N | [-cne] -M }\n\n)
#        netstat { [-veenac] -i | [-vnc] -N | [-cne] -M }\n\n

$ #_usage4  Original Message:(        -r, --routing            display routing table\n)
#         -r, --routing            zeige Routingtabelle an\n

$ #_usage5  Original Message:(        -N, --netlink            display netlink kernel messages\n)
#         -N, --netlink            zeige Netlink Kernelnachrichten an\n

$ #_usage6  Original Message:(        -i, --interfaces         display interface table\n)
#         -i, --interfaces         zeige Interfacetabelle an\n

$ #_usage7  Original Message:(        -M, --masquerade         display masqueraded connections\n\n)
#         -M, --masquerade         display masqueraded connections\n\n

$ #_usage8  Original Message:(        -v, --verbose            be verbose\n)
#         -v, --verbose            sei gesprächig\n

$ #_usage9  Original Message:(        -n, --numeric            dont resolve names\n)
#         -n, --numeric	           Namen nur numerisch ausgeben\n

$ #_usage10 Original Message:(        -e, --extend             display other/more informations\n)
#         -e, --extend             Andere/Mehr Infos ausgeben\n

$ #_usage11 Original Message:(        -c, --continous          continous lising\n\n)
#         -c, --continous          Fortwährend anzeigen\n\n

$ #_usage12 Original Message:(        -a, --all, --listening   display all\n)
#         -a, --all, --listening   Alle anzeigen\n

$ #_usage13  Original Message:(       -o, --timers             display timers\n\n)
#         -o, --timers             Timer auch anzeigen\n\n

$ #_usage14  Original Message:(<Socket>={-t|--tcp} {-u|--udp} {-w|--raw} {-x|--unix} --ax25 --ipx --netrom\n)
# <Socket>={-t|--tcp} {-u|--udp} {-w|--raw} {-x|--unix} --ax25 --ipx --netrom\n

$ #_usage15  Original Message:(<Af>= -A {inet|ipx|netrom|ddp|ax25},... --inet --ipx --netrom --ddp --ax25\n)
# <Af>= -A {inet|ipx|netrom|ddp|ax25},... --inet --ipx --netrom --ddp --ax25\n

$ #_internet Original Message:(Active Internet connections)
# Aktive Internetverbindungen

$ #_servers Original Message:( (including servers))
#  (einschl. Server)

$ #_header_internet Original Message:(\nProto Recv-Q Send-Q Local Address          Foreign Address        (State)       User\n)
# \nProto Empf-Q Send-Q Lokale Adresse         Fremde Adresse         (Zustand)     Benutzer\n

$ #_header_ipx Original Message:(Active IPX sockets\nProto Recv-Q Send-Q Local Address          Foreign Address        (State)       User\n)
# Aktive IPX-Sockets\nProto Empf-Q Send-Q Lokale Adresse         Fremde Adresse         (Zustand)     Benutzer\n

$ #_type_no_route Original Message:(No routing for address family `%s'.\n)
# Kein Routing für Adressfamilie `%s'.\n

$ #_route_no_support Original Message:(Adress family `%s' not supported.\n)
# Adressfamilie `%s' noch nicht unterstützt.\n

$ #_route_not_yet Original Message:(Address family `%s' Not yet supported.\n)
# Adaressfamilie `%s' noch nicht unterstützt.\n

$ #_nlp_title Original Message:(Netlink Kernel Messages)
# Netlink Kernelnachrichten

$ #_nlp_cnt Original Message:( (continous))
#  (fortwährend)

$ #_noax25 Original Message:(AX.25 not configured in this system.\n)
# AX.25 not configured in this system.\n

$ #_ax25 Original Message:(Kernel AX.25 routing table\n)
# Kernel AX.25 routing table\n

$ #_header_ax25 Original Message:(Destination  Iface    Use\n)
# Destination  Iface    Use\n

$ #_nonetrom Original Message:(NET/ROM not configured in this system.\n)
# NET/ROM not configured in this system.\n

$ #_netrom Original Message:(Kernel NET/ROM routing table\n)
# Kernel NET/ROM routing table\n

$ #_header_netrom Original Message:(Destination  Mnemonic  Quality  Neighbour  Iface\n)
# Destination  Mnemonic  Quality  Neighbour  Iface\n
