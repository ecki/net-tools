$set 5  #hostname

$ #_root Original Message:(%s: you must be root to change the host name\n)
# %s: Nur root darf den Rechnernamen ändern\n

$ #_toolong Original Message:(%s: name too long\n)
# %s: Name zu lang\n

$ #_nodns1 Original Message:(%s: You can't change the DNS domain name with this command\n)
# %s: Sie können den DNS-Name nicht mit diesem Befehl ändern\n

$ #_nodns2 Original Message:(\nUnless you are using bind or NIS for host lookups you can change the DNS\n)
# \nWenn Sie nicht bind oder NIS benutzen, können Sie den DNS-Domainname,\n

$ #_nodns3 Original Message:(domain name (which is part of the FQDN) in the /etc/hosts file.\n)
# welcher Teil des FQDN ist, in /etc/hosts ändern.\n

$ #_cant_open Original Message:(%s: can't open `%s'\n)
# %s: kann `%s' nicht öffnen\n

$ #_usage1 Original Message:(Usage: hostname [-v] {hostname|-F file} set hostname (from file)\n)
# Benutzung: hostname [-v] {hostname|-F datei}  setzt rechnername (aus datei)\n

$ #_usage2 Original Message:(       domainname [-v] {nisdomain|-F file} set NIS domainname (from file)\n)
#        domainname [-v] {nisdomain|-F datei}   setzt NIS domainname (aus datei)\n

$ #_usage3 Original Message:(       hostname [-v] [-d|-f|-s|-a|-i|-y] display formated name\n)
#        hostname [-v] [-d|-f|-s|-a|-i|-y]      zeigt formatierte Namen an\n

$ #_usage4 Original Message:(       hostname [-v] display hostname\n\n)
#        hostname [-v]                          zeigt Rechnername an\n\n

$ #_usage5 Original Message:(       hostname -V|--version|-h|--help print info and exit\n\n)
#        hostname -V|--version|-h|--help        gibt Info aus und beendet sich\n\n

$ #_usage6 Original Message:(    dnsdomainname=hostname -d, {yp,nis,}domainname=hostname -y\n\n)
#     dnsdomainname=hostname -d, {yp,nis,}domainname=hostname -y\n\n

$ #_usage7 Original Message:(    -s, --short           short host name\n)
#     -s, --short           kurzer Rechnername\n

$ #_usage8 Original Message:(    -a, --alias           alias names\n)
#     -a, --alias           alias Namen\n

$ #_usage9 Original Message:(    -i, --ip-address      addresses for the hostname\n)
#     -i, --ip-address      Adressen der Rechnernamen\n

$ #_usage10 Original Message:(    -f, --fqdn, --long    long host name (FQDN)\n)
#     -f, --fqdn, --long    langer Rechnername (FQDN)\n

$ #_usage11 Original Message:(    -d, --domain          DNS domain name\n)
#     -d, --domain          DNS Domainname\n

$ #_usage12 Original Message:(    -y, --yp, --nis       NIS/YP domainname\n)
#     -y, --yp, --nis       NIS/YP Domainname\n

$ #_usage13 Original Message:(    -F, --file            read hostname or nis domainname from given File\n\n)
#     -F, --file            lese Rechnernamen oder NIS Domainnamen aus Datei\n\n

$ #_usage14 Original Message:(   This comand can get or set the hostname or the NIS domainname. You can\n)
#    Dieses Programm kann den Rechnernamen und den NIS-Domainnamen setzen und\n

$ #_usage15 Original Message:(   also get the DNS domain or the FQDN (fully qualified domain name).\n)
#    auslesen. Ausserdem kann es die DNS Domain oder den FQDN auslesen.\n

$ #_usage16 Original Message:(   Unless you are using bind or NIS for host lookups you can change the\n)
#    Wenn Sie nicht bind oder NIS benutzen, können Sie den FQDN (Vollständig\n

$ #_usage17 Original Message:(   FQDN (Fully Qualified Domain Name) and the DNS domain name (which is\n)
#    qualifizierter Domain-Name) und den DNS-Domainnamen (welcher Teil\n

$ #_usage18 Original Message:(   part of the FQDN) in the /etc/hosts file.\n)
#    des FQDN ist) in /etc/hosts ändern.\n

$ #_verb_set Original Message:(  Setting hostname to `%s'\n)
#  Setze Rechnername auf `%s'\n

$ #_verb_res Original Message:(Resolving `%s' ...\n)
# Ermittle `%s' ...\n

$ #_verb_fnd Original Message:(Result: h_name=`%s'\n)
# Ergebnis: h_name=`%s'\n

$ #_verb_ali Original Message:(Result: h_aliases=`%s'\n)
# Ergebnis: h_aliases=`%s'\n

$ #_verb_ipn Original Message:(Result: h_addr_list=`%s'\n)
# Ergebnis: h_addr_list=`%s'\n

$ #_verb_get Original Message:(gethostname()=`%s'\n)
# gethostname()=`%s'\n

$ #_verb_dset Original Message:(Setting domainname to `%s'\n)
# Setze NIS Domainname auf `%s'\n

$ #_verb_dget Original Message:(getdomainname()=`%s'\n)
# getdomainname()=%s'\n

$ #_dname_root Original Message:(%s: you must be root to change the domain name\n)
# %s: Sie müssen root sein um den Domainnamen zu ändern\n

$ #_dname_toolong Original Message:(%s: name too long\n)
# %s: Domainname ist zu lang\n
