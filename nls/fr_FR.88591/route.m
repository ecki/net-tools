$set 2  #route

$ #_rresolve Original Message:(rresolve: unsupport address family %d !\n)
# rresolve: famille d'adresse pas supportée %d !\n

$ #_usage1 Original Message:(Usage: route [-nvee] [-A inet|ipx|ddp|netrom],...  route {--version|--help}\n\n)
# Usage: route [-nvee] [-A inet|ipx|ddp|netrom],...  route {--version|--help}\n\n

$ #_usage2 Original Message:(       route [-v] del {-net|-host} Target [gw Gateway] [netmask Nm]\n)
#        route [-v] del {-net|-host} Target [gw Gateway] [netmask Nm]\n

$ #_usage3 Original Message:(             [metric N] [[dev] If]\n\n)
#              [metric N] [[dev] If]\n\n

$ #_usage4 Original Message:(       route [-v] add {-net|-host} Target [gw Gateway] [netmask Nm]\n)
#        route [-v] add {-net|-host} Target [gw Gateway] [netmask Nm]\n

$ #_usage5 Original Message:(             [metric N] [mss M] [window W] [irtt I] [reject]\n)
#              [metric N] [mss M] [window W] [irtt I] [reject]\n

$ #_usage6 Original Message:(             [mod] [dyn] [reinstate] [[dev] If]\n)
#              [mod] [dyn] [reinstate] [[dev] If]\n

$ #_table Original Message:(Kernel routing table\n)
# Table de routage noyau\n

$ #_cant_use Original Message:(route: %s: cannot use a NETWORK as gateway!\n)
# route: %s: ne peut utiliser un RESEAU comme passerelle !\n

$ #_MSS Original Message:(route: Invalid MSS.\n)
# route: MSS invalide.\n

$ #_window Original Message:(route: Invalid window.\n)
# route: Taille de fenetre invalide.\n

$ #_irtt Original Message:(route: Invalid initial rtt.\n)
# route: irtt invalide.\n

$ #_netmask1 Original Message:(route: netmask doesn't make sense with host route\n)
# route: netmask n'a pas de sens avec une route vers un hôte\n

$ #_netmask2 Original Message:(route: bogus netmask %s\n)
# route: mauvais netmask %s\n

$ #_netmask3 Original Message:(route: netmask doesn't match route address\n)
# route: netmask ne correspond pas à l'adresse de route\n

$ #_fam_not_supp Original Message:(%s: address family not supported!\n)
# %s: address family not supported!\n

$ #_af_no_route Original Message:(route: %s: address family doesn't support routing.\n)
# route: %s: address family doesn't support routing.\n

$ #_wrong_af Original Message:(route: %s: command only supports 'inet' AF.\n)
# route: %s: command only supports 'inet' AF.\n
