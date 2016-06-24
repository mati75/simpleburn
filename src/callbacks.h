void callbackslink();

void on_ejectmedia_clicked(GtkWidget *widget, gpointer user_data);

void on_selected_toggled(GtkCellRendererToggle *cell_renderer, gchar *path, gpointer user_data);

void on_blankfast_clicked(GtkWidget *widget, gpointer user_data);

void on_blankfull_clicked(GtkWidget *widget, gpointer user_data);

void on_fileburn_clicked(GtkWidget *widget, gpointer user_data);

void on_audioburn_clicked(GtkWidget *widget, gpointer user_data);

void on_dirburn_clicked(GtkWidget *widget, gpointer user_data);

void on_tracks_row_activated(GtkTreeView *tree_view, GtkTreePath *path, GtkTreeViewColumn *column, gpointer user_data);

void on_extractaudio_clicked(GtkWidget *widget, gpointer user_data);

void on_previewvideo_clicked(GtkWidget *widget, gpointer user_data);

void on_extractvideo_clicked(GtkWidget *widget, gpointer user_data);

void on_extractiso_clicked(GtkWidget *widget, gpointer user_data);

void on_detectmedia_clicked(GtkWidget *widget, gpointer user_data);

void on_videotitle_changed(GtkWidget *widget, gpointer user_data);

void reset_mediadetection();
