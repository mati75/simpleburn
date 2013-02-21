void copymediahandler( GtkWidget *widget, gpointer user_data ) {
	GtkWidget *cdreaderwidget, *cdwriterwidget;
	gchar **command;
	gchar *cdreader, *cdwriter;
	gchar *action;
	T_cdinfos readercdinfos, writercdinfos;
	
	cdreaderwidget = (GtkWidget *) gtk_builder_get_object( xml, "copycdreader" );
	cdreader = gtk_combo_box_get_active_text( (GtkComboBox*) cdreaderwidget );
	cdwriterwidget = (GtkWidget *) gtk_builder_get_object( xml, "copycdwriter" );
	cdwriter = gtk_combo_box_get_active_text( (GtkComboBox*) cdwriterwidget );
	
	setcdinfos( cdreader, &readercdinfos );
	setcdinfos( cdwriter, &writercdinfos );
	
	if ( readerandwriterarenotsame( cdreader, cdwriter ) &&
			hasnonemptymedia( readercdinfos ) && haswritablereadymedia( writercdinfos ) &&
			hasenoughspaceforcopy( readercdinfos, writercdinfos ) ) {
	
		command = g_new ( gchar *, 4 );
		command[1] = g_strdup_printf( "%s", cdreader );
		command[2] = g_strdup_printf( "%s", cdwriter );
		command[3] = NULL;
		
		if ( readercdinfos.hasaudiocdmedia ) {
			command[0] = g_strdup_printf( "simpleburn-copy-audio" );
			action = g_strdup_printf( gettext("Copy audio CD in '%s' to '%s' ..."), cdreader, cdwriter );
		} else {
			command[0] = g_strdup_printf( "simpleburn-copy-data" );
			action = g_strdup_printf( gettext("Copy data CD / DVD in '%s' to '%s' ..."), cdreader, cdwriter );
		}
		
		displayinfo( action );
		startprogress( command, TRUE, TRUE, cdwriter );
		
		g_strfreev( command );
		g_free( action );
	}
	
	g_free( cdreader );
	g_free( cdwriter );
}
