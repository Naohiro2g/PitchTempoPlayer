## Process this file with automake to produce Makefile.in

#icon file
icondir = $(datadir)/icons
dist_icon_DATA = ptplayer.svg \
		 ptplayer.png \
	    	 ptp-equalizer.png \
		 ptp-previous.png \
		 ptp-rewind.png \
		 ptp-play.png \
		 ptp-pause.png \
		 ptp-record.png \
		 ptp-stop.png \
		 ptp-forward.png \
		 ptp-next.png \
		 ptp-microphone.png \
		 ptp-document-open.png \
		 ptp-document-save-as.png \
		 ptp-speaker-mute.png \
		 ptp-qjackctl.png

gtk_update_icon_cache = gtk-update-icon-cache -f -t $(icondir)

#install-data-hook: update-icon-cache
#uninstall-hook: update-icon-cache
#update-icon-cache:
#	@-if test -z "$(datadir)"; then \
#		echo "Updating Gtk icon cache."; \
#		$(gtk_update_icon_cache); \
#	else \
#		echo "*** Icon cache not updated.  After (un)install, run #this:"; \
#		echo "***   $(gtk_update_icon_cache)"; \
#	fi

#desktop file
desktop_in_files= cplayer.desktop.in
desktop_files=$(desktop_in_files:.desktop.in=.desktop)

Applicationsdir = $(datadir)/applications
Applications_DATA = $(desktop_files)

DISTCLEANFILES = $(desktop_files)



