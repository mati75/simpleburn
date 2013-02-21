void mediablanking( gchar *blanktype, gchar *cdwriter ) {
	gchar **command;
	
	command = g_new ( gchar *, 4 );
	command[0] = g_strdup_printf( "simpleburn-blank-media" );
	command[1] = g_strdup_printf( "%s", blanktype );
	command[2] = g_strdup_printf( "%s", cdwriter );
	command[3] = NULL;
	startprogress( command, FALSE, FALSE, cdwriter );
	g_strfreev( command );
}


void fastblankinghandler( GtkWidget *widget, gpointer user_data ) {
	GtkWidget *cdwriterwidget;
	gchar *cdwriter;
	gchar *action;
	T_cdinfos writercdinfos;
	
	cdwriterwidget = (GtkWidget *) gtk_builder_get_object( xml, "blankcdwriter" );
	cdwriter = gtk_combo_box_get_active_text( (GtkComboBox*) cdwriterwidget );
	
	setcdinfos( cdwriter, &writercdinfos );
	
	if ( hasrewritablemedia( writercdinfos ) ) {
		if ( writercdinfos.hascdrommedia )
			action = g_strdup_printf( gettext("Fast '%s' CD/RW blanking ..."), cdwriter );
		else
			action = g_strdup_printf( gettext("Fast '%s' DVD/RW blanking ..."), cdwriter );
		displayinfo( action );
		mediablanking( "fast", cdwriter );
		g_free( action );
	}
	
	g_free( cdwriter );
}


void completeblankinghandler( GtkWidget *widget, gpointer user_data ) {
	GtkWidget *cdwriterwidget;
	gchar *cdwriter;
	gchar *action;
	T_cdinfos writercdinfos;
	
	cdwriterwidget = (GtkWidget *) gtk_builder_get_object( xml, "blankcdwriter" );
	cdwriter = gtk_combo_box_get_active_text( (GtkComboBox*) cdwriterwidget );
	
	setcdinfos( cdwriter, &writercdinfos );
	
	if ( hasrewritablemedia( writercdinfos ) ) {
		if ( writercdinfos.hascdrommedia )
			action = g_strdup_printf( gettext("Full '%s' CD/RW blanking ..."), cdwriter );
		else
			action = g_strdup_printf( gettext("Full '%s' DVD/RW blanking ..."), cdwriter );
		displayinfo( action );
		mediablanking( "all", cdwriter );
		g_free( action );
	}
	
	g_free( cdwriter );
}
