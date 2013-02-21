void extractimagehandler( GtkWidget *widget, gpointer user_data ) {
	GtkWidget *cdreaderwidget, *directorywidget, *filenamewidget, *burnimagefilewidget;
	gchar **command;
	gchar *cdreader, *directory, *imagefile, *tmpfilename, *tmpcommand;
	gchar *action;
	T_cdinfos readercdinfos;
	
	cdreaderwidget = (GtkWidget *) gtk_builder_get_object( xml, "extractimagecdreader" );
	cdreader = gtk_combo_box_get_active_text( (GtkComboBox*) cdreaderwidget );
	directorywidget = (GtkWidget *) gtk_builder_get_object( xml, "extractimagefiledirectory" );
	directory = gtk_file_chooser_get_filename( (GtkFileChooser*) directorywidget );
	filenamewidget = (GtkWidget *) gtk_builder_get_object( xml, "extractimagefilename" );
	burnimagefilewidget = (GtkWidget *) gtk_builder_get_object( xml, "burnimagefile" );
	
	setcdinfos( cdreader, &readercdinfos );
	
	if ( hasiso9660media( readercdinfos ) && hasfilename( "extractimagefilename" ) ) {
		if ( ! g_regex_match_simple(".iso$", gtk_entry_get_text( (GtkEntry*) filenamewidget ), 0, 0) ) {
			tmpfilename = g_strdup_printf( "%s.iso", gtk_entry_get_text( (GtkEntry*) filenamewidget ) );
			gtk_entry_set_text( (GtkEntry*) filenamewidget, tmpfilename );
			g_free( tmpfilename );
		}

		imagefile = g_strdup_printf( "%s/%s", directory, gtk_entry_get_text( (GtkEntry*) filenamewidget ) );
		
		tmpcommand = g_strdup_printf( "touch \"%s\"", imagefile );
		g_spawn_command_line_sync( tmpcommand, NULL, NULL, NULL, NULL );
		g_free( tmpcommand );
		
		gtk_file_chooser_set_filename( (GtkFileChooser*) burnimagefilewidget, imagefile );		
	
		command = g_new ( gchar *, 4 );
		command[0] = g_strdup_printf( "simpleburn-extract-iso" );
		command[1] = g_strdup_printf( "%s", cdreader );
		command[2] = g_strdup_printf( "\"%s\"", imagefile );
		command[3] = NULL;
		
		action = g_strdup_printf( gettext("Extracting ISO image of '%s' to '%s' ..."), cdreader, imagefile );
		displayinfo( action );
		startprogress( command, TRUE, TRUE, NULL );
		
		g_strfreev( command );
		g_free( action );
		g_free( imagefile );
	}
	
	g_free( cdreader );
	g_free( directory );
}


void extractaudiohandler( GtkWidget *widget, gpointer user_data ) {
	GtkWidget *cdreaderwidget, *directorywidget, *burndirectorywidget, *audiotrackswidget, *audioformatwidget;
	gchar **command;
	gchar *action;
	gchar *cdreader, *directory, *audioformat, *audiotracks;
	T_cdinfos readercdinfos;
	//~ GRegex *re;
	
	cdreaderwidget = (GtkWidget *) gtk_builder_get_object( xml, "extractaudiocdreader" );
	cdreader = gtk_combo_box_get_active_text( (GtkComboBox*) cdreaderwidget );
	directorywidget = (GtkWidget *) gtk_builder_get_object( xml, "extractaudiodirectory" );
	directory = gtk_file_chooser_get_filename( (GtkFileChooser*) directorywidget);
	burndirectorywidget = (GtkWidget *) gtk_builder_get_object( xml, "burndirectorydirectory" );
	audiotrackswidget = (GtkWidget *) gtk_builder_get_object( xml, "extractaudiotracks" );
	//~ re = g_regex_new( " ", 0, 0, NULL );
	//~ audiotracks = g_regex_replace( re, gtk_entry_get_text( (GtkEntry*) audiotrackswidget ), -1, 0, "", 0, NULL);
	//~ g_regex_unref( re );
	audiotracks = g_strdup_printf( "%s", gtk_entry_get_text( (GtkEntry*) audiotrackswidget ) );
	audioformatwidget = (GtkWidget *) gtk_builder_get_object( xml, "extractaudioformat" );
	audioformat = gtk_combo_box_get_active_text( (GtkComboBox*) audioformatwidget );
	
	setcdinfos( cdreader, &readercdinfos );
	
	//~ if (g_ascii_strcasecmp( "", audiotracks ) == 0) {
		//~ g_free( audiotracks );
		//~ audiotracks = g_strdup_printf( "1-%d", readercdinfos.trackscount );
	//~ }

	if ( hasaudiocdmedia( readercdinfos ) ) {
		gtk_file_chooser_set_filename( (GtkFileChooser*) burndirectorywidget, directory );
		
		command = g_new ( gchar *, 6 );
		command[0] = g_strdup_printf( "simpleburn-extract-audio" );
		command[1] = g_strdup_printf( "%s", cdreader );
		command[2] = g_strdup_printf( "\"%s\"", directory );
		command[3] = g_strdup_printf( "%s", audioformat );
		command[4] = g_strdup_printf( "%s", audiotracks );
		command[5] = NULL;
		
		action = g_strdup_printf( gettext("Extracting audio of '%s' to '%s' ..."), cdreader, directory );
		displayinfo( action );
		startprogress( command, TRUE, TRUE, NULL );
		
		g_strfreev( command );
		g_free( action );
	}
	
	g_free( audiotracks );
	g_free( audioformat );
	g_free( cdreader );
	g_free( directory );
}
