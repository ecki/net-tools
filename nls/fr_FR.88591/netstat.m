$set 4  #netstat

$ #_args Original Message:(%s -> %d args)
# %s -> %d args

$ #_netstat Original Message:(netstat: unsupported address family %d !\n)
# netstat: famille d'adresse pas supporté %d !\n

$ #_UNKN Original Message:(UNKNOWN)
# INCONNU

$ #_off Original Message:(off (0.00/%ld))
# off (0.00/%ld)

$ #_on Original Message:(on (%2.2f/%ld))
# on (%2.2f/%ld)

$ #_unkn Original Message:(unkn-%d (%2.2f/%ld))
# inconnu-%d (%2.2f/%ld)

$ #_off2 Original Message:(off (0.00/%ld) %c)
# off (0.00/%ld) %c

$ #_on2 Original Message:(on (%2.2f/%ld) %c)
# on (%2.2f/%ld) %c

$ #_unkn2 Original Message:(unkn-%d (%2.2f/%ld) %c)
# inconnu-%d (%2.2f/%ld) %c

$ #_off3 Original Message:(off (0.00/%ld) %c)
# off (0.00/%ld) %c

$ #_on3 Original Message:(on (%2.2f/%ld) %c)
# on (%2.2f/%ld) %c

$ #_unkn3 Original Message:(unkn-%d (%2.2f/%ld) %c)
# inconnu-%d (%2.2f/%ld) %c

$ #_unix Original Message:(Active UNIX domain sockets\n)
# Prises du domaine UNIX actives\n

$ #_header_unix Original Message:(Proto RefCnt Flags      Type            State           Path\n)
# Proto CptRef Options    Type            Etat            Chemin\n

$ #_noflags Original Message:([NO FLAGS])
# [PAS D'OPTIONS]

$ #_interface Original Message:(Kernel Interface table\n)
# Table des interfaces du noyau\n

$ #_header_iface Original Message:(Iface   MTU Met  RX-OK RX-ERR RX-DRP RX-OVR  TX-OK TX-ERR TX-DRP TX-OVR Flags\n)
# Iface   MTU Met  RX-OK RX-ERR RX-DRP RX-OVR  TX-OK TX-ERR TX-DRP TX-OVR Opts\n

$ #_unkn_iface Original Message:(%s: unknown interface.\n)
# %s: interface inconnue.\n

$ #_usage1  Original Message:(usage: netstat [-veenc] [<Af>] -r            netstat {-V|--version|-h|--help}\n)
# usage: netstat [-veenc] [<Af>] -r            netstat {-V|--version|-h|--help}\n

$ #_usage2  Original Message:(       netstat [-vncao] [<Socket>]\n)
#        netstat [-vncao] [<Socket>]\n

$ #_usage3  Original Message:(       netstat { [-veenac] -i | [-vnc] -N | [-cne] -M }\n\n)
#        netstat { [-veenac] -i | [-vnc] -N | [-cne] -M }\n\n

$ #_usage4  Original Message:(        -r, --routing            display routing table\n)
#         -r, --routing            display routing table\n

$ #_usage5  Original Message:(        -N, --netlink            display netlink kernel messages\n)
#         -N, --netlink            display netlink kernel messages\n

$ #_usage6  Original Message:(        -i, --interfaces         display interface table\n)
#         -i, --interfaces         display interface table\n

$ #_usage7  Original Message:(        -M, --masquerade         display masqueraded connections\n\n)
#         -M, --masquerade         display masqueraded connections\n\n

$ #_usage8  Original Message:(        -v, --verbose            be verbose\n)
#         -v, --verbose            be verbose\n

$ #_usage9  Original Message:(        -n, --numeric            dont resolve names\n)
#         -n, --numeric	           dont resolve names\n

$ #_usage10 Original Message:(        -e, --extend             display other/more informations\n)
#         -e, --extend             display other/more informations\n

$ #_usage11 Original Message:(        -c, --continous          continous lising\n\n)
#         -c, --continous          continous lising\n\n

$ #_usage12 Original Message:(        -a, --all, --listening   display all\n)
#         -a, --all, --listening   display all\n

$ #_usage13  Original Message:(       -o, --timers             display timers\n\n)
#         -o, --timers             display timers\n\n

$ #_usage14  Original Message:(<Socket>={-t|--tcp} {-u|--udp} {-w|--raw} {-x|--unix} --ax25 --ipx --netrom\n)
# <Socket>={-t|--tcp} {-u|--udp} {-w|--raw} {-x|--unix} --ax25 --ipx --netrom\n

$ #_usage15  Original Message:(<Af>= -A {inet|ipx|netrom|ddp|ax25},... --inet --ipx --netrom --ddp --ax25\n)
# <Af>= -A {inet|ipx|netrom|ddp|ax25},... --inet --ipx --netrom --ddp --ax25\n

$ #_internet Original Message:(Active Internet connections)
# Connexions Internet Actives

$ #_servers Original Message:( (including servers))
#  (y compris les serveurs)

$ #_header_internet Original Message:(\nProto Recv-Q Send-Q Local Address          Foreign Address        (State)       User\n)
# \nProto Recv-Q Send-Q Adresse Locale         Adresse Distante       (Etat)        Utilisateur\n

$ #_header_ipx Original Message:(Active IPX sockets\nProto Recv-Q Send-Q Local Address          Foreign Address        (State)       User\n)
# Prises IPX actives\nProto Recv-Q Send-Q Adresse Locale         Adresse Distante       (Etat)        Utilisateur\n

$ #_type_no_route Original Message:(No routing for address family `%s'.\n)
# No routing for address family `%s'.\n

$ #_route_no_support Original Message:(Adress family `%s' not supported.\n)
# Adress family `%s' not supported.\n

$ #_route_not_yet Original Message:(Address family `%s' Not yet supported.\n)
# Address family `%s' Not yet supported.\n

$ #_nlp_title Original Message:(Netlink Kernel Messages)
# Netlink Kernel Messages

$ #_nlp_cnt Original Message:( (continous))
#  (continous)

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
