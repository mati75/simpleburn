#include "t_cdinfos.h"
#include "config.h"

void setcdinfos ( gchar *device, T_cdinfos *cdinfos ) {
	gchar *command, *output, **infos;
	gint exitvalue;
	
	#ifdef DEBUG
		g_print ("DEBUG: setcdinfos: BEGIN\n");
	#endif
	g_strlcpy( cdinfos->cddevice, device, 20 );
	command = g_strdup_printf( "simpleburn-media-detection %s oneline", device );
	g_spawn_command_line_sync( command, &output, NULL, &exitvalue, NULL );
	#ifdef DEBUG
		g_print ("DEBUG: setcdinfos: '%s'\n", command);
		g_print ("DEBUG: setcdinfos: EXIT=%d, OUTPUT=%s\n", exitvalue, output);
	#endif
	
	cdinfos->hasmedia = FALSE;
	cdinfos->hasrewritablemedia = FALSE;
	cdinfos->hascdrommedia = FALSE;
	cdinfos->hasdvdmedia = FALSE;
	cdinfos->hasemptymedia = FALSE;
	cdinfos->hasaudiocdmedia = FALSE;
	cdinfos->hasdatamedia = FALSE;
	cdinfos->hasiso9660media = FALSE;
	cdinfos->hasudfmedia = FALSE;
	cdinfos->mediasize = 0;
	cdinfos->mediacapacity = 0;
	cdinfos->trackscount = 0;
	
	if ( exitvalue == 0) {
		#ifdef DEBUG
			g_print ("DEBUG: setcdinfos: simpleburn-media-detection success (CD / DVD media found)\n");
		#endif
		infos = g_strsplit( output, ":", 0 );
		if ( g_ascii_strncasecmp( "CD", infos[0], 2 ) == 0) {
			cdinfos->hasmedia = TRUE;
			cdinfos->hascdrommedia = TRUE;
		} else {
			if ( g_ascii_strncasecmp( "DVD", infos[0], 3 ) == 0) {
				cdinfos->hasmedia = TRUE;
				cdinfos->hasdvdmedia = TRUE;
			}
		}
				
		if ( cdinfos->hasmedia ) {
			if ( g_ascii_strncasecmp( "1", infos[4], 1 ) == 0)
				cdinfos->hasrewritablemedia = TRUE;

			if ( g_ascii_strncasecmp( "audio", infos[1], 5 ) == 0)
				cdinfos->hasaudiocdmedia = TRUE;
			else {
				if ( g_ascii_strncasecmp( "iso9660", infos[1], 7 ) == 0) {
					cdinfos->hasdatamedia = TRUE;
					cdinfos->hasiso9660media = TRUE;
				} else {
					if ( g_ascii_strncasecmp( "udf", infos[1], 3 ) == 0) {
						cdinfos->hasdatamedia = TRUE;
						cdinfos->hasudfmedia = TRUE;
					} else
						cdinfos->hasemptymedia = TRUE;
				}
			}
			
			cdinfos->mediacapacity = g_ascii_strtoull( infos[2], NULL, 10);
			cdinfos->mediasize = g_ascii_strtoull( infos[3], NULL, 10);
			cdinfos->trackscount = (guint8) g_ascii_strtoull( infos[5], NULL, 10);
		}
		g_strfreev( infos );
	}
	
	g_free( command );
	g_free( output );
	#ifdef DEBUG
		g_print ("DEBUG: setcdinfos: END\n");
	#endif
}


#ifdef UDEV_DETECTION
void listcddevices( gchar ***devices, gboolean writersonly ) {//'devices' is the address (for return value) of a null terminated array of strings
	glob_t cddevices;
	gchar *devicename, *removablepath;
	gint i, p;
	gchar *command, *output, *removable;
	gint exitvalue;
	
	#ifdef DEBUG
		g_print ("DEBUG: udev/listcddevices: BEGIN\n");
	#endif
	if ( glob( "/sys/block/*", 0, NULL, &cddevices ) == GLOB_NOMATCH ) {
		#ifdef DEBUG
			g_print ("DEBUG: udev/listcddevices: no block devices in /sys/block\n");
		#endif
		*devices = g_new( gchar *, 1 );
		(*devices)[0] = NULL;
	} else {
		#ifdef DEBUG
			g_print ("DEBUG: udev/listcddevices: %d block devices in '/sys/block'\n", cddevices.gl_pathc);
		#endif
		*devices = g_new( gchar *, cddevices.gl_pathc+1 );
		p = 0;
		for ( i = 0; i < cddevices.gl_pathc; i++ ) {
			#ifdef DEBUG
				g_print ("DEBUG: udev/listcddevices: n°%d: '%s'\n", i, cddevices.gl_pathv[i]);
			#endif
			removablepath = g_strdup_printf( "%s/removable", cddevices.gl_pathv[i] );
			g_file_get_contents( removablepath, &removable, NULL, NULL );
			if (removable[0] == '1') {
				devicename = g_path_get_basename( cddevices.gl_pathv[i] );
				command = g_strdup_printf( "%s/cdrom_id /dev/%s", UDEV_ROOT, devicename );
				g_spawn_command_line_sync( command, &output, NULL, &exitvalue, NULL );
				if ( exitvalue == 0 ) { //if it is a CD / DVD device
					#ifdef DEBUG
						g_print ("DEBUG: udev/listcddevices: '/dev/%s' is a CD / DVD device\n", devicename);
					#endif
					if ( ! writersonly || g_regex_match_simple( "ID_CDROM_CD_R=1", output, 0, 0 ) ) {
						(*devices)[p] = g_strdup_printf( "/dev/%s", devicename );
						#ifdef DEBUG
							g_print ("DEBUG: udev/listcddevices: '%s' added to devices list [%d]\n", (*devices)[p], p);
						#endif
						p++;
					}
				}
				g_free( command );
				g_free( output );
				g_free( devicename );
			}
			g_free( removable );
			g_free( removablepath );
		}
		(*devices)[p] = NULL;
	}
	globfree( &cddevices );
	
	#ifdef DEBUG
		g_print ("DEBUG: udev/listcddevices: END\n");
	#endif
}
#endif


#ifdef LIBCDIO_DETECTION
void listcddevices (gchar ***devices, gboolean writersonly) {//'devices' is the address (for return value) of a null terminated array of strings
	gint i, p;
	gchar ** tmpdevices;
	cdio_drive_read_cap_t read_cap;
	cdio_drive_write_cap_t write_cap;
	cdio_drive_misc_cap_t misc_cap;
	
	#ifdef DEBUG
		g_print ("DEBUG: cdio/listcddevices: BEGIN\n");
	#endif
	
	tmpdevices = cdio_get_devices (DRIVER_DEVICE);
	if (tmpdevices == NULL) {
		*devices = g_new (gchar *, 1);
		(*devices)[0] = NULL;
	} else {
		*devices = g_new (gchar *, g_strv_length (tmpdevices)+1);
		p = 0;
		for (i = 0; tmpdevices[i] != NULL; i++) {
			#ifdef DEBUG
			g_print ("DEBUG: cdio/listcddevices: n°%d: '%s'\n", i, tmpdevices[i]);
			#endif
			#ifndef ALLOW_DEVICES_SYMLINKS
			if (! g_file_test (tmpdevices[i], G_FILE_TEST_IS_SYMLINK)) {
			#endif
				cdio_get_drive_cap_dev (tmpdevices[i], &read_cap, &write_cap, &misc_cap);
				if (write_cap != 0 || ! writersonly) {
					(*devices)[p] = g_strdup_printf ("%s", tmpdevices[i]);
					#ifdef DEBUG
						g_print ("DEBUG: cdio/listcddevices: '%s' added to devices list [%d]\n", (*devices)[p], p);
					#endif
					p++;
				}
			#ifndef ALLOW_DEVICES_SYMLINKS
			}
			#endif
		}
		(*devices)[p] = NULL;
		g_strfreev (tmpdevices);
	}
	
	#ifdef DEBUG
		g_print ("DEBUG: cdio/listcddevices: END\n");
	#endif
}
#endif


void listcdwriters( gchar ***devices ) {
	#ifdef DEBUG
		g_print ("DEBUG: listcdwriters: BEGIN\n");
	#endif
	listcddevices( devices, TRUE );
	#ifdef DEBUG
		g_print ("DEBUG: listcdwriters: devices list:\n");
		int i;
		for (i = 0; (*devices)[i] != NULL; i++)
			g_print ("DEBUG: listcdwriters: devices[%d] = %s\n", i, (*devices)[i]);
		g_print ("DEBUG: listcdwriters: END\n");
	#endif
}


void listcdreaders( gchar ***devices ) {
	#ifdef DEBUG
		g_print ("DEBUG: listcdreaders: BEGIN\n");
	#endif
	listcddevices( devices, FALSE );
	#ifdef DEBUG
		g_print ("DEBUG: listcdreaders: devices list:\n");
		int i;
		for (i = 0; (*devices)[i] != NULL; i++)
			g_print ("DEBUG: listcdreaders: devices[%d] = %s\n", i, (*devices)[i]);
		g_print ("DEBUG: listcdreaders: END\n");
	#endif
}
