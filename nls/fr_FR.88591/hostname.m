$set 5  #hostname

$ #_root Original Message:(%s: you must be root to change the host name\n)
# %s: vous devez etre super-utilisateur pour changer le nom d'hôte\n

$ #_toolong Original Message:(%s: name too long\n)
# %s: nom trop long\n

$ #_nodns1 Original Message:(%s: You can't change the DNS domain name with this command\n)
# %s: Vous ne pouvez pas changer le nom de domainde DNS avec cette commande\n

$ #_nodns2 Original Message:(\nUnless you are using bind or NIS for host lookups you can change the DNS\n)
# \nSauf si vous utilisez `bind' ou NIS pour la recherche des hôtes, vous pouvez changer le nom\n

$ #_nodns3 Original Message:(domain name (which is part of the FQDN) in the /etc/hosts file.\n)
# de domaine DNS (qui fait partie de FQDN) dans le fichier /etc/hosts.\n

$ #_cant_open Original Message:(%s: can't open `%s'\n)
# %s: ne peut ouvrir `%s'\n

$ #_usage1 Original Message:(Usage: hostname [-v] {hostname|-F file} set hostname (from file)\n)
# Usage: hostname [-v] {hostname|-F file}      set hostname (from file)\n

$ #_usage2 Original Message:(       domainname [-v] {nisdomain|-F file} set NIS domainname (from file)\n)
#        domainname [-v] {nisdomain|-F file}   set NIS domainname (from file)\n

$ #_usage3 Original Message:(       hostname [-v] [-d|-f|-s|-a|-i|-y] display formated name\n)
#        hostname [-v] [-d|-f|-s|-a|-i|-y]     display formated name\n

$ #_usage4 Original Message:(       hostname [-v] display hostname\n\n)
#        hostname [-v]                         display hostname\n\n

$ #_usage5 Original Message:(       hostname -V|--version|-h|--help print info and exit\n\n)
#        hostname -V|--version|-h|--help       print info and exit\n\n

$ #_usage6 Original Message:(    dnsdomainname=hostname -d, {yp,nis,}domainname=hostname -y\n\n)
#     dnsdomainname=hostname -d, {yp,nis,}domainname=hostname -y\n\n

$ #_usage7 Original Message:(    -s, --short           short host name\n)
#     -s, --short           short host name\n

$ #_usage8 Original Message:(    -a, --alias           alias names\n)
#     -a, --alias           alias names\n

$ #_usage9 Original Message:(    -i, --ip-address      addresses for the hostname\n)
#     -i, --ip-address      addresses for the hostname\n

$ #_usage10 Original Message:(    -f, --fqdn, --long    long host name (FQDN)\n)
#     -f, --fqdn, --long    long host name (FQDN)\n

$ #_usage11 Original Message:(    -d, --domain          DNS domain name\n)
#     -d, --domain          DNS domain name\n

$ #_usage12 Original Message:(    -y, --yp, --nis       NIS/YP domainname\n)
#     -y, --yp, --nis       NIS/YP domainname\n

$ #_usage13 Original Message:(    -F, --file            read hostname or nis domainname from given File\n\n)
#     -F, --file            read hostname or nis domainname from given File\n\n

$ #_usage14 Original Message:(   This comand can get or set the hostname or the NIS domainname. You can\n)
#    This comand can get or set the hostname or the NIS domainname. You can\n

$ #_usage15 Original Message:(   also get the DNS domain or the FQDN (fully qualified domain name).\n)
#    also get the DNS domain or the FQDN (fully qualified domain name).\n

$ #_usage16 Original Message:(   Unless you are using bind or NIS for host lookups you can change the\n)
#    Unless you are using bind or NIS for host lookups you can change the\n

$ #_usage17 Original Message:(   FQDN (Fully Qualified Domain Name) and the DNS domain name (which is\n)
#    FQDN (Fully Qualified Domain Name) and the DNS domain name (which is\n

$ #_usage18 Original Message:(   part of the FQDN) in the /etc/hosts file.\n)
#    part of the FQDN) in the /etc/hosts file.\n

$ #_verb_set Original Message:(  Setting hostname to `%s'\n)
#    Setting hostname to `%s'\n

$ #_verb_res Original Message:(Resolving `%s' ...\n)
# Resolving `%s' ...\n

$ #_verb_fnd Original Message:(Result: h_name=`%s'\n)
# Result: h_name=`%s'\n

$ #_verb_ali Original Message:(Result: h_aliases=`%s'\n)
# Result: h_aliases=`%s'\n

$ #_verb_ipn Original Message:(Result: h_addr_list=`%s'\n)
# Result: h_addr_list=`%s'\n

$ #_verb_get Original Message:(gethostname()=`%s'\n)
# gethostname()=`%s'\n

$ #_verb_dset Original Message:(Setting domainname to %s'\n)
# Setting domainname to %s'\n

$ #_verb_dget Original Message:(getdomainname()=%s'\n)
# getdomainname()=%s'\n

$ #_dname_root Original Message:(%s: you must be root to change the domain name\n)
# %s: you must be root to change the domain name\n

$ #_dname_toolong Original Message:(%s: name too long\n)
# %s: name too long\n
