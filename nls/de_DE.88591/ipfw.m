$set 9  #ipfw

$ #_ipf_blocking Original Message:(blocking)
# blockiert

$ #_ipf_fwding Original Message:(forwarding)
# weiterleitend

$ #_ipf_accnting Original Message:(accounting)
# abrechnend

$ #_ipf_msqrading Original Message:(masquerading)
# verkleidend

$ #_usage1 Original Message:(usage:\tipfw [-ndhcVr] [--version] [--help]\n\t     [--numeric] [--count] [--reset] [--debug] [--append]\n)
# usage:\tipfw [-ndhcVr] [--version] [--help]\n\t     [--numeric] [--count] [--reset] [--debug] [--append]\n

$ #_usage2 Original Message:(\tipfw p[olicy]      {b[locking]|f[orwarding]} {accept|deny|reject}\n)
# \tipfw p[olicy]      {b[locking]|f[orwarding]} {accept|deny|reject}\n

$ #_usage3 Original Message:(\tipfw [-nrc] l[ist] {b[locking]|f[orwarding]|a[ccounting]}\n)
# \tipfw [-nrc] l[ist] {b[locking]|f[orwarding]|a[ccounting]}\n

$ #_usage4 Original Message:(\tipfw f[lush]       {b[locking]|f[orwarding]|a[ccounting]}\n)
# \tipfw f[lush]       {b[locking]|f[orwarding]|a[ccounting]}\n

$ #_usage5 Original Message:(\tipfw {a[dd]|d[el]} {b[locking]|f[orwarding]} {accept|deny|reject} \n\t     Type [iface Addr] from Src to Dst [flags Flags]\n)
# \tipfw {a[dd]|d[el]} {b[locking]|f[orwarding]} {accept|deny|reject} \n\t     Type [iface Addr] from Src to Dst [flags Flags]\n

$ #_usage6 Original Message:(\tipfw c[heck]       {b[locking]|f[orwarding]}\n\t     Type [iface Addr] from Src to Dst [flags Flags]\n)
# \tipfw c[heck]       {b[locking]|f[orwarding]}\n\t     Type [iface Addr] from Src to Dst [flags Flags]\n

$ #_usage7 Original Message:(\tipfw {a[dd]|d[el]} a[ccounting]\n\t     Type [iface Addr] from Src to Dst [flags Flags]\n)
# \tipfw {a[dd]|d[el]} a[ccounting]\n\t     Type [iface Addr] from Src to Dst [flags Flags]\n

$ #_usage8 Original Message:(\tipfw z[ero] {b[locking]|f[orwarding]|a[ccounting]}\n)
# \tipfw z[ero] {b[locking]|f[orwarding]|a[ccounting]}\n

$ #_usage9 Original Message:(\tipfw {a[dd]|d[el]} m[asquerade] Type from Src to Dst\n\n)
# \tipfw {a[dd]|d[el]} m[asquerade] Type from Src to Dst\n\n

$ #_usage10 Original Message:(\tType={udp|tcp}:\t\tFlags={bidir|syn|ack|prn} ...\n)
# \tType={udp|tcp}:\t\tFlags={bidir|syn|ack|prn} ...\n

$ #_usage11 Original Message:(\tSrc,Dst={1.2.3.4/24|Host|Netname} [[Port1:Port2] Port3 ... Port10]\n\n)
# \tSrc,Dst={1.2.3.4/24|Host|Netname} [[Port1:Port2] Port3 ... Port10]\n\n

$ #_usage12 Original Message:(\tType={icmp}:\t\tFlags={bidir,prn}\n)
# \tType={icmp}:\t\tFlags={bidir,prn}\n

$ #_usage13 Original Message:(\tSrc={1.2.3.4/24|Host|Netname} [[Type1:Type2] Type3 ... Type10]\n)
# \tSrc={1.2.3.4/24|Host|Netname} [[Type1:Type2] Type3 ... Type10]\n

$ #_usage14 Original Message:(\tDst={1.2.3.4/24|Host|Netname}\n\n)
# \tDst={1.2.3.4/24|Host|Netname}\n\n

$ #_usage15 Original Message:(\tType={all}:\t\tFlags={bidir,prn}\n)
# \tType={all}:\t\tFlags={bidir,prn}\n

$ #_usage16 Original Message:()
# \tSrc,Dst={1.2.3.4/24|Host|Netname}\n

$ #_range_set Original Message:(ipfw: range flag set but only %d ports\n)
# ipfw: Bereichsoption gesetzt, aber nur %d Ports\n

$ #_unkn Original Message:(unknown command (%d) passed to do_setsockopt - bye!\n)
# unbekannter Befehl (%d) an do_setsockopt - tschüssi!\n

$ #_ip Original Message:(ip header length %d, should be %d\n)
# ip Headerlänge %d, sollte %d sein\n

$ #_data_ip Original Message:(data = struct iphdr : struct %shdr {\n)
# data = struct iphdr : struct %shdr {\n

$ #_data_ipfw Original Message:(data = struct ip_fw {\n)
# data = struct ip_fw {\n

$ #_accept Original Message:(\taccept )
# \taccept 

$ #_deny Original Message:(\tdeny )
# \tdeny 

$ #_univ Original Message:(\tuniversal\n)
# \tuniversal\n

$ #_missing Original Message:(ipfw: missing protocol name\n)
# ipfw: Protokollname fehlt\n

$ #_illegal Original Message:(illegal protocol name \"%s\"\n)
# ungültiger Protokollname "%s"\n

$ #_missing_ip Original Message:(ipfw: missing ip address\n)
# ipfw: IP-Adresse fehlt\n

$ #_periods Original Message:(ipfw: two periods in a row in ip address (%s)\n)
# ipfw: zwei Punkte hintereinander in IP-Adresse (%s)\n

$ #_unkn_host Original Message:(ipfw: unknown host \"%s\"\n)
# ipfw: unbekannter Rechner "%s"\n

$ #_addr_length Original Message:(ipfw: hostentry addr length = %d, expected %d)
# ipfw: Rechnereintrag Adreßlänge = %d, erwartet %d

$ #_matched Original Message:(ipfw: Only %d fields matched in IP address!\n)
# ipfw: Nur %d Felder gefunden in IP-Adresse!\n

$ #_too_large Original Message:(ipfw: number too large in ip address (%s)\n)
# ipfw: Zahl zu groß in IP-Adresse (%s)\n

$ #_inc_format Original Message:(ipfw: incorrect ip address format \"%s\" (expected 3 periods)\n)
# ipfw: ungültiges IP-Adreßformat "%s" (habe 3 Punkte erwartet)\n

$ #_not_allowed Original Message:(ipfw: ip netmask not allowed here (%s)\n)
# ipfw: IP-Netzmaske hier nicht erlaubt (%s)\n

$ #_missing_mask Original Message:(ipfw: missing mask value (%s)\n)
# ipfw: fehlender Maskenwert (%s)\n

$ #_non_num Original Message:(ipfw: non-numeric mask value (%s)\n)
# ipfw: nichtnumerischer Maskenwert (%s)\n

$ #_junk_mask Original Message:(ipfw: junk after mask (%s)\n)
# ipfw: Schrott hinter Maske (%s)\n

$ #_out_range Original Message:(ipfw: mask length value out of range (%s)\n)
# ipfw: Maskenlänge nicht im gültigen Bereich (%s)\n

$ #_junk_ip Original Message:(ipfw: junk after ip address (%s)\n)
# ipfw: Schrott hinter IP-Adresse (%s)\n

$ #_illegal_port Original Message:(ipfw: illegal port number (%s)\n)
# ipfw: ungültige Portnummer (%s)\n

$ #_portnum_out Original Message:(ipfw: port number out of range (%d)\n)
# ipfw: Portnummer nicht im gültigen Bereich (%d)\n

$ #_unkn_service Original Message:(ipfw: unknown %s service \"%s\"\n)
# ipfw: unbekannter %s Dienst "%s"\n

$ #_too_port Original Message:(ipfw: too many port numbers (max %d, got at least %d, next parm="%s")\n)
# ipfw: zuviele Portnummern (max %d, bekam mindestens %d, nächster Wert="%s")\n

$ #_port_ranges Original Message:(port ranges are only allowed for the first port value pair (%s)\n)
# Portbereiche sind nur für das erste Portwertepaar erlaubt (%s)\n

$ #_no_range Original Message:(ipfw: port range not allowed here (%s)\n)
# ipfw: Portbereich hier nicht erlaubt (%s)\n

$ #_missing_port Original Message:(ipfw: missing port number%s\n)
# ipfw: fehlende Portnummer\n

$ #_nomore_port Original Message:(ipfw: not enough port numbers (expected %d, got %d)\n)
# ipfw: nicht genügend viele Portnummern (erwartet %d, bekam %d)\n

$ #_check_blocking Original Message:(blocking)
# blocking

$ #_check_forwarding Original Message:(forwarding)
# forwarding

$ #_check Original Message:(check %s )
# check %s 

$ #_only_check Original Message:(ipfw: can only check TCP or UDP packets\n)
# ipfw: kann nur TCP- oder UDP-Packete überprüfen\n

$ #_missing_from Original Message:(ipfw: missing \"from\" keyword\n)
# ipfw: fehlendes "from"\n

$ #_expect_from Original Message:(ipfw: expected \"from\" keyword, got \"%s\"\n)
# ipfw: habe "from" erwartet, bekam "%s"\n

$ #_missing_to Original Message:(ipfw: missing \"to\" keyword\n)
# ipfw: fehlendes "to"\n

$ #_expect_to Original Message:(ipfw: expected \"to\" keyword, got \"%s\"\n)
# ipfw: habe "to" erwaretet, bekam "%s"\n

$ #_paq_accept Original Message:(packet accepted by %s firewall\n)
# Packet von %s Firewall akzeptiert\n

$ #_blocking Original Message:(blocking)
# blocking

$ #_forwarding Original Message:(forwarding)
# forwarding

$ #_paq_reject Original Message:(packet rejected by %s firewall\n)
# Packet von %s Firewall abgewiesen\n

$ #_extra Original Message:(ipfw: extra parameters at end of command ()
# ipfw: überflüssige Parameter am Ende des Befehls (

$ #_usage21 Original Message:(usage: ipfirewall add %s ...\n)
# usage: ipfirewall add %s ...\n

$ #_add Original Message:(add %s )
# add %s 

$ #_missing_acc Original Message:(ipfw: missing \"accept\" or \"deny\" keyword\n)
# ipfw: fehlendes "accept" oder "deny"\n

$ #_expect_acc Original Message:(ipfw: expected \"accept\", \"deny\" or \"reject\", got \"%s\"\n)
# ipfw: habe "accept", "deny" oder "reject" erwartet, bekam "%s"\n

$ #_missing_proto Original Message:(ipfw: missing protocol name.\n)
# ipfw: fehlender Protokollname.\n

$ #_missing_iface Original Message:(ipfw: missing interface address.\n)
# ipfw: fehlender Schnittstellenname.\n

$ #_invalid_iface Original Message:(Invalid interface address.\n)
# Ungültige Schnittstellenadresse.\n

$ #_missing_from2 Original Message:(ipfw: missing \"from\" keyword\n)
# ipfw: fehlendes "from"\n

$ #_expect_from2 Original Message:(ipfw: expected \"from\", got \"%s\"\n)
# ipfw: habe "from" erwartet, bekam "%s"\n

$ #_missing_to2 Original Message:(ipfw: missing \"to\" keyword\n)
# ipfw: fehlendes "to"\n

$ #_expect_to2 Original Message:(ipfw: expected \"to\", got \"%s\"\n)
# ipfw: habe "to" erwartet, bekam "%s"\n

$ #_extra2 Original Message:(ipfw: extra parameters at end of command ()
# ipfw: überflüssige Parameter am Ende des Befehls (

$ #_usage22 Original Message:(usage: ipfirewall delete %s ...\n)
# usage: ipfirewall delete %s ...\n

$ #_delete Original Message:(delete %s )
# delete %s 

$ #_missing_acc2 Original Message:(ipfw: missing \"accept\" or \"deny\" keyword\n)
# ipfw: fehlendes "accept" oder "deny"\n

$ #_expect_acc2 Original Message:(ipfw: expected \"accept\" or \"deny\", got \"%s\"\n)
# ipfw: habe "accept" oder "deny" erwartet, bekam "%s"\n

$ #_missing_proto2 Original Message:(ipfw: missing protocol name.\n)
# ipfw: fehlender Protokollname.\n

$ #_missing_iface2 Original Message:(ipfw: missing interface address.\n)
# ipfw: fehlende Schnittstellenadresse.\n

$ #_invalid_iface2 Original Message:(Invalid interface address.\n)
# Ungültige Schnittstellenadresse.\n

$ #_missing_from3 Original Message:(ipfw: missing \"from\" keyword\n)
# ipfw: fehlendes "from"\n

$ #_expect_from3 Original Message:(ipfw: expected \"from\", got \"%s\"\n)
# ipfw: habe "from" erwartet, bekam "%s"\n

$ #_missing_to3 Original Message:(ipfw: missing \"to\" keyword\n)
# ipfw: fehlendes "to"\n

$ #_expect_to3 Original Message:(ipfw: expected \"to\", got \"%s\"\n)
# ipfw: habe "to" erwartet, bekam "%s"\n

$ #_extra3 Original Message:(ipfw: extra parameters at end of command ()
# ipfw: überflüssige Parameter am Ende des Befehls (

$ #_anywhere Original Message:(anywhere)
# irgendwo

$ #_bytes Original Message:(  Packets    Bytes  )
#   Packete    Bytes  

$ #_proto Original Message:(Type    Proto Flags From                To                  Iface               Ports\n)
# Type    Proto Flags From                To                  Iface               Ports\n

$ #_list_accept Original Message:(accept  )
# accept  

$ #_list_deny Original Message:(deny    )
# deny    

$ #_list_any Original Message:(any)
# any

$ #_expect_kwds Original Message:(blocking, forwarding or accounting keyword expected.\n)
# blocking, forwarding oder accounting erwartet.\n

$ #_found_kwds Original Message:(Found '%s': 'blocking', 'forwarding' or 'accounting' keyword expected.\n)
# Habe gefunden '%s': 'blocking', 'forwarding' oder 'accounting' erwartet.\n

$ #_raw_socket Original Message:(ipfw: raw socket creation)
# ipfw: raw socket creation

$ #_expect_main_blocking Original Message:(ipfw: expected \"blocking\" or \"forwarding\".\n)
# ipfw: habe "blocking" oder "forwarding" erwartet.\n

$ #_expect_main_accept Original Message:(ipfw: expected \"accept\", \"deny\" or \"reject\".\n)
# ipfw: habe "accept", "deny" oder "reject" erwartet.\n

$ #_expect_main_accounting Original Message:(ipfw: expected \"accounting\", \"blocking\" or \"forwarding\".\n)
# ipfw: habe "accounting", "blocking" oder "forwarding" erwartet.\n

$ #_illegal_check Original Message:(ipfw: illegal `check' keyword: %s\n)
# ipfw: ungültiges `check': %s\n

$ #_main_missing Original Message:((missing))
# (fehlt)

$ #_unkn_cmd Original Message:(ipfw: unknown command `%s'\n\n)
# ipfw: unbekannter Befehl `%s'\n\n

$ #_unkn_kwd Original Message:(ipfw: unknown `%s' keyword: `%s'\n)
# ipfw: unbekanntes `%s' Schlüsselwort: `%s'\n

$ #_reject Original Message:(\treject )
# \treject 

$ #_badflag Original Message:(ipfw: \"%s\" flag only applicable to TCP rules\n)
# ipfw: "%s" flag only applicable to TCP rules\n

$ #_missing_fromi Original Message:(ipfw: missing \"from\" or \"iface\" keyword\n)
# ipfw: missing "from" or "iface" keyword\n

$ #_paq_deny Original Message:(packet denied by %s firewall\n)
# packet denied by %s firewall\n

$ #_list_reject Original Message:(reject  )
# reject  

$ #_dp_deny Original Message:(Default policy: deny\n)
# Default policy: deny\n

$ #_dp_accept Original Message:(Default policy: accept\n)
# Default policy: accept\n

$ #_dp_reject Original Message:(Default policy: reject\n)
# Default policy: reject\n

$ #_no_support Original Message:(ipfw: no support for `%s'. Please recompile with newer Kernel.\n)
# ipfw: no support for `%s'. Please recompile with newer Kernel.\n

$ #_list_account Original Message:(account )
# account 
