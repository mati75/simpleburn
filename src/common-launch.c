#define LOGSIZE 500

GPid pid=0;
typedef struct {
	GPid pid;
	GtkProgressBar *progressbar;
	guint updatetimeoutid;
	guint waittimeoutid;
	guint timetimeoutid;
	gboolean commandhasioc;
	GIOChannel *stdoutioc;
	GTimeVal starttime;
	GTimeVal progressstarttime;
	glong totaltime;
	gchar *cdwriter;
} s_commanddata;


gboolean pulsebarhandler( gpointer data ) {
	s_commanddata *commanddata = (s_commanddata*) data;
	
	gtk_progress_bar_pulse( commanddata->progressbar );
	return TRUE;
}


gboolean progressbarhandler( GIOChannel *source, GIOCondition condition, gpointer data ) {
	s_commanddata *commanddata = (s_commanddata*) data;
	gdouble progressfraction;
	gchar *s_progressfraction;
	gchar *line;
	GTimeVal currenttime;
	glong progresselapsedtime, elapsedtime, totaltime, remainingtime;
	
	if ( commanddata->waittimeoutid != 0 ) { //the first time data is received, the pulse bar thread has to be ended
		g_source_remove( commanddata->waittimeoutid );
		commanddata->waittimeoutid = 0;
		g_get_current_time( &commanddata->progressstarttime );
		gtk_progress_bar_set_fraction( commanddata->progressbar, 0 );
		gtk_progress_bar_set_text( commanddata->progressbar, "  0 %" );
	}
	
	g_io_channel_read_line( commanddata->stdoutioc, &line, NULL, NULL, NULL );
	if ( line != NULL ) {
		line [strlen(line)-1] = '\0';
		progressfraction = g_ascii_strtoull( line, NULL, 10) / 100.0;

		if ( progressfraction != gtk_progress_bar_get_fraction( commanddata->progressbar) ) {
			gtk_progress_bar_set_fraction( commanddata->progressbar, progressfraction );
			s_progressfraction = g_strdup_printf( "%3s %c", line, '%' );
			gtk_progress_bar_set_text( commanddata->progressbar, s_progressfraction );
			g_free ( s_progressfraction );
			
			g_get_current_time( &currenttime );
			elapsedtime = currenttime.tv_sec - commanddata->starttime.tv_sec;
			progresselapsedtime = currenttime.tv_sec - commanddata->progressstarttime.tv_sec;
			totaltime = (glong) (progresselapsedtime / progressfraction) + (elapsedtime - progresselapsedtime);
			if ( labs( commanddata->totaltime - totaltime ) > 5 )
				commanddata->totaltime = totaltime;
		} else {
			gtk_progress_bar_set_fraction( commanddata->progressbar, progressfraction ); //unneeded but force window repainting
		}
		
		g_free( line );
	}
	return TRUE;
}


gchar *displayminutesandseconds( glong timeinseconds ) {
	glong minutes, seconds;
	minutes = (guint) floor( timeinseconds / 60 );
	seconds = (guint) timeinseconds % 60;
	return g_strdup_printf( "%02ld' %02ld''", minutes, seconds );
}


gboolean timehandler( gpointer data ) { //displays times
	s_commanddata *commanddata = (s_commanddata*) data;
	GtkLabel *w_elapsedtime, *w_totaltime, *w_remainingtime;
	gchar *s_elapsedtime, *s_totaltime, *s_remainingtime;
	GTimeVal currenttime;
	glong elapsedtime, remainingtime;
	
	g_get_current_time( &currenttime );
	elapsedtime = currenttime.tv_sec - commanddata->starttime.tv_sec;
	w_elapsedtime = (GtkLabel*) gtk_builder_get_object( xml, "elapsedtime" );
	s_elapsedtime = displayminutesandseconds( elapsedtime );
	gtk_label_set_text( w_elapsedtime, s_elapsedtime );
	g_free( s_elapsedtime );
	
	if ( commanddata->totaltime != 0 ) {
		w_totaltime = (GtkLabel*) gtk_builder_get_object( xml, "totaltime" );
		w_remainingtime = (GtkLabel*) gtk_builder_get_object( xml, "remainingtime" );
		
		if ( gtk_progress_bar_get_fraction( commanddata->progressbar ) < 1 ) {
			s_totaltime = displayminutesandseconds( commanddata->totaltime );
			gtk_label_set_text( w_totaltime, s_totaltime );
			g_free( s_totaltime );
			remainingtime = commanddata->totaltime - elapsedtime;
			s_remainingtime = displayminutesandseconds( remainingtime );
			gtk_label_set_text( w_remainingtime, s_remainingtime );
			g_free( s_remainingtime );
		} else {
			gtk_label_set_text( w_totaltime, "" );
			gtk_label_set_text( w_remainingtime, "" );
		}
	}
	
	return TRUE;
}


void stopprogress( GPid pid, gint status, gpointer data ) {
	s_commanddata *commanddata = (s_commanddata*) data;
	GtkLabel *w_elapsedtime, *w_totaltime, *w_remainingtime;
	gchar *command;
	
	pid = 0;
	if ( commanddata->waittimeoutid != 0 )
		g_source_remove( commanddata->waittimeoutid );
	g_source_remove( commanddata->updatetimeoutid );
	g_source_remove( commanddata->timetimeoutid );
	
	if ( commanddata->commandhasioc ) {
		g_io_channel_shutdown( commanddata->stdoutioc, FALSE, NULL );
		g_io_channel_unref ( commanddata->stdoutioc );
	}
	
	if (status == 0) {
		displayinfo( gettext( "Operation completed." ) );
	} else {
		displayerror( gettext( "Error : something went wrong." ) );
	}
	
	gtk_progress_bar_set_fraction( commanddata->progressbar, 0 );
	gtk_progress_bar_set_text( commanddata->progressbar, "" );
	
	w_elapsedtime = (GtkLabel*) gtk_builder_get_object( xml, "elapsedtime" );
	w_remainingtime = (GtkLabel*) gtk_builder_get_object( xml, "remainingtime" );
	w_totaltime = (GtkLabel*) gtk_builder_get_object( xml, "totaltime" );	
	gtk_label_set_text( w_elapsedtime, "" );
	gtk_label_set_text( w_remainingtime, "" );
	gtk_label_set_text( w_totaltime, "" );

	if ( commanddata->cdwriter != NULL ) {
		command = g_strdup_printf( "eject %s", commanddata->cdwriter );
		g_spawn_command_line_sync( command, NULL, NULL, NULL, NULL );
		g_free( command );
		g_free( commanddata->cdwriter );
	}
		
	g_free( commanddata );
	
	enableactions();
	gtk_widget_set_sensitive( (GtkWidget *) gtk_builder_get_object( xml, "aborting" ), FALSE );
}


void abortinghandler( GtkWidget *widget, gpointer user_data ) {
	gchar *command;
	
	gtk_widget_set_sensitive( (GtkWidget *) gtk_builder_get_object( xml, "aborting" ), FALSE );
	command = g_strdup_printf( "simpleburn-abort-operation %d", pid );
	g_spawn_command_line_sync( command, NULL, NULL, NULL, NULL );
	g_free( command );
	pid = 0;
}


void quitsimpleburnhandler( GtkWidget *widget, gpointer user_data ) {
	if ( pid != 0 ) //pid !=0 <=> an operation is underway
		abortinghandler( widget, user_data );
	gtk_main_quit();
}

void startprogress( gchar **command, gboolean commandhasioc, gboolean cancelable, gchar *cdwriter ) {
	gint i, stdoutfd;
	s_commanddata *commanddata = g_new( s_commanddata, 1 );
	
	disableactions();
	
	if ( cdwriter != NULL ) //for a burning operation, media ejection force the redetection (see "stopprogress()")
		commanddata->cdwriter = g_strdup_printf( "%s", cdwriter );
	else //for an extraction operation, it isn't needed to eject the media
		commanddata->cdwriter = NULL;
	
	g_get_current_time( &commanddata->starttime );
	commanddata->totaltime = 0;
	commanddata->timetimeoutid = g_timeout_add( 333, timehandler, commanddata );
		
	g_spawn_async_with_pipes( NULL, command, NULL, G_SPAWN_SEARCH_PATH|G_SPAWN_DO_NOT_REAP_CHILD, NULL, NULL, &commanddata->pid, NULL, &stdoutfd, NULL, NULL);
	pid = commanddata->pid;
	g_child_watch_add( commanddata->pid, stopprogress, commanddata );
	
	commanddata->progressbar = (GtkProgressBar*) gtk_builder_get_object( xml, "progressbar" );
	
	if ( commandhasioc ) { //if the progress can be displayed
		commanddata->commandhasioc = TRUE;
		commanddata->stdoutioc = g_io_channel_unix_new( stdoutfd );
		commanddata->waittimeoutid = g_timeout_add( 100, pulsebarhandler, commanddata ); //first use a pulse bar until the progress can be displayed
		commanddata->updatetimeoutid = g_io_add_watch( commanddata->stdoutioc, G_IO_IN, progressbarhandler, commanddata );
	} else { //else use a pulse bar
		commanddata->commandhasioc = FALSE;
		commanddata->waittimeoutid = 0;
		commanddata->updatetimeoutid = g_timeout_add( 100, pulsebarhandler, commanddata );
	}
	
	if ( cancelable )
		gtk_widget_set_sensitive( (GtkWidget *) gtk_builder_get_object( xml, "aborting" ), TRUE );
}
