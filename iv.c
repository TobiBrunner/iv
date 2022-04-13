#define _GNU_SOURCE
#include <gtk/gtk.h>
#include <dirent.h>
#include <string.h>

enum list{

    LIST_ITEM = 0,
    N_COLUMNS
};

typedef struct {
    GtkWidget *window;
    GtkWidget *dir_list;
    GtkWidget *file_list;
    GtkWidget *tf_path;
    GtkWidget *label;
    GtkWidget *commandLine;
    GtkWidget *image;
    GtkWidget *box_container;
    GtkWidget *box_left;
    GtkWidget *box_right;
    GtkWidget *paned;
    GtkWidget *paned_lists;
    GtkWidget *scrolled_dir_list;
    GtkWidget *scrolled_file_list;
} GuiModel;

static int one (const struct dirent *unused) {
    return 1;
}

void guimodel_init(GuiModel *m);
void guimodel_assemble(GuiModel *m);
void init_list(GtkWidget *widget);
void init_file_list(GuiModel *m);
void add_to_list(GtkWidget *widget, const gchar *str);
void get_selected_file(GtkWidget *widget, gpointer data);
const char* removeCurrentDirFromPath(gchar *string);
static void get_selected_dir(GtkTreeView *tree_view, GtkTreePath *path, GtkTreeViewColumn *column, gpointer user_data);
void add_files_in_dir_to_list(GuiModel *m, const gchar *str);
void clear_dir_list_store(GuiModel *m);
void update_list_stores(GtkWidget *widget, gpointer data);

int main(int argc, char *argv[]) {
    gtk_init(&argc, &argv);
    GuiModel m;

    guimodel_init(&m);
    guimodel_assemble(&m);

    gtk_window_set_title(GTK_WINDOW(m.window), "ImageViewer - IV");
    gtk_window_set_position(GTK_WINDOW(m.window), GTK_WIN_POS_CENTER);
    gtk_container_set_border_width(GTK_CONTAINER(m.window), 10);
    gtk_window_set_default_size(GTK_WINDOW(m.window), 1500, 1000);

    // initialisiert Eingabefeld mit aktuellem Pfad
    gtk_entry_set_text(GTK_ENTRY(m.tf_path), get_current_dir_name());


    gtk_tree_view_set_headers_visible(GTK_TREE_VIEW(m.dir_list), FALSE);
    init_list(m.dir_list);

    gtk_tree_view_set_headers_visible(GTK_TREE_VIEW(m.file_list), FALSE);
    init_list(m.file_list);

    // befüllt die beiden Listen mit Ordnern und Dateien
    add_files_in_dir_to_list(&m, get_current_dir_name());

    GtkTreeSelection *fileSelection; 
    fileSelection = gtk_tree_view_get_selection(GTK_TREE_VIEW(m.file_list));

    // Callback für ausgewählte Datei
    g_signal_connect(fileSelection, "changed", G_CALLBACK(get_selected_file), &m);

    // Callback für ausgewählten Ordner
    g_signal_connect(m.dir_list, "row-activated", G_CALLBACK(get_selected_dir), &m);


    //Callback für Enter drücken im Eingabefeld
    g_signal_connect(G_OBJECT(m.tf_path), "activate", G_CALLBACK(update_list_stores), &m);
    g_signal_connect(G_OBJECT (m.window), "destroy",G_CALLBACK(gtk_main_quit), NULL);

    gtk_widget_show_all(m.window);
    gtk_main();

    return 0;
}

void guimodel_init(GuiModel *m) {
    m->window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
    m->tf_path = gtk_entry_new();
    m->label = gtk_label_new("selected File");
    m->commandLine = gtk_entry_new();
    m->image = gtk_image_new_from_file("placeholder-image.png");
    m->paned = gtk_paned_new(GTK_ORIENTATION_HORIZONTAL);
    m->paned_lists = gtk_paned_new(GTK_ORIENTATION_VERTICAL);
    m->box_left = gtk_box_new(GTK_ORIENTATION_VERTICAL, 0);
    m->box_right = gtk_box_new(GTK_ORIENTATION_VERTICAL, 0);
    m->scrolled_dir_list = gtk_scrolled_window_new(NULL, NULL);
    m->scrolled_file_list = gtk_scrolled_window_new(NULL, NULL);
    m->dir_list = gtk_tree_view_new();
    m->file_list = gtk_tree_view_new();
}

void guimodel_assemble(GuiModel *m) {
    gtk_container_add(GTK_CONTAINER(m->window), m->paned);

    // setup paned boxes
    gtk_widget_set_size_request (m->paned, 200, -1);
    gtk_paned_pack1 (GTK_PANED(m->paned), m->box_left, TRUE, TRUE);
    gtk_widget_set_size_request (m->box_left, 50, -1);
    gtk_paned_pack2 (GTK_PANED(m->paned), m->box_right, FALSE, TRUE);
    gtk_widget_set_size_request (m->box_right, 50, -1);

    // setup paned dir and file list
    gtk_widget_set_size_request (m->paned_lists, 200, -1);
    gtk_paned_pack1 (GTK_PANED(m->paned_lists), m->scrolled_dir_list, TRUE, TRUE);
    gtk_widget_set_size_request (m->scrolled_dir_list, 50, -1);
    gtk_paned_pack2 (GTK_PANED(m->paned_lists), m->scrolled_file_list, FALSE, TRUE);
    gtk_widget_set_size_request (m->scrolled_file_list, 50, -1);

    // add lists to scrolledwindow
    gtk_container_add(GTK_CONTAINER(m->scrolled_dir_list), m->dir_list);
    gtk_container_add(GTK_CONTAINER(m->scrolled_file_list), m->file_list);

    // set min height of lists
    gtk_scrolled_window_set_min_content_height(GTK_SCROLLED_WINDOW(m->scrolled_dir_list), 150);
    gtk_scrolled_window_set_min_content_height(GTK_SCROLLED_WINDOW(m->scrolled_file_list), 150);

    // add items to left box
    gtk_box_pack_start(GTK_BOX(m->box_left), m->tf_path, FALSE, TRUE, 0);
    gtk_box_pack_start(GTK_BOX(m->box_left), m->paned_lists, FALSE, TRUE, 0);
    // gtk_box_pack_start(GTK_BOX(m->box_left), m->commandLine, FALSE, TRUE, 0);
    
    // add image to right box
    gtk_box_pack_start(GTK_BOX(m->box_right), m->image, TRUE, TRUE, 0);
}

void init_list(GtkWidget *widget) {

    GtkCellRenderer *renderer;
    GtkTreeViewColumn *column;
    GtkListStore *store;

    renderer = gtk_cell_renderer_text_new ();
    column = gtk_tree_view_column_new_with_attributes("List", renderer, "text", LIST_ITEM, NULL);
    gtk_tree_view_append_column(GTK_TREE_VIEW(widget), column);

    store = gtk_list_store_new(N_COLUMNS, G_TYPE_STRING);
    gtk_tree_view_set_model(GTK_TREE_VIEW(widget), GTK_TREE_MODEL(store));

    g_object_unref(store);
}

void add_to_list(GtkWidget* widget, const gchar *str) {
    
    GtkListStore *store;
    GtkTreeIter iter;

    store = GTK_LIST_STORE(gtk_tree_view_get_model(GTK_TREE_VIEW(widget)));

    gtk_list_store_append(store, &iter);
    gtk_list_store_set(store, &iter, LIST_ITEM, str, -1);
}

void add_files_in_dir_to_list(GuiModel *m, const gchar *str) {

    struct dirent **dir;
    int n;
    n = scandir (str, &dir, one, alphasort);   
    if (n >= 0) {
        int cnt;
        for (cnt = 0; cnt < n; ++cnt) {
            if (dir[cnt]->d_type == DT_DIR) {
                if(dir[cnt]->d_name[0] == '.' && dir[cnt]->d_name[1] == '.') {
                    add_to_list(m->dir_list, dir[cnt]->d_name);
                }
                if(dir[cnt]->d_name[0] != '.') {
                    add_to_list(m->dir_list, dir[cnt]->d_name);
              }
            } else if (dir[cnt]->d_type == DT_REG) {
                if(dir[cnt]->d_name[0] != '.') {
                    add_to_list(m->file_list, dir[cnt]->d_name);
              }
            } 
        }
    } else
        perror ("Couldn't open the directory");
}

void get_selected_file(GtkWidget *widget, gpointer data) {
    
    GuiModel *m = (GuiModel*)data;
    GtkTreeIter iter;
    GtkTreeModel *model;
    gchar *value;

    if (gtk_tree_selection_get_selected(GTK_TREE_SELECTION(widget), &model, &iter)) {

        gtk_tree_model_get(model, &iter, LIST_ITEM, &value,  -1);
        //gtk_label_set_text(GTK_LABEL(m->label), value);
        char buf[100];
        strcpy(buf, gtk_entry_get_text(GTK_ENTRY(m->tf_path)));
        if(buf[strlen(buf)-1] != '/') {
              strcat(buf, "/");
          }
        strcat(buf, value);

        gtk_image_set_from_file(GTK_IMAGE(m->image), buf);

        g_free(value);
    }
}

static void get_selected_dir(GtkTreeView *tree_view, GtkTreePath *path, GtkTreeViewColumn *column, gpointer user_data) {
    GuiModel *m = (GuiModel*)user_data;
    gchar *string;
    GtkTreeIter iter;
    GtkTreeModel *model = gtk_tree_view_get_model (tree_view);
    gtk_tree_model_get_iter(model, &iter, path); 
    gtk_tree_model_get (model, &iter, 0, &string, -1);

    char buf[100];
        strcpy(buf, gtk_entry_get_text(GTK_ENTRY(m->tf_path)));
    
    if ((strcmp(string, "..") == 0)) {
        strcpy(buf, removeCurrentDirFromPath(buf));
        gtk_entry_set_text(GTK_ENTRY(m->tf_path), buf);

    } else {
        if(buf[strlen(buf)-1] != '/') {
            strcat(buf, "/");
        }
        strcat(buf, string);
        gtk_entry_set_text(GTK_ENTRY(m->tf_path), buf);
    }

    update_list_stores(NULL, m);
    g_free(string);
 }

const char* removeCurrentDirFromPath(char *string) {
    int length = strlen(string)-1;
    for(int i = length; i > 0; --i) {
        string[i] = '\0';
         if(string[i-1] == '/'){
             break;
         }
    }
    return string;
}

void clear_dir_list_store(GuiModel *m) {
    GtkTreeModel *model = gtk_tree_view_get_model(GTK_TREE_VIEW(m->dir_list));
    gtk_list_store_clear(GTK_LIST_STORE(model));

    model = gtk_tree_view_get_model(GTK_TREE_VIEW(m->file_list));
    gtk_list_store_clear(GTK_LIST_STORE(model));
}

void update_list_stores(GtkWidget *widget, gpointer data) {
    GuiModel *m = (GuiModel*)data;
    const gchar *path;

    path = gtk_entry_get_text(GTK_ENTRY(m->tf_path));
    clear_dir_list_store(m);
    add_files_in_dir_to_list(m, path);
}