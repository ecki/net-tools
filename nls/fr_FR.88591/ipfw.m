$set 9  #ipfw

$ #_ipf_blocking Original Message:(blocking)
# blocage

$ #_ipf_fwding Original Message:(forwarding)
# renvoi

$ #_ipf_accnting Original Message:(accounting)
# comptabilité

$ #_ipf_msqrading Original Message:(masquerading)
# déguisement

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
# ipfw: intervalle d'options positionné mais seulement %d ports\n

$ #_unkn Original Message:(ipfw: unknown command (%d) passed to do_setsockopt - bye!\n)
# ipfw: commande inconnue (%d) passée à do_setsockopt - bye !\n

$ #_ip Original Message:(ip header length %d, should be %d\n)
# longueur de l'en-tête ip = %d, devrait être %d\n

$ #_data_ip Original Message:(data = struct iphdr : struct %shdr {\n)
# donnée = struct iphdr : struct %shdr {\n

$ #_data_ipfw Original Message:(data = struct ip_fw {\n)
# donnée = struct ip_fw {\n

$ #_accept Original Message:(\taccept )
# \taccepte 

$ #_deny Original Message:(\tdeny )
# \trefuse 

$ #_univ Original Message:(\tuniversal\n)
# \tuniversel\n

$ #_missing Original Message:(ipfw: missing protocol name\n)
# ipfw: nom du protocole manquant\n

$ #_illegal Original Message:(illegal protocol name \"%s\"\n)
# nom de protocole pas autorisé "%s"\n

$ #_missing_ip Original Message:(ipfw: missing ip address\n)
# ipfw: adresse IP manquante\n

$ #_periods Original Message:(ipfw: two periods in a row in ip address (%s)\n)
# ipfw: deux points finaux dans une adresse ip (%s)\n

$ #_unkn_host Original Message:(ipfw: unknown host \"%s\"\n)
# ipfw: hôte inconnu "%s"\n

$ #_addr_length Original Message:(ipfw: hostentry addr length = %d, expected %d (i.e. sizeof(struct in_addr))\n)
# ipfw: longueur adresse hôte = %d, attendu %d (i.e. sizeof(struct in_addr))\n

$ #_matched Original Message:(ipfw: Only %d fields matched in IP address!\n)
# ipfw: Seulement %d champs corrects dans l'adresse IP !\n

$ #_too_large Original Message:(ipfw: number too large in ip address (%s)\n)
# ipfw: nombre trop grand dans l'adresse IP (%s)\n

$ #_inc_format Original Message:(ipfw: incorrect ip address format \"%s\" (expected 3 periods)\n)
# ipfw: format d'adresse IP incorrect "%s" (3 points)\n

$ #_not_allowed Original Message:(ipfw: ip netmask not allowed here (%s)\n)
# ipfw: masque de réseau IP pas autorisé ici (%s)\n

$ #_missing_mask Original Message:(ipfw: missing mask value (%s)\n)
# ipfw: il manque la valeur du masque (%s)\n

$ #_non_num Original Message:(ipfw: non-numeric mask value (%s)\n)
# ipfw: valeur du masque pas numérique (%s)\n

$ #_junk_mask Original Message:(ipfw: junk after mask (%s)\n)
# ipfw: détruit après le masque (%s)\n

$ #_out_range Original Message:(ipfw: mask length value out of range (%s)\n)
# ipfw: longueur de masque en dehors des limites (%s)\n

$ #_junk_ip Original Message:(ipfw: junk after ip address (%s)\n)
# ipfw: détruit après l'adresse IP (%s)\n

$ #_illegal_port Original Message:(ipfw: illegal port number (%s)\n)
# ipfw: numéro de port incorrect (%s)\n

$ #_portnum_out Original Message:(ipfw: port number out of range (%d)\n)
# ipfw: numéro de port en dehors des limites (%d)\n

$ #_unkn_service Original Message:(ipfw: unknown %s service \"%s\"\n)
# ipfw: service %s inconnu "%s"\n

$ #_too_port Original Message:(ipfw: too many port numbers (max %d, got at least %d, next parm=\"%s\")\n)
# ipfw: trop de numéros de port (max %d, eu au moins %d, prochain parm="%s")\n

$ #_port_ranges Original Message:(ipfw: port ranges are only allowed for the first port value pair (%s)\n)
# ipfw: les intervalles de num. de ports sont permis seulement pour la première paire (%s)\n

$ #_no_range Original Message:(ipfw: port range not allowed here (%s)\n)
# ipfw: intervalle de num. de ports pas autorisés (%s)\n

$ #_missing_port Original Message:(ipfw: missing port number%s\n)
# ipfw: numéro%s de port manquant(s)\n

$ #_nomore_port Original Message:(ipfw: not enough port numbers (expected %d, got %d)\n)
# ipfw: pas assez de numéros de ports (souhaité %d, eu %d)\n

$ #_check_blocking Original Message:(blocking)
# blocage

$ #_check_forwarding Original Message:(forwarding)
# renvoi

$ #_check Original Message:(check %s )
# vérifie le %s 

$ #_only_check Original Message:(ipfw: can only check TCP or UDP packets\n)
# ipfw: ne peut vérifier que les paquets TCP ou UDP\n

$ #_missing_from Original Message:(ipfw: missing \"from\" keyword\n)
# ipfw: il manque le mot clé "from"\n

$ #_expect_from Original Message:(ipfw: expected \"from\" keyword, got \"%s\"\n)
# ipfw: mot clé attendu "from", eu "%s"\n

$ #_missing_to Original Message:(ipfw: missing \"to\" keyword\n)
# ipfw: il manque le mot clé "to"\n

$ #_expect_to Original Message:(ipfw: expected \"to\" keyword, got \"%s\"\n)
# ipfw: mot clé attendu "to", eu "%s"\n

$ #_paq_accept Original Message:(packet accepted by %s firewall\n)
# paquet accepté par le firewall %s\n

$ #_paq_reject Original Message:(packet rejected by %s firewall\n)
# paquet rejeté par le firewall %s\n

$ #_blocking Original Message:(blocking)
# blocage

$ #_forwarding Original Message:(forwarding)
# renvoi

$ #_extra Original Message:(ipfw: extra parameters at end of command ()
# ipfw: paramètres en trop à la fin de la commande (

$ #_usage21 Original Message:(usage: ipfirewall add %s ...\n)
# Syntaxe: ipfirewall add %s ...\n

$ #_add Original Message:(add %s )
# ajoute (add) %s 

$ #_missing_acc Original Message:(ipfw: missing \"accept\" or \"deny\" keyword\n)
# ipfw: il manque les mots clés "accept" ou "deny"\n

$ #_expect_acc Original Message:(ipfw: expected \"accept\", \"deny\" or \"reject\", got \"%s\"\n)
# ipfw: mots clés attendus "accept", "deny" ou "reject", eu "%s"\n

$ #_missing_proto Original Message:(ipfw: missing protocol name.\n)
# ipfw: il manque le nom du protocole.\n

$ #_missing_iface Original Message:(ipfw: missing interface address.\n)
# ipfw: il manque l'adresse de l'interface.\n

$ #_invalid_iface Original Message:(Invalid interface address.\n)
# Adresse d'interface incorrecte.\n

$ #_missing_from2 Original Message:(ipfw: missing \"from\" keyword\n)
# ipfw: il manque le mot clé "from"\n

$ #_expect_from2 Original Message:(ipfw: expected \"from\", got \"%s\"\n)
# ipfw: mot clé attendu "from", eu "%s"\n

$ #_missing_to2 Original Message:(ipfw: missing \"to\" keyword\n)
# ipfw: il manque le mot clé "to"\n

$ #_expect_to2 Original Message:(ipfw: expected \"to\", got \"%s\"\n)
# ipfw: mot clé attendu "to", eu "%s"\n

$ #_extra2 Original Message:(ipfw: extra parameters at end of command ()
# ipfw: paramètres en trop à la fin de la commande (

$ #_usage22 Original Message:(usage: ipfirewall delete %s ...\n)
# Syntaxe: ipfirewall delete %s ...\n

$ #_delete Original Message:(delete %s )
# supprime (delete) %s 

$ #_missing_acc2 Original Message:(ipfw: missing \"accept\" or \"deny\" keyword\n)
# ipfw: il manque les mots clés "accept" ou "deny"\n

$ #_expect_acc2 Original Message:(ipfw: expected \"accept\" or \"deny\", got \"%s\"\n)
# ipfw: mots clés attendus "accept" ou "deny", eu "%s"\n

$ #_missing_proto2 Original Message:(ipfw: missing protocol name.\n)
# ipfw: il manque le nom du protocole.\n

$ #_missing_iface2 Original Message:(ipfw: missing interface address.\n)
# ipfw: il manque l'adresse de l'interface.\n

$ #_invalid_iface2 Original Message:(Invalid interface address.\n)
# Adresse d'interface incorrecte.\n

$ #_missing_from3 Original Message:(ipfw: missing \"from\" keyword\n)
# ipfw: il manque le mot clé "from"\n

$ #_expect_from3 Original Message:(ipfw: expected \"from\", got \"%s\"\n)
# ipfw: mot clé attendu "from", eu "%s"\n

$ #_missing_to3 Original Message:(ipfw: missing \"to\" keyword\n)
# ipfw: il manque le mot clé "to"\n

$ #_expect_to3 Original Message:(ipfw: expected \"to\", got \"%s\"\n)
# ipfw: expected "to", got "%s"\n

$ #_extra3 Original Message:(ipfw: extra parameters at end of command ()
# ipfw: paramètres en trop à la fin de la commande (

$ #_anywhere Original Message:(anywhere)
# partout

$ #_bytes Original Message:(  Packete    Bytes  )
#   Paquets    octets 

$ #_proto Original Message:(Type    Proto Flags From                To                  Iface               Ports\n)
# Type    Proto Flags From                To                  Iface               Ports\n

$ #_list_accept Original Message:(accept  )
# accepte 

$ #_list_deny Original Message:(deny    )
# refuse  

$ #_list_any Original Message:(any)
# quelconque

$ #_expect_kwds Original Message:(blocking, forwarding or accounting keyword expected.\n)
# mots clés attendus : `blocking', `forwarding' ou `accounting'.\n

$ #_found_kwds Original Message:(Found '%s': 'blocking', 'forwarding' or 'accounting' keyword expected.\n)
# Trouvé '%s': mots clés 'blocking', 'forwarding' ou 'accounting' attendus.\n

$ #_raw_socket Original Message:(ipfw: raw socket creation)
# ipfw: création d'une prise de type `raw'

$ #_expect_main_blocking Original Message:(ipfw: expected \"blocking\" or \"forwarding\".\n)
# ipfw: "blocking" ou "forwarding" attendus.\n

$ #_expect_main_accept Original Message:(ipfw: expected \"accept\", \"deny\" or \"reject\".\n)
# ipfw: "accept", "deny" ou "reject" attendus.\n

$ #_expect_main_accounting Original Message:(ipfw: expected \"accounting\", \"blocking\" or \"forwarding\".\n)
# ipfw: "accounting", "blocking" ou "forwarding" attendus.\n

$ #_illegal_check Original Message:(ipfw: illegal `check' keyword: %s\n)
# ipfw: mot clé `check' incorrect: %s\n

$ #_main_missing Original Message:((missing))
# (manquant)

$ #_unkn_cmd Original Message:(ipfw: unknown command `%s'\n\n)
# ipfw: commande inconnue `%s'\n\n

$ #_unkn_kwd Original Message:(ipfw: unknown `%s' keyword: `%s'\n)
# ipfw: mot clé inconnu pour `%s': `%s'\n

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
