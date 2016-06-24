#include <stdlib.h>

#define MMC_TRACK_INFORMATION_DATA_SIZE 48
#define CDIO_MMC_GPCMD_READ_TRACK_INFORMATION 0x52
/**
	Retrieves MMC TRACK_INFORMATION DATA.
	@param p_cdio the CD object to be acted upon.
	@param tracknum the track number to query (use 0x00 for the whole disc).
	@param mmc_track_information_data, in return. Free when no longer needed (even if operation fails).
 */
driver_return_code_t
mmc_get_track_information_data (const CdIo_t *p_cdio,
								track_t trackno,
								uint8_t **mmc_track_information_data)
{
	mmc_cdb_t cdb = {{0, }};
	*mmc_track_information_data = (uint8_t*) malloc (MMC_TRACK_INFORMATION_DATA_SIZE * sizeof (uint8_t));
	memset (*mmc_track_information_data, 0, MMC_TRACK_INFORMATION_DATA_SIZE);
	CDIO_MMC_SET_COMMAND (cdb.field, CDIO_MMC_GPCMD_READ_TRACK_INFORMATION);
	CDIO_MMC_SET_READ_LENGTH16 (cdb.field, MMC_TRACK_INFORMATION_DATA_SIZE);
	cdb.field[1] = trackno;
	return mmc_run_cmd (p_cdio, 0, &cdb, SCSI_MMC_DATA_READ, 
			MMC_TRACK_INFORMATION_DATA_SIZE, *mmc_track_information_data);
}


#define MMC_DISC_INFORMATION_DATA_SIZE 42
/**
	Retrieves MMC DISC_INFORMATION DATA.
	@param p_cdio the CD object to be acted upon.
	@param mmc_disc_information_data, in return. Free when no longer needed (even if operation fails).
 */
driver_return_code_t
mmc_get_disc_information_data ( const CdIo_t *p_cdio, 
								uint8_t **mmc_disc_information_data)
{
	mmc_cdb_t cdb = {{0, }};
	*mmc_disc_information_data = (uint8_t*) malloc (MMC_DISC_INFORMATION_DATA_SIZE * sizeof (uint8_t));
	memset (*mmc_disc_information_data, 0, MMC_DISC_INFORMATION_DATA_SIZE);
	CDIO_MMC_SET_COMMAND (cdb.field, CDIO_MMC_GPCMD_READ_DISC_INFO);
	CDIO_MMC_SET_READ_LENGTH16 (cdb.field, MMC_DISC_INFORMATION_DATA_SIZE);
	return mmc_run_cmd (p_cdio, 0, &cdb, SCSI_MMC_DATA_READ, 
			MMC_DISC_INFORMATION_DATA_SIZE, *mmc_disc_information_data);
}


/**
	Detects if a disc (CD or DVD) is empy or not.
  ** Warning: blanked DVD-RW report non empty ! **
	@param p_cdio the CD object to be acted upon.
	@param is_empty, in return.
 */
driver_return_code_t
mmc_get_disc_empty (const CdIo_t *p_cdio, 
					bool *is_empty)
{
	driver_return_code_t i_status;
	uint8_t *mmc_disc_information_data;
	uint8_t emptybyte; 
	i_status = mmc_get_disc_information_data (p_cdio, &mmc_disc_information_data);
	if (i_status == DRIVER_OP_SUCCESS) {
		*is_empty = (mmc_disc_information_data[2] & 0x03) == 0x00;
	}
	free (mmc_disc_information_data);
	
	return i_status;
}


/**
	Detects a disc (CD or DVD) capacity.
	@param p_user_data the CD object to be acted upon.
	@param capacity, in return.
 */
driver_return_code_t
mmc_get_disc_capacity ( const CdIo_t *p_cdio, 
						uint32_t *capacity)
{
	driver_return_code_t i_status;
	uint8_t *mmc_track_information_data;
	uint8_t *p;
	i_status = mmc_get_track_information_data (p_cdio, 0x00, &mmc_track_information_data); //will "not work" with multi track CDs, but they have to be blanked before use
	if (i_status == DRIVER_OP_SUCCESS) {
		p = mmc_track_information_data + 24;
		*capacity = CDIO_MMC_GET_LEN32 (p);
		*capacity *= 2;
	} else {
		*capacity = 0;
	}
	free (mmc_track_information_data);
	return i_status;
}


gboolean
mmc_disctype_is_rewritable (cdio_mmc_feature_profile_t disctype) { 
	return disctype == CDIO_MMC_FEATURE_PROF_CD_RW || disctype == CDIO_MMC_FEATURE_PROF_DVD_RAM || disctype == CDIO_MMC_FEATURE_PROF_DVD_PRW;
}


gboolean
mmc_disctype_is_writable (cdio_mmc_feature_profile_t disctype) { 
	return disctype != CDIO_MMC_FEATURE_PROF_CD_ROM && disctype != CDIO_MMC_FEATURE_PROF_DVD_ROM;
}
