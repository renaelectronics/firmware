#
# Regular cron jobs for the renaconf package
#
0 4	* * *	root	[ -x /usr/bin/renaconf_maintenance ] && /usr/bin/renaconf_maintenance
