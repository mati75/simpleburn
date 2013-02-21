guint64 getfilesize( gchar *filename ) {
	struct stat fileinfos;
	gchar *s_size;
	guint64 size;
	
	stat( filename, &fileinfos );
	s_size = g_strdup_printf ("%lu", fileinfos.st_size);
	size = g_ascii_strtoull (s_size, NULL, 10) / 1024;
	g_free (s_size);
	return size;
	//~ return fileinfos.st_size / 1024; should work, but no !!!
}


guint64 getdirectorysize( gchar *directoryname ) {
	gchar *command, *output;
	guint64 size;
	gint exitstatus;
	
	command = g_strdup_printf( "mkisofs -J -r -N -d -hide-rr-moved -print-size '%s'", directoryname );
	g_spawn_command_line_sync( command, &output, NULL, &exitstatus, NULL );
	g_free( command );
	if ( exitstatus !=0 ){
		g_free( output );
		command = g_strdup_printf( "genisoimage -J -r -N -d -hide-rr-moved -print-size '%s'", directoryname );
		g_spawn_command_line_sync( command, &output, NULL, NULL, NULL );
		g_free( command );
	}
	size = g_ascii_strtoull( output, NULL, 10);
	g_free( output );
	return size;
}


guint64 getaudiodirectorysize( gchar *directoryname ) {
	guint64 size = 0;
	gchar *wavfilespattern;
	glob_t wavfiles;
	gint i;
	
	wavfilespattern = g_strdup_printf( "%s/*.wav", directoryname );
	glob( wavfilespattern, GLOB_NOMATCH, NULL, &wavfiles );
	g_free( wavfilespattern );
	for ( i=0; i<wavfiles.gl_pathc; i++ )
		size += getfilesize( wavfiles.gl_pathv[i] );
		
	globfree( &wavfiles );
	size = (guint64) size * 2048 / 2352;
	return size;
}
