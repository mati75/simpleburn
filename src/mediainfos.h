typedef struct {
	gint id;
	gchar lang[3];
} T_LanguageInfos;

typedef struct {
	gint id;
	gint length; //in seconds
	T_LanguageInfos **languages;
	gint langscount;
	T_LanguageInfos **subtitles;
	gint subscount;
	gchar *cropinfos;
} T_TitleInfos;

struct {
	gchar *mediatype;
	gboolean hasmedia;
	gboolean cdtype;
	gchar * mediacontent;
	gboolean emptycontent;
	gboolean videocontent;
	gboolean audiocontent;
	gboolean iso9660content;
	gboolean rewritablemedia;
	guint64 mediacapacity;
	guint64 mediasize; //bytes
	guint8 trackscount;
	gchar *medialabel;
	T_TitleInfos **titles;
} mediainfos;

void init_mediainfos();

void free_mediainfos();

gchar **load_mediainfos();

void commoninfos(gchar *firstinfoline);

void audioinfos(gchar **infolines);

void videoinfos(gchar **infolines);

void on_detectmedia_clicked(GtkWidget *widget, gpointer user_data);

void on_videotitle_changed(GtkWidget *widget, gpointer user_data);
