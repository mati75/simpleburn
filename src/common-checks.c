gboolean hasrewritablemedia( T_cdinfos cdinfos ) {
	if ( cdinfos.hasrewritablemedia )
		return TRUE;
	else {
		displayerror( gettext( "Error : no rewritable CD / DVD in drive.") );
		return FALSE;
	}
}


gboolean hascdrommedia( T_cdinfos cdinfos ) {
	if ( cdinfos.hascdrommedia )
		return TRUE;
	else {
		displayerror( gettext( "Error : no CD in drive." ) );
		return FALSE;
	}
}


gboolean hasdvdmedia( T_cdinfos cdinfos ) {
	if ( cdinfos.hasdvdmedia )
		return TRUE;
	else {
		displayerror( gettext( "Error : no DVD in drive." ) );
		return FALSE;
	}
}


gboolean haswritablereadymedia( T_cdinfos cdinfos ) {
	gchar *message;
	if ( cdinfos.hasemptymedia || cdinfos.hasdvdmedia && cdinfos.hasrewritablemedia ) //rewritable DVD can be burned even if they are not blank
		return TRUE;
	else {
		message = g_strdup_printf( gettext( "Error : media in drive is not blank." ) );
		displayerror( message );
		g_free( message );
		return FALSE;
	}
}



gboolean hasnonemptymedia( T_cdinfos cdinfos ) {
	gchar *message;
	if ( ! cdinfos.hasemptymedia )
		return TRUE;
	else {
		message = g_strdup_printf( gettext( "Error : media in drive is blank." ) );
		displayerror( message );
		g_free( message );
		return FALSE;
	}
}


gboolean hasaudiocdmedia( T_cdinfos cdinfos ) {
	if ( cdinfos.hasaudiocdmedia )
		return TRUE;
	else {
		displayerror( gettext( "Error : no audio CD in drive." ) );
		return FALSE;
	}
}


gboolean hasiso9660media( T_cdinfos cdinfos ) {
	if ( cdinfos.hasiso9660media )
		return TRUE;
	else {
		displayerror( gettext( "Error : no ISO9660 formatted media in drive." ) );
		return FALSE;
	}
}


gboolean hasenoughspaceforimage( gchar *filename, T_cdinfos cdinfos ) {
	if ( cdinfos.mediacapacity !=0 && getfilesize( filename ) > cdinfos.mediacapacity ) {
		displayerror( gettext( "Error : not enough space on media.") );
		return FALSE;
	}
	else
		return TRUE;
}


gboolean hasenoughspacefordirectory( gchar *directoryname, T_cdinfos cdinfos ) {
	if ( cdinfos.mediacapacity !=0 && getdirectorysize( directoryname ) > cdinfos.mediacapacity ) {
		displayerror( gettext( "Error : not enough space on media.") );
		return FALSE;
	}
	else
		return TRUE;
}


gboolean hasenoughspaceforaudiodirectory( gchar *directoryname, T_cdinfos cdinfos ) {
	if ( cdinfos.mediacapacity !=0 && getaudiodirectorysize( directoryname ) > cdinfos.mediacapacity ) {
		displayerror( gettext( "Error : not enough space on media.") );
		return FALSE;
	}
	else
		return TRUE;
}


gboolean hasenoughspaceforcopy( T_cdinfos readercdinfos, T_cdinfos writercdinfos ) {
	if ( writercdinfos.mediacapacity !=0 &&  writercdinfos.mediacapacity < readercdinfos.mediasize ) {
		displayerror( gettext( "Error : not enough space on media.") );
		return FALSE;
	}
	else
		return TRUE;
}


gboolean hasfilenameset( gchar *widgetname ) {
	GtkWidget *widget;
	gchar *filename;
	
	widget = (GtkWidget *) gtk_builder_get_object( xml, widgetname );
	filename = gtk_file_chooser_get_filename( (GtkFileChooser*) widget );
	if ( filename == NULL ) {
		displayerror( gettext( "Error : image file is not set.") );
		return FALSE;
	} else {
		g_free( filename);
		return TRUE;
	}
}


gboolean hasfilename( gchar *widgetname ) {
	GtkWidget *widget;
	
	widget = (GtkWidget *) gtk_builder_get_object( xml, widgetname );
	if ( g_ascii_strncasecmp( "", gtk_entry_get_text ( (GtkEntry*) widget), 1 ) == 0 ) {
		displayerror( gettext( "Error : image file name empty.") );
		return FALSE;
	}
	else
		return TRUE;
}


gboolean readerandwriterarenotsame( gchar *reader, gchar *writer ) {
	if ( g_ascii_strcasecmp( reader, writer ) == 0 ) {
		displayerror( gettext( "Error : CD / DVD reader and writer must be different for copy." ) );
		return FALSE;
	}
	else
		return TRUE;
}
