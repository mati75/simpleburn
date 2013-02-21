#include <stdio.h>
#include <unistd.h>

#include <glib.h>

#include <cdio/cdio.h>
#include <cdio/cd_types.h>
#include <cdio/mmc.h>
#include "t_cdinfos.h"

//temporary untill libcdio inclusion
#include <stdlib.h>
#include <string.h>
#include "tmp/libcdio-cmds.c"
#include "tmp/libcdio-disctype.c"


int main (int argc, char *argv[]) {
	T_cdinfos cdinfos;
	gchar *device;
	CdIo_t *cdio;
	cdio_fs_anal_t fs;
	int i;
	cdio_iso_analysis_t cdio_iso_analysis; 
	_disctype_t disctype;
	gchar *command, *output;
	
	//for media loading
	uint8_t status[2];
	gint countdown;
	
	//for output only
	gchar mediatype[4];
	gchar mediacontent[8];
	
	if (argc == 1) {
		printf ("usage: %s device\nexample: %s /dev/sr0\n", argv[0], argv[0]);
		return;
	} else
		device = argv[1];
	
	cdinfos.hasmedia = FALSE;
	cdinfos.hasrewritablemedia = FALSE;
	cdinfos.hascdrommedia = FALSE;
	cdinfos.hasdvdmedia = FALSE;
	cdinfos.hasemptymedia = FALSE;
	cdinfos.hasaudiocdmedia = FALSE;
	cdinfos.hasdatamedia = FALSE;
	cdinfos.hasiso9660media = FALSE;
	cdinfos.hasudfmedia = FALSE;
	cdinfos.mediasize = 0;
	cdinfos.mediacapacity = 0;
	cdinfos.trackscount = 0;
	
	cdio = cdio_open (device, DRIVER_UNKNOWN);
	
	//wait until media loaded
	countdown = 8; //2 more seconds are needed after drive is loaded
	while (countdown != 0 && mmc_get_event_status (cdio,status) == DRIVER_OP_SUCCESS && status[1] == 0) { //1: no drive, 2: loaded
		sleep (1);
		countdown--;
	}
	if (countdown != 8)
		sleep(2);
	
	disctype = _mmc_get_disctype (cdio, NULL);
	
	if (_mmc_is_disctype_cdrom (disctype) || _mmc_is_disctype_dvd (disctype)) { //if there is a CD or DVD media
		cdinfos.hasmedia = TRUE;
		
		if (_mmc_is_disctype_cdrom (disctype)) {
			cdinfos.hascdrommedia = TRUE;
			sprintf (mediatype, "cd");
		} else {
			cdinfos.hasdvdmedia = TRUE;
			sprintf (mediatype, "dvd");
		}
	
		if (_mmc_is_disctype_rewritable (disctype))
			cdinfos.hasrewritablemedia = TRUE;
		
		cdinfos.mediacapacity = _mmc_get_disc_capacity (cdio, NULL);
		if (_mmc_get_disc_empty (cdio, NULL)) {
			sprintf (mediacontent, "blank");
			cdinfos.hasemptymedia = TRUE;
			cdinfos.mediasize = 0;
		} else {
			cdinfos.mediasize = cdio_get_disc_last_lsn (cdio) * 2;
			fs = cdio_guess_cd_type (cdio, 0, 1, &cdio_iso_analysis);
			if (CDIO_FSTYPE (fs) == CDIO_FS_AUDIO) {
				sprintf (mediacontent, "audio");
				cdinfos.hasaudiocdmedia = true;
			} else {
				if (CDIO_FSTYPE (fs) == CDIO_FS_ISO_9660 || CDIO_FSTYPE (fs) == CDIO_FS_UDF || CDIO_FSTYPE (fs) == CDIO_FS_ISO_UDF) {
					sprintf (mediacontent, "iso9660");
					cdinfos.hasdatamedia = TRUE;
					cdinfos.hasiso9660media = TRUE;
					command = g_strdup_printf ("simpleburn-get-datasize %s", device);
					g_spawn_command_line_sync (command, &output, NULL, NULL, NULL);
					g_free (command);
					cdinfos.mediasize = g_ascii_strtoull (output, NULL, 10);
					g_free (output);
				} else {
					sprintf (mediacontent, "blank");
					cdinfos.hasemptymedia = TRUE;
					cdinfos.mediasize = 0;
				}
			}
		}
		
		if (_mmc_is_disctype_cdrom (disctype) && ! cdinfos.hasemptymedia || ! cdinfos.hasemptymedia && ! cdinfos.hasrewritablemedia)
			cdinfos.mediacapacity = 0;
		
		if (cdinfos.mediasize != 0)
			cdinfos.trackscount = (guint8) cdio_get_last_track_num (cdio);
		
		if (argc == 3 && strncmp (argv[2], "oneline", 8) == 0)
		    printf ("%s:%s:%llu:%llu:%d:%d\n", mediatype, mediacontent, cdinfos.mediacapacity, cdinfos.mediasize, cdinfos.hasrewritablemedia, cdinfos.trackscount);
		else	
		    printf ("mediatype=%s\nmediacontent=%s\nmediacapacity=%llu\nmediasize=%llu\nrewritablemedia=%d\ntrackscount=%d\n", 
			mediatype, mediacontent, cdinfos.mediacapacity, cdinfos.mediasize, cdinfos.hasrewritablemedia, cdinfos.trackscount);
		
		cdio_destroy (cdio);
		return 0;
	} else {
		cdio_destroy (cdio);
		return 1;
	}
	
}
