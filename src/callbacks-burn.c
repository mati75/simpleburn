void burnimagehandler( GtkWidget *widget, gpointer user_data ) {
	GtkWidget *cdwriterwidget, *imagefilewidget;
	gchar **command;
	gchar *cdwriter, *imagefile;
	gchar *action;
	T_cdinfos writercdinfos;
	
	cdwriterwidget = (GtkWidget *) gtk_builder_get_object( xml, "burnimagecdwriter" );
	cdwriter = gtk_combo_box_get_active_text( (GtkComboBox*) cdwriterwidget );
	imagefilewidget = (GtkWidget *) gtk_builder_get_object( xml, "burnimagefile" );
	imagefile = gtk_file_chooser_get_filename( (GtkFileChooser*) imagefilewidget );
	
	setcdinfos( cdwriter, &writercdinfos );
	
	if ( haswritablereadymedia( writercdinfos ) && hasenoughspaceforimage( imagefile, writercdinfos ) &&
			hasfilenameset( "burnimagefile" ) ) {
		command = g_new ( gchar *, 4 );
		command[0] = g_strdup_printf( "simpleburn-burn-iso" );
		command[1] = g_strdup_printf( "%s", cdwriter );
		command[2] = g_strdup_printf( "\"%s\"", imagefile );
		command[3] = NULL;
		
		action = g_strdup_printf( gettext("Burning '%s' ISO image on '%s' ..."), imagefile, cdwriter );
		displayinfo( action );
		startprogress( command, TRUE, TRUE, cdwriter );
		
		g_strfreev( command );
		g_free( action );
	}
}


void burnaudiodirectoryhandler( GtkWidget *widget, gpointer user_data ) {
	GtkWidget *cdwriterwidget, *directorywidget;
	gchar **command;
	gchar *cdwriter, *directory;
	gchar *action;
	T_cdinfos writercdinfos;
	
	cdwriterwidget = (GtkWidget *) gtk_builder_get_object( xml, "burndirectorycdwriter" );
	cdwriter = gtk_combo_box_get_active_text( (GtkComboBox*) cdwriterwidget );
	directorywidget = (GtkWidget *) gtk_builder_get_object( xml, "burndirectorydirectory" );
	directory = gtk_file_chooser_get_filename( (GtkFileChooser*) directorywidget);
	
	setcdinfos( cdwriter, &writercdinfos );
	
	if ( hascdrommedia( writercdinfos ) && haswritablereadymedia( writercdinfos ) && 
			hasenoughspaceforaudiodirectory( directory, writercdinfos ) ) {
		command = g_new ( gchar *, 4 );
		command[0] = g_strdup_printf( "simpleburn-burn-audio" );
		command[1] = g_strdup_printf( "%s", cdwriter );
		command[2] = g_strdup_printf( "\"%s\"", directory );
		command[3] = NULL;
		
		action = g_strdup_printf( gettext("Burning audio files in '%s' on '%s' ..."), directory, cdwriter );
		displayinfo( action );
		startprogress( command, TRUE, TRUE, cdwriter );
		
		g_strfreev( command );
		g_free( action );
	}
	
	g_free( cdwriter );
	g_free( directory );
}


void burndatadirectoryhandler( GtkWidget *widget, gpointer user_data ) {
	GtkWidget *cdwriterwidget, *directorywidget;
	gchar **command;
	gchar *action;
	gchar *cdwriter, *directory;
	T_cdinfos writercdinfos;
	
	cdwriterwidget = (GtkWidget *) gtk_builder_get_object( xml, "burndirectorycdwriter" );
	cdwriter = gtk_combo_box_get_active_text( (GtkComboBox*) cdwriterwidget );
	directorywidget = (GtkWidget *) gtk_builder_get_object( xml, "burndirectorydirectory" );
	directory = gtk_file_chooser_get_filename( (GtkFileChooser*) directorywidget);
	
	setcdinfos( cdwriter, &writercdinfos );
	
	if ( haswritablereadymedia( writercdinfos ) && hasenoughspacefordirectory( directory, writercdinfos ) ) {
		command = g_new ( gchar *, 4 );
		command[0] = g_strdup_printf( "simpleburn-burn-data" );
		command[1] = g_strdup_printf( "%s", cdwriter );
		command[2] = g_strdup_printf( "\"%s\"", directory );
		command[3] = NULL;
		
		action = g_strdup_printf( gettext("Burning data in directory '%s' on '%s' ..."), directory, cdwriter );
		displayinfo( action );
		startprogress( command, TRUE, TRUE, cdwriter );
		
		g_strfreev( command );
		g_free( action );
	}
	
	g_free( cdwriter );
	g_free( directory );
}
