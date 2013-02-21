#include <gtk/gtk.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include <math.h>
#include <libintl.h>
#include <locale.h>
#include <glob.h>
#include <signal.h>

#ifdef LIBCDIO_DETECTION
#include <cdio/cdio.h>
#include <cdio/cd_types.h>
#include <cdio/mmc.h>
#endif

#include "config.h"

GtkBuilder *xml;
#include "common-detection.c"
#include "common-interface.c"
#include "common-filesize.c"
#include "common-checks.c"
#include "common-launch.c"
#include "callbacks-blank.c"
#include "callbacks-burn.c"
#include "callbacks-extract.c"
#include "callbacks-copy.c"
#include "callbacks-misc.c"
#include "callbacks-videodvd.c"


int main( int argc, char *argv[] ) {	
	GtkWindow *mainwindow;
	GtkAboutDialog *aboutdialog;
	
	#ifdef DEBUG
		g_print ("DEBUG: main: BEGIN\n");
	#endif
	setlocale( LC_ALL, "" );
	bindtextdomain( PROJECT_NAME, LOCALE_DIR );
	textdomain( PROJECT_NAME );
	#ifdef DEBUG
		g_print ("DEBUG: main: i18n done\n");
	#endif	

	gtk_init( &argc, &argv );
	#ifdef DEBUG
		g_print ("DEBUG: main: gtk_init done\n");
	#endif
	xml = gtk_builder_new();
	gtk_builder_add_from_file( xml, UI_FILE, NULL );
	#ifdef DEBUG
		g_print ("DEBUG: main: ui loaded\n");
	#endif

	initcddevices();
	#ifdef DEBUG
		g_print ("DEBUG: main: devices detected\n");
	#endif
	initvideodvdcomboboxes();
	videoDVDTitlesCount = 0;
	
	addisofilter(); //to the burnimagecdwriterwidget; can't be done in Glade
	gtk_progress_bar_set_text( (GtkProgressBar*) gtk_builder_get_object( xml, "progressbar" ), "" ); //to hide default 0% progress
	gtk_builder_connect_signals( xml, NULL );
	
	mainwindow = (GtkWindow *) gtk_builder_get_object( xml, "mainwindow" );
	gtk_window_set_icon_from_file (mainwindow, APP_ICON, NULL);
	aboutdialog = (GtkAboutDialog *) gtk_builder_get_object( xml, "aboutdialog" );
	gtk_about_dialog_set_version( aboutdialog, PROJECT_VERSION );
	gtk_widget_show ((GtkWidget *) mainwindow);
	#ifdef DEBUG
		g_print ("DEBUG: main: application window shown\n");
	#endif
	gtk_main();
	#ifdef DEBUG
		g_print ("DEBUG: main: END\n");
	#endif
}
