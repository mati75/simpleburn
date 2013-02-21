void detectmediahandler( GtkWidget *widget, gpointer user_data ) {
	GtkWidget *cdreaderwidget;
	gchar *cdreader, *message, *rewritable;
	T_cdinfos readercdinfos;
	
	cdreaderwidget = (GtkWidget *) gtk_builder_get_object( xml, "infocdreader" );
	cdreader = gtk_combo_box_get_active_text( (GtkComboBox*) cdreaderwidget );
	
	setcdinfos( cdreader, &readercdinfos );
	
	if ( ! readercdinfos.hasmedia )
		message = g_strdup_printf( "%s.", gettext( "No media inserted" ) );
	else {
		if ( readercdinfos.hasrewritablemedia )
			rewritable = g_strdup_printf( " %s", gettext( "rewritable" ) );
		else
			rewritable = g_strdup_printf( "" );
		
		if ( readercdinfos.hascdrommedia ) {
			if ( readercdinfos.hasemptymedia )
				message = g_strdup_printf( "%s%s, %lu M.", gettext( "Media type : blank CD" ), rewritable, (long unsigned int) readercdinfos.mediacapacity /1024 );
			else {
				if ( readercdinfos.hasaudiocdmedia )
					message = g_strdup_printf( "%s%s, %lu M.", gettext( "Media type : audio CD" ), rewritable, (long unsigned int) readercdinfos.mediasize /1024 );
				else {
					if ( readercdinfos.hasiso9660media )
						message = g_strdup_printf( "%s%s, %lu M.", gettext( "Media type : data CD (ISO9660)" ), rewritable, (long unsigned int) readercdinfos.mediasize /1024 );
					else
						message = g_strdup_printf( "%s%s, %lu M.", gettext( "Media type : data CD (UDF)" ), rewritable, (long unsigned int) readercdinfos.mediasize /1024 );
				}
			}
		} else {
			if ( readercdinfos.hasdvdmedia ) {
				if ( readercdinfos.hasemptymedia )
					message = g_strdup_printf( "%s%s, %lu M.", gettext( "Media type : blank DVD" ), rewritable, (long unsigned int) readercdinfos.mediacapacity /1024 );
				else { if ( readercdinfos.hasiso9660media )
						message = g_strdup_printf( "%s%s, %lu M.", gettext( "Media type : DVD (ISO9660)" ), rewritable, (long unsigned int) readercdinfos.mediasize /1024 );
					else
						message = g_strdup_printf( "%s%s, %lu M.", gettext( "Media type : DVD (UDF)" ), rewritable, (long unsigned int) readercdinfos.mediasize /1024 );
				}
			} else 
				message = g_strdup_printf( "%s%s, %lu M.", gettext( "Media type : unknown" ), rewritable, (long unsigned int) readercdinfos.mediasize /1024 );
		}
		g_free( rewritable );
	}
	
	displayinfo( message );
	g_free( cdreader );
	g_free( message );
}


void ejectmediahandler( GtkWidget *widget, gpointer user_data ) {
	GtkWidget *cdreaderwidget;
	gchar *cdreader, *command;
	gint exitstatus;
	
	cdreaderwidget = (GtkWidget *) gtk_builder_get_object( xml, "infocdreader" );
	cdreader = gtk_combo_box_get_active_text( (GtkComboBox*) cdreaderwidget );
	command = g_strdup_printf( "cdrecord -eject dev=%s", cdreader );
	g_spawn_command_line_sync( command, NULL, NULL, &exitstatus, NULL );
	g_free( command );
	if ( exitstatus !=0 ){
		command = g_strdup_printf( "wodim -eject dev=%s", cdreader );
		g_spawn_command_line_sync( command, NULL, NULL, NULL, NULL );
		g_free( command );
	}
	g_free( cdreader );
}
