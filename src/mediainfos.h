/**
 * open cdio and wait until media is loaded
 * returns null on cdio open error
 * returns opened cdio when ready
 */
CdIo_t *loaddevice(gchar *device);

void load_audioinfos(CdIo_t *cdio);

void load_mediainfos(gchar *device);

gboolean load_videoinfos(gchar *device); //video DVD (list title and lengths)

void load_titleinfos(gchar *device, gint title); //video DVD (title detail)

void recase(gchar *str);
void lowercase(gchar *str);
